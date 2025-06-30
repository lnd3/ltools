#include "testing/Test.h"
#include "logging/Log.h"

#include "nodegraph/core/NodeGraphData.h"
#include "nodegraph/core/NodeGraphInput.h"
#include "nodegraph/core/NodeGraphOutput.h"
#include "nodegraph/core/NodeGraphBase.h"
#include <nodegraph/operations/NodeGraphOpSignalFilter.h>
#include <math/MathAll.h>

using namespace l;
using namespace l::nodegraph;


float MAFilter(float width, float weightAccent, float balance, float gamma) {
    int32_t widthInt = 1 + static_cast<int32_t>(width);
    int32_t bufferSize = widthInt;
    float widthFrac = width - l::math::floor(width);

    float state = 100.0f;
    float inputWeight = 1.0f;

    float outVal = 0.0;
    float balanceFactor = 1.0f - balance;
    float balanceDelta = balance / static_cast<float>(widthInt - 1);
    float balanceDivisorSum = 0.0f;
    float weight = l::math::pow(inputWeight, weightAccent);

    { // remove a part of the first sample of the sum as it is not part of the moving average
        balanceFactor -= balanceDelta * widthFrac;

        auto fac = weight * balanceFactor;
        auto sign = l::math::functions::sign(fac);
        fac = l::math::pow(fac * sign, gamma);
        fac *= sign;
        outVal = fac * state * widthFrac;
        balanceDivisorSum = fac * widthFrac;

        balanceFactor += balanceDelta * widthFrac;
    }
    for (int32_t j = 0; j < bufferSize; j++) {
        float weight = l::math::pow(inputWeight, weightAccent);
        auto fac = weight * balanceFactor;
        auto sign = l::math::functions::sign(fac);
        fac = l::math::pow(fac * sign, gamma);
        fac *= sign;
        outVal += fac * state;
        balanceDivisorSum += fac;
        balanceFactor += balanceDelta;
    }

    {// Check that balanceFactor summed to 1 (but first subtract the last unused addition)
        balanceFactor -= balanceDelta; 
        TEST_FUZZY(balanceFactor, 1.0f, 0.001f, "");
    }

    auto signal = outVal / balanceDivisorSum;

    return signal;
}

TEST(NodeGraphOp, MAFilterSumming) {

    for (int32_t i = 0; i < 3; i++) {
        MAFilter(4.3f + i * 1.3f, 1.0f, 1.5f, 1.0f);
    }
    for (int32_t i = 0; i < 3; i++) {
        auto v0 = MAFilter(4.3f + i * 1.3f, 1.0f, 1.5f, 1.0f);
        auto v1 = MAFilter(4.3f + i * 1.3f, 1.0f, 1.5f, 1.000001f);
        TEST_FUZZY(v0, v1, 0.0001f, "");
    }
    for (int32_t i = 0; i < 3; i++) {
        MAFilter(4.3f + i * 1.3f, 1.4f, 1.5f, 3.0f);
    }

    return 0;
}
