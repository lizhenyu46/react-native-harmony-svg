/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SVGGradient.h"
#include <native_drawing/drawing_shader_effect.h>

namespace rnoh {

SvgGradient::SvgGradient(GradientType gradientType)
{
    gradientAttr_.gradient.SetType(gradientType);
    InitNoneFlag();
}

void SvgGradient::OnDraw(OH_Drawing_Canvas *canvas) {
}

void SvgGradient::SetAttrX1(const std::string& x1) {
    gradientAttr_.gradient.GetLinearGradient().x1 = SvgAttributesParser::ParseDimension(x1);
}

void SvgGradient::SetAttrY1(const std::string& y1) {
    gradientAttr_.gradient.GetLinearGradient().y1 = SvgAttributesParser::ParseDimension(y1);
}

void SvgGradient::SetAttrX2(const std::string& x2) {
    gradientAttr_.gradient.GetLinearGradient().x2 = SvgAttributesParser::ParseDimension(x2);
}

void SvgGradient::SetAttrY2(const std::string& y2) {
    gradientAttr_.gradient.GetLinearGradient().y2 = SvgAttributesParser::ParseDimension(y2);
}

void SvgGradient::SetAttrFx(const std::string& fx) 
{
    gradientAttr_.gradient.GetRadialGradient().fRadialCenterX = SvgAttributesParser::ParseDimension(fx);
}

void SvgGradient::SetAttrFy(const std::string& fy) 
{
    gradientAttr_.gradient.GetRadialGradient().fRadialCenterY = SvgAttributesParser::ParseDimension(fy);
}

void SvgGradient::SetAttrCx(const std::string& cx) 
{
    gradientAttr_.gradient.GetRadialGradient().radialCenterX = SvgAttributesParser::ParseDimension(cx);
}

void SvgGradient::SetAttrCy(const std::string& cy) 
{
    gradientAttr_.gradient.GetRadialGradient().radialCenterY = SvgAttributesParser::ParseDimension(cy);
}

void SvgGradient::SetAttrRx(const std::string& rx) 
{
    gradientAttr_.gradient.GetRadialGradient().radialHorizontalSize = SvgAttributesParser::ParseDimension(rx);
}

void SvgGradient::SetAttrRy(const std::string& ry) 
{
    gradientAttr_.gradient.GetRadialGradient().radialVerticalSize = SvgAttributesParser::ParseDimension(ry);
}

void SvgGradient::SetAttrGradient(std::vector<double> gradient) {
    auto stopCount = gradient.size() / 2;
    for (auto i = 0; i < stopCount; i++) {
        auto stopIndex = i * 2;
        GradientColor gradientColor;
        gradientColor.SetDimension(Dimension(gradient[stopIndex]));
        gradientColor.SetColor(Color((int32_t)gradient[stopIndex + 1]));
        gradientAttr_.gradient.AddColor(gradientColor);
    }
}

void SvgGradient::SetAttrGradientUnits(int gradientUnits) {
    // gradientAttr_.gradient.setGradientUnits(gradientTransforms);
}

void SvgGradient::SetAttrGradientTransforms(std::vector<double> gradientTransforms) {
    // gradientAttr_.gradient.SetGradientTransform(gradientTransforms);
}

const Gradient& SvgGradient::GetGradient() const
{
    return gradientAttr_.gradient;
}

} // namespace rnoh
