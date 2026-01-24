
#include "math/MathFixedPoint.h"

namespace l::math::fp {


    FixedPoint::FixedPoint() : value_(0), scale_(1) {}
    
    FixedPoint::FixedPoint(int64_t scaledValue, int64_t scale)
        : value_(scaledValue), scale_(scale) {
        normalise();
    }
    
    FixedPoint::FixedPoint(double value, int32_t numdecimals, bool floorValue) {
        auto v = l::math::abs(value);
        if (v < 1.0) {
            scale_ = 1000000000000000000;
            value_ = static_cast<int64_t>(value * scale_);
        }
        else if (v < 1000000000.0) {
            scale_ = 1000000000;
            value_ = static_cast<int64_t>(value * scale_);
        }
        else {
            scale_ = 1;
            value_ = static_cast<int64_t>(value * scale_);
        }
        if (!floorValue) {
            round(numdecimals);
        }
        else {
            floor(numdecimals);
        }
    }
    
    FixedPoint::FixedPoint(double value, bool floorValue) {
        auto v = l::math::abs(value);
        auto numdecimals = 0;
        if (v < 1.0) {
            scale_ = 1000000000000000000;
            value_ = static_cast<int64_t>(value * scale_);
            numdecimals = 9;
        }
        else if (v < 1000000000.0) {
            scale_ = 1000000000;
            value_ = static_cast<int64_t>(value * scale_);
            numdecimals = 9;
        }
        else {
            scale_ = 1;
            value_ = static_cast<int64_t>(value * scale_);
            numdecimals = 0;
        }
        if (!floorValue) {
            round(numdecimals);
        }
        else {
            floor(numdecimals);
        }
    }

    FixedPoint::FixedPoint(float value, int32_t numdecimals, bool floorValue) : FixedPoint(static_cast<double>(value), numdecimals, floorValue) {
    }
    
    FixedPoint::FixedPoint(float value, bool floorValue) : FixedPoint(static_cast<double>(value), floorValue) {
    }

    FixedPoint::FixedPoint(std::string_view number, bool floorValue) {
        auto [n, d, s] = l::string::to_fixed_int(number);
        value_ = n;
        auto scale = static_cast<int64_t>(0.5f + l::math::pow(10.0f, static_cast<float>(d)));
        scale_ = scale;

        if (!floorValue) {
            round(d);
        }
        else {
            floor(d);
        }
    }

    void FixedPoint::normalise() {
        while (scale_ >= 10 && value_ >= 10 && (value_ % 10) == 0) {
            value_ = value_ / 10;
            scale_ = scale_ / 10;
        }
    }

    void FixedPoint::rescale(int64_t newScale, bool truncate) {
        if (newScale > scale_) { // scale up, no loss in precision
            int64_t diff = newScale / scale_;
            //ASSERT(l::math::abs(value_ * diff) < 100000000000000000);
            value_ *= diff;
            scale_ *= diff;
        }
        else if (newScale < scale_) {
            int64_t diff = scale_ / newScale;
            //ASSERT(diff >= 10);
            scale_ /= diff;
            if (!truncate) {
                diff /= 10;
            }
            value_ /= diff;
            if (!truncate) {
                value_ += 5; // add 0.5 before floor
                value_ /= 10; // round (floor(0.5 + x))
            }
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

    void FixedPoint::floor(int32_t numDecimals) {
        int64_t scale = 1;
        while (numDecimals-- > 0) {
            scale *= 10;
        };
        rescale(scale, true);
        normalise();
    }

    int32_t FixedPoint::numDigits() const {
        return static_cast<int>(l::math::logx(10.0f, static_cast<float>(scale_)));
    }

    double FixedPoint::toDouble() const {
        return static_cast<double>(value_) / static_cast<double>(scale_);
    }

    float FixedPoint::toFloat() const {
        return static_cast<float>(toDouble());
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
