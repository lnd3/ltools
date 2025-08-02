
#include "math/MathFixedPoint.h"

namespace l::math::fp {

    void FixedPoint::normalise() {
        while (scale_ >= 10 && value_ >= 10 && (value_ % 10) == 0) {
            value_ = value_ / 10;
            scale_ = scale_ / 10;
        }
    }

    void FixedPoint::rescale(int64_t newScale) {
        if (newScale > scale_) { // scale up, no loss in precision
            int64_t diff = newScale / scale_;
            ASSERT(l::math::abs(value_ * diff) < 100000000000000000);
            value_ *= diff;
            scale_ *= diff;
        }
        else if (newScale < scale_) {
            int64_t diff = scale_ / newScale;
            ASSERT(diff >= 10);
            scale_ /= diff;
            diff /= 10;
            value_ /= diff;
            value_ += 5; // add 0.5 before floor
            value_ /= 10; // round (floor(0.5 + x))
        }
    }

    void FixedPoint::round(int32_t numDecimals) {
        int64_t scale = 1;
        while (numDecimals-- > 0) {
            scale *= 10;
        };
        rescale(scale);
        normalise();
    }

    int32_t FixedPoint::numDigits() const {
        return static_cast<int>(l::math::logx(10.0f, static_cast<float>(scale_)));
    }

    double FixedPoint::toDouble() const {
        return static_cast<double>(value_) / static_cast<double>(scale_);
    }

    float FixedPoint::toFloat() const {
        return static_cast<float>(value_) / static_cast<float>(scale_);
    }

    std::string FixedPoint::toString() const {
        std::ostringstream oss;

        int64_t int_part = value_ / scale_;
        int64_t frac_part = l::math::abs(value_ % scale_);

        oss << int_part;

        if (scale_ > 1) {
            // Determine number of digits in the scale (e.g., 100000000 -> 8)
            int decimal_digits = static_cast<int>(l::math::logx(10.0f, static_cast<float>(scale_)));

            // Pad with leading zeros if necessary
            oss << "." << std::setw(decimal_digits) << std::setfill('0') << frac_part;
        }

        return oss.str();
    }


}
