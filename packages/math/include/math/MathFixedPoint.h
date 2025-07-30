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
        static FixedPoint FromDouble(double d, int64_t scale = 100000000);
        static FixedPoint FromFloat(float f, int64_t scale = 100000000);
        static FixedPoint FromString(std::string_view number, int64_t scale = 100000000);
        static int64_t GetScaleFromString(std::string_view number, int32_t precision_digits = 1);
        static int64_t GetScale(int32_t precision_digits = 1);
        static int64_t GetScaleFromFloat(float scaleFloat, int32_t precision_digits = 1);
        static int64_t GetScaleFromDouble(double scaleDouble, int32_t precision_digits = 1);

        // Constructors
        FixedPoint() : value_(0), scale_(100000000) {}
        FixedPoint(int64_t scaledValue, int64_t scale = 100000000)
            : value_(scaledValue), scale_(scale) {
        }
        FixedPoint(float value, int32_t precision_digits = 1) {
            scale_ = GetScaleFromFloat(value, precision_digits);
            value_ = static_cast<int64_t>(l::math::round(value * scale_));
            normalise(precision_digits);
        }
        FixedPoint(double value, int32_t precision_digits = 1) {
            scale_ = GetScaleFromDouble(value, precision_digits);
            value_ = static_cast<int64_t>(l::math::round(value * scale_));
            normalise(precision_digits);
        }
        FixedPoint(std::string_view number, int32_t precision_digits = 1) {
            scale_ = GetScaleFromString(number, precision_digits);
            auto f = std::atof(number.data());
            value_ = static_cast<int64_t>(l::math::round(f * scale_));
        }

        int32_t numDigits() const;
        double toDouble() const;
        float toFloat() const;
        std::string toString() const;
        void normalise(int32_t precision_digits = 1);

        // Arithmetic
        FixedPoint operator+(const FixedPoint& other) const {
            ASSERT(scale_ == other.scale_);  // Ensure matching scales
            return FixedPoint(value_ + other.value_, scale_);
        }

        FixedPoint operator-(const FixedPoint& other) const {
            ASSERT(scale_ == other.scale_);
            return FixedPoint(value_ - other.value_, scale_);
        }

        FixedPoint operator*(int64_t multiplier) const {
            return FixedPoint(value_ * multiplier, scale_);
        }

        bool operator==(const FixedPoint& other) const {
            return value_ == other.value_ && scale_ == other.scale_;
        }

        bool operator!=(const FixedPoint& other) const {
            return !(*this == other);
        }

        bool operator<(const FixedPoint& other) const {
            ASSERT(scale_ == other.scale_);
            return value_ < other.value_;
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
            ASSERT(scale_ == other.scale_);
            return FixedPoint(value_ % other.value_, scale_);
        }

        // Getters
        int64_t rawValue() const { return value_; }
        int64_t scale() const { return scale_; }

        private:
            int64_t value_ = 0;
            int64_t scale_ = 1;  // per-instance scale (e.g., 1e8)

    };


}
