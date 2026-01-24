#pragma once

#include <logging/LoggingAll.h>
#include <math/MathFunc.h>

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace l::math::fp {

    class FixedPoint {
    public:
        static FixedPoint FromDouble(double d, int64_t scale) {
            return FixedPoint(static_cast<int64_t>(l::math::round(d * scale)), scale);
        }

        static FixedPoint FromFloat(float f, int64_t scale) {
            return FixedPoint(static_cast<int64_t>(l::math::round(f * scale)), scale);
        }

        static FixedPoint FromString(std::string_view number, int64_t scale) {
            return FromDouble(std::atof(number.data()), scale);
        }

        static int64_t GetMinScaleFromDouble(double scaleDouble) {
            if (scaleDouble == 0.0) {
                return 0;
            }
            int32_t numDecimals = 0;
            while (scaleDouble < 1.0 && numDecimals < 18) {
                scaleDouble *= 10.0;
                ++numDecimals;
            }
            if (numDecimals > 18) return false; // avoid int64_t overflow
            auto scale = static_cast<int64_t>(0.5f + l::math::pow(10.0f, static_cast<float>(numDecimals)));
            return scale;
        }

        FixedPoint();
        explicit FixedPoint(int64_t scaledValue, int64_t scale);
        explicit FixedPoint(double value, int32_t numdecimals, bool floorValue = false);
        explicit FixedPoint(double value, bool floorValue = false);
        explicit FixedPoint(float value, int32_t numdecimals, bool floorValue = false);
        explicit FixedPoint(float value, bool floorValue = false);
        FixedPoint(std::string_view number, bool floorValue = false);

        int32_t numDigits() const;
        double toDouble() const;
        float toFloat() const;
        std::string toString() const;

        template<size_t SIZE>
        void getString(l::string::string_buffer<SIZE>& buf) const {
            int64_t int_part = value_ / scale_;
            int64_t frac_part = l::math::abs(value_ % scale_);

            buf.printf("%lld", int_part);

            if (scale_ > 1) {
                int decimal_digits = static_cast<int>(l::math::logx(10.0f, static_cast<float>(scale_)));
                buf.printf(".%0*d", decimal_digits, frac_part);
            }
        }

        void normalise();
        void rescale(int64_t scale, bool truncate = false);
        void round(int32_t numDecimals);
        void floor(int32_t numDecimals);

        // Arithmetic
        FixedPoint operator+(const FixedPoint& other) const {
            FixedPoint tmp(other);
            tmp.rescale(scale_);
            return FixedPoint(value_ + tmp.value_, scale_);
        }

        FixedPoint operator-(const FixedPoint& other) const {
            FixedPoint tmp(other);
            tmp.rescale(scale_);
            return FixedPoint(value_ - tmp.value_, scale_);
        }

        FixedPoint operator*(const FixedPoint& other) const {
            if (other.scale() != scale()) {
                FixedPoint tmp(other);
                tmp.rescale(scale_);
                return FixedPoint(value_ * tmp.value_, scale_ * scale_);
            }
            return FixedPoint(value_ * other.value_, scale_ * scale_);
        }

        FixedPoint operator*(int64_t multiplier) const {
            return FixedPoint(value_ * multiplier, scale_);
        }

        bool operator==(const FixedPoint& other) const {
            FixedPoint tmp(other);
            tmp.rescale(scale_);
            return value_ == tmp.value_ && scale_ == tmp.scale_;
        }

        bool operator!=(const FixedPoint& other) const {
            return !(*this == other);
        }

        bool operator<(const FixedPoint& other) const {
            FixedPoint tmp(other);
            tmp.rescale(scale_);
            return value_ < tmp.value_;
        }

        bool operator>(const FixedPoint& other) const {
            return other < *this;
        }

        bool operator<=(const FixedPoint& other) const {
            return !(*this > other);
        }

        bool operator>=(const FixedPoint& other) const {
            return !(*this < other);
        }

        FixedPoint operator%(const FixedPoint& other) const {
            FixedPoint tmp(other);
            tmp.rescale(scale_);
            return FixedPoint(value_ % tmp.value_, scale_);
        }

        // Getters
        int64_t rawValue() const { return value_; }
        int64_t scale() const { return scale_; }

        private:
            int64_t value_ = 0;
            int64_t scale_ = 1;  // per-instance scale (e.g., 1e8)

    };


}
