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

        FixedPoint() : value_(0), scale_(1) {}
        FixedPoint(int64_t scaledValue, int64_t scale = 100000000)
            : value_(scaledValue), scale_(scale) {
            normalise();
        }
        FixedPoint(double value, int8_t numdecimals) {
            auto v = l::math::abs(value);
            if (v < 1.0) {
                scale_ = 1000000000000000000;
                value_ = static_cast<int64_t>(l::math::round(value * scale_));
            }
            else if (v < 1000000000.0) {
                scale_ = 1000000000;
                value_ = static_cast<int64_t>(l::math::round(value * scale_));
            }
            else {
                scale_ = 1;
                value_ = static_cast<int64_t>(l::math::round(value * scale_));
            }
            round(numdecimals);
        }
        FixedPoint(double value) {
            auto v = l::math::abs(value);
            if (v < 1.0) {
                scale_ = 1000000000000000000;
                value_ = static_cast<int64_t>(l::math::round(value * scale_));
                round(9);
            }
            else if (v < 1000000000.0) {
                scale_ = 1000000000;
                value_ = static_cast<int64_t>(l::math::round(value * scale_));
                round(9);
            }
            else {
                scale_ = 1;
                value_ = static_cast<int64_t>(l::math::round(value * scale_));
                round(0);
            }
        }
        FixedPoint(float value, int8_t numdecimals) : FixedPoint(static_cast<double>(value), numdecimals) {}
        FixedPoint(float value) : FixedPoint(static_cast<double>(value)) {}

        FixedPoint(std::string_view number) {
            auto [n, d, s] = l::string::to_fixed_int(number);
            value_ = n;
            auto scale = static_cast<int64_t>(0.5f + l::math::pow(10.0f, static_cast<float>(d)));
            scale_ = scale;
            normalise();
        }

        int32_t numDigits() const;
        double toDouble() const;
        float toFloat() const;
        std::string toString() const;
        void normalise();
        void rescale(int64_t scale);
        void round(int32_t numDecimals);

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
