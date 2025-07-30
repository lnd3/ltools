
#include "math/MathFixedPoint.h"

namespace l::math::fp {
    FixedPoint FixedPoint::FromDouble(double d, int64_t scale) {
        return FixedPoint(static_cast<int64_t>(l::math::round(d * scale)), scale);
    }

    FixedPoint FixedPoint::FromFloat(float f, int64_t scale) {
        return FixedPoint(static_cast<int64_t>(l::math::round(f * scale)), scale);
    }

    FixedPoint FixedPoint::FromString(std::string_view number, int64_t scale) {
        return FromDouble(std::atof(number.data()), scale);
    }

    int64_t FixedPoint::GetScaleFromString(std::string_view number, int32_t precision_digits) {
        if (number.empty() || precision_digits == 0) {
            return 0;
        }

        auto it = std::find(number.begin(), number.end(), '.');
        if (it == number.end()) {
            return 0; // str contains an integer and doesnt need fixed point
        }


        // Find index of most significant digit
        size_t decimal_start = 1 + it - number.begin();
        size_t first_nonzero = decimal_start;
        while (first_nonzero < number.size() && number[first_nonzero] == '0') {
            ++first_nonzero;
        }

        if (first_nonzero == number.size()) return false; // no non-zero digit found
        auto decimalZeroes = static_cast<int32_t>(first_nonzero - decimal_start);

        int32_t total_digits = decimalZeroes + precision_digits;
        if (total_digits > 18) return false; // avoid int64_t overflow

        auto scale = static_cast<int64_t>(0.5f + l::math::pow(10.0f, static_cast<float>(total_digits)));
        return scale;
    }

    int64_t FixedPoint::GetScaleFromFloat(float scaleFloat, int32_t precision_digits) {
        return GetScaleFromDouble(static_cast<double>(scaleFloat), precision_digits);
    }

    int64_t FixedPoint::GetScaleFromDouble(double scaleDouble, int32_t precision_digits) {
        if (scaleDouble == 0.0 || precision_digits == 0) {
            return 0;
        }

        int32_t decimalZeroes = 0;
        scaleDouble *= 10.0;
        while (scaleDouble < 1.0 && decimalZeroes < 18) {
            scaleDouble *= 10.0;
            ++decimalZeroes;
        }
        int32_t total_digits = decimalZeroes + precision_digits;
        if (total_digits > 18) return false; // avoid int64_t overflow

        auto scale = static_cast<int64_t>(0.5f + l::math::pow(10.0f, static_cast<float>(total_digits)));
        return scale;
    }

    void FixedPoint::normalise(int32_t precision_digits) {
        auto precisionScale = static_cast<int32_t>(0.5 + l::math::pow(10.0, static_cast<double>(precision_digits)));
        while (value_ >= precisionScale) {
            value_ = value_ / precisionScale;
            scale_ = scale_ / 10;
        }
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
