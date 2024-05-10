#include "SvgNode.h"
#include <native_drawing/drawing_matrix.h>
#include <native_drawing/drawing_path.h>
#include <regex>
#include <string>
#include "properties/SvgDomType.h"
#include "utils/LinearMap.h"
#include "utils/StringUtils.h"
#include "utils/SvgAttributesParser.h"
#include "utils/Utils.h"
#include "utils/StringUtils.h"
#include "utils/SvgAttributesParser.h"
#include "SVGGradient.h"

namespace rnoh {

void SvgNode::InitStyle(const SvgBaseAttribute &attr) {
    InheritAttr(attr);
    if (hrefFill_) {
        LOG(INFO) << "[UpdateCommonProps] hrefFill_";
        auto href = attributes_.fillState.GetHref();
        if (!href.empty()) {
            auto gradient = GetGradient(href);
            if (gradient) {
                LOG(INFO) << "[UpdateCommonProps] fill state set gradient";
                attributes_.fillState.SetGradient(gradient.value(), true);
            } else {
                LOG(INFO) << "[UpdateCommonProps] no gradient";
            }
        } else {
            LOG(INFO) << "[UpdateCommonProps] href empty";
        }
    }
    OnInitStyle();
    if (passStyle_) {
        for (auto &node : children_) {
            // pass down style only if child inheritStyle_ is true
            node->InitStyle((node->inheritStyle_) ? attributes_ : SvgBaseAttribute());
        }
    }
}

void SvgNode::OnDrawTraversed(OH_Drawing_Canvas *canvas) {
    auto smoothEdge = GetSmoothEdge();
    for (auto &node : children_) {
        if (node && node->drawTraversed_) {
            if (GreatNotEqual(smoothEdge, 0.0f)) {
                node->SetSmoothEdge(smoothEdge);
            }
            node->Draw(canvas);
        }
    }
}

const Rect &SvgNode::GetRootViewBox() const {
    if (!context_) {
        LOG(INFO) << "[GetRootViewBox] failed, svgContext is null";
        static Rect empty;
        return empty;
    }
    LOG(INFO) << "[GetRootViewBox] get root view box";
    return context_->GetRootViewBox();
}

void SvgNode::OnClipPath(OH_Drawing_Canvas *canvas) {
    LOG(INFO) << "[SvgNode] Draw OnClipPath enter";
    if (!context_) {
        LOG(WARNING) << "[SvgNode] OnClipPath: Context is null!";
        return;
    }
    auto refSvgNode = context_->GetSvgNodeById(hrefClipPath_);
    if (!refSvgNode) {
        LOG(WARNING) << "[SvgNode] OnClipPath: SvgNode is null!";
        return;
    };
    auto *clipPath = refSvgNode->AsPath();
    if (!clipPath) {
        LOG(WARNING) << "[SvgNode] OnClipPath: Path is null!";
        return;
    };
    OH_Drawing_CanvasClipPath(canvas, clipPath, OH_Drawing_CanvasClipOp::INTERSECT, true);
    OH_Drawing_PathDestroy(clipPath);
}

void SvgNode::OnMask(OH_Drawing_Canvas *canvas) {
    if (!context_) {
        LOG(INFO) << "NO CONTEXT";
        return;
    }
    auto refMask = context_->GetSvgNodeById(attributes_.maskId);
    if (!refMask) {
        return;
    };
    refMask->Draw(canvas);
}

void SvgNode::OnTransform(OH_Drawing_Canvas *canvas) {
    // input transfrom: (float scaleX, float skewY, float skewX, float scaleY, float transX, float transY)
    const auto &transform = attributes_.transform;
    auto *matrix = OH_Drawing_MatrixCreate();
    /*
    /* (OH_Drawing_Matrix* , float scaleX, float skewX, float transX, float skewY, float scaleY, float transY, float persp0, float persp1, float persp2 )
    */
    OH_Drawing_MatrixSetMatrix(matrix, transform[0], transform[2], transform[4] * scale_, transform[1], transform[3],
                               transform[5] * scale_, 0, 0, 1.0);
    OH_Drawing_CanvasConcatMatrix(canvas, matrix);
    OH_Drawing_MatrixDestroy(matrix);
}

double SvgNode::ConvertDimensionToPx(const Dimension &value, const Size &viewPort, SvgLengthType type) const {
    switch (value.Unit()) {
    case DimensionUnit::PERCENT: {
        if (type == SvgLengthType::HORIZONTAL) {
            return value.Value() * viewPort.Width();
        }
        if (type == SvgLengthType::VERTICAL) {
            return value.Value() * viewPort.Height();
        }
        if (type == SvgLengthType::OTHER) {
            return value.Value() * sqrt(viewPort.Width() * viewPort.Height());
        }
        return 0.0;
    }
    case DimensionUnit::PX:
        return value.Value();
    case DimensionUnit::VP:
        return vpToPx(value.Value());
    default:
        return vpToPx(value.Value());
    }
}

double SvgNode::ConvertDimensionToPx(const Dimension& value, double baseValue) const
{
    return value.Value() * baseValue;
}

std::optional<Gradient> SvgNode::GetGradient(const std::string& href)
{
    if (!context_) {
        LOG(INFO) << "NO CONTEXT";
        return std::nullopt;
    }
    auto refSvgNode = context_->GetSvgNodeById(href);
    CHECK_NULL_RETURN(refSvgNode, std::nullopt);
    auto svgGradient = std::dynamic_pointer_cast<SvgGradient>(refSvgNode);
    if (svgGradient) {
        return std::make_optional(svgGradient->GetGradient());
    }
    return std::nullopt;
}

void SvgNode::Draw(OH_Drawing_Canvas *canvas) {
    // mask and filter create extra layers, need to record initial layer count
    LOG(INFO) << "[SvgNode] Draw enter";
    const auto count = OH_Drawing_CanvasGetSaveCount(canvas);
    OH_Drawing_CanvasSave(canvas);
    if (!hrefClipPath_.empty()) {
        OnClipPath(canvas);
    }
    if (!attributes_.transform.empty()) {
        OnTransform(canvas);
    }
    if (!attributes_.maskId.empty()) {
        OnMask(canvas);
    }

    OnDraw(canvas);
    // on marker

    OnDrawTraversed(canvas);
    OH_Drawing_CanvasRestoreToCount(canvas, count);
}

void SvgNode::UpdateCommonProps(const ConcreteProps &props) {
    attributes_.id = props->name;

    if (hrefRender_) {
        attributes_.transform = props->matrix;
        attributes_.maskId = props->mask;
        attributes_.selfOpacity = props->opacity;
        attributes_.markerStart = props->markerStart;
        attributes_.markerMid = props->markerMid;
        attributes_.markerEnd = props->markerEnd;
        // clipPath
        attributes_.clipPath = props->clipPath;
        hrefClipPath_ = props->clipPath;
    }

    std::unordered_set<std::string> set;

    for (const auto& prop : props->propList) {
        LOG(INFO) << "[UpdateCommonProps] insert prop:" << prop;
        set.insert(prop);
    }
    attributes_.fillState.SetColor(Color((uint32_t)*props->fill.payload), set.count("fill"));
    attributes_.fillState.SetOpacity(std::clamp(props->fillOpacity, 0.0, 1.0), set.count("fillOpacity"));
    attributes_.fillState.SetFillRule(std::to_string(props->fillRule), set.count("fillRule"));
    attributes_.fillState.SetHref(props->fill.brushRef);
    LOG(INFO) << "[UpdateCommonProps] fill brushRef:" << props->fill.brushRef;

    attributes_.strokeState.SetColor(Color((uint32_t)*props->stroke.payload), set.count("stroke"));
    attributes_.strokeState.SetLineWidth(vpToPx(StringUtils::StringToDouble(props->strokeWidth)), set.count("strokeWidth"));
    attributes_.strokeState.SetStrokeDashArray(StringUtils::stringVectorToDoubleVector(props->strokeDasharray),
                                               set.count("strokeDasharray"));
    attributes_.strokeState.SetStrokeDashOffset(vpToPx(props->strokeDashoffset), set.count("strokeDashoffset"));
    attributes_.strokeState.SetLineCap(SvgAttributesParser::GetLineCapStyle(std::to_string(props->strokeLinecap)),
                                       set.count("strokeLinecap"));
    attributes_.strokeState.SetLineJoin(SvgAttributesParser::GetLineJoinStyle(std::to_string(props->strokeLinejoin)),
                                        set.count("strokeLinejoin"));
    auto limit = vpToPx(props->strokeMiterlimit);
    if (GreatOrEqual(limit, 1.0)) {
        attributes_.strokeState.SetMiterLimit(limit, set.count("strokeMiterlimit"));
    }
    attributes_.strokeState.SetOpacity(std::clamp(props->strokeOpacity, 0.0, 1.0), set.count("strokeOpacity"));
}

void SvgNode::ContextTraversal() {
    if (!attributes_.id.empty()) {
        context_->Push(attributes_.id, shared_from_this());
    }
    for (const auto &child : children_) {
        child->SetContext(context_);
        child->ContextTraversal();
    }
}
} // namespace rnoh