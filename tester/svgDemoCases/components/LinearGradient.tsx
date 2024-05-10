import React from "react";
// import  from 'react-native-svg'
import { Svg, Circle, Stop } from 'react-native-svg'
import { GenDefs } from './gen'
import { genTransformProps, CaseParams } from '../genUtil'
const basicProps = {
    id: 'myGradient',
}

const basicCases: CaseParams[] = [
    {
        type: 'mulKey',
        id: 'pattern1',
        values: [
            {
                cx: '50%',
                cy: '50%'
            },
            {
                cx: '40%',
                cy: '40%'
            },
            {
                cx: '80%',
                cy: '80%'
            },
        ]
    }
]

const allCases = [
    ...basicCases,
]
const EffectCom = (
    <Circle cx="50" cy="50" r="40" fill={`url(#myGradient)`} />
)
const DefChildren = (
    [
        <Stop offset="30%" stopColor="yellow" />,
        <Stop offset="95%" stopColor="red" />
    ]
)


export default function () {
    return (
        <GenDefs
            cases={allCases}
            basicProps={basicProps}
            renderComChildren={DefChildren}
            EffectCom={EffectCom}
            defName="LinearGradient"
        >
        </GenDefs>
    )
}