#include "nodegraph/core/NodeGraphData.h"

#include "logging/Log.h"

#include "math/MathFunc.h"

namespace l::nodegraph {

    std::pair<float, float> GetInputBounds(InputBound bound) {
        switch (bound) {
        case InputBound::INPUT_0_TO_1:
            return { 0.0f, 1.0f };
        case InputBound::INPUT_0_TO_2:
            return { 0.0f, 2.0f };
        case InputBound::INPUT_NEG_1_POS_1:
            return { -1.0f, 1.0f };
        case InputBound::INPUT_0_100:
            return { 0.0f, 100.0f };
        case InputBound::INPUT_UNBOUNDED:
            return { -l::math::constants::FLTMAX, l::math::constants::FLTMAX };
        case InputBound::INPUT_CUSTOM:
            return { 0.0f, 0.0f };
        }
        return { 0.0f, 0.0f };
    }

    /*********************************************************************************/

}