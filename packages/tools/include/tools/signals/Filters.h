#pragma once

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>

namespace l::signals {

    template<class T, size_t MaxSize = 4>
    class SignalBase {
    public:
        SignalBase() = default;
        virtual ~SignalBase() = default;

        virtual void Reset() = 0;
        virtual T Process(T inVal) = 0;

        virtual void Set(int32_t i, T value) {
            ASSERT(i >= 0 && i < MaxSize);
            mData[i] = value;
        }

        virtual T& Get(int32_t i) {
            ASSERT(i >= 0 && i < MaxSize);
            return mData[i];
        }

        int32_t mSize;
        std::array<T, MaxSize> mData;
    };

    template<class T, class = std::enable_if_t<std::is_floating_point_v<T>>>
    class SignalProcessor {
    public:
        SignalProcessor() = default;
        virtual ~SignalProcessor() = default;

        virtual void Reset() {
            for (auto& entry : mFilters) {
                entry->Reset();
            }
        }

        virtual void Set(int32_t index, int32_t id, T value) {
            mFilters.at(index)->Set(id, value);
        }

        virtual T Process(T inVal) {
            T out = 0.0;
            for (auto& entry : mFilters) {
                auto nextValue = entry->Process(inVal);
                out += nextValue;
            }
            return out;
        }

        template<class V, class = std::enable_if_t<std::is_base_of_v<SignalBase<T>, V>>, class ...Params>
        SignalProcessor<T>& Chain(Params&&... params) {
            mFilters.push_back(std::make_unique<V>(std::forward<Params&&>(params)...));
            return *this;
        }

    protected:
        std::vector<std::unique_ptr<SignalBase<T>>> mFilters;
    };

    template<class T>
    class SignalAdd : public SignalBase<T> {
    public:
        SignalAdd(T value = 0.0) {
            this->mData[0] = value;
        }
        ~SignalAdd() = default;

        void Reset() {}

        T Process(T inVal) {
            return inVal + this->mData[0];
        }
    };

    template<class T>
    class SignalMul : public SignalBase<T> {
    public:
        SignalMul(T value = 0.0) {
            this->mData[0] = value;
        }
        ~SignalMul() = default;

        void Reset() {}

        T Process(T inVal) {
            return inVal * this->mData[0];
        }
    };

    template<class T>
    class SignalQuotient : public SignalBase<T> {
    public:
        SignalQuotient() = default;
        ~SignalQuotient() = default;

        void Reset() {
            this->mData[0] = 0.0;
        }

        T Process(T inVal) {
            auto prevValue = this->mData[0];

            if (prevValue == 0.0) {
                prevValue = inVal;
            }
            T out = 0.0;
            if (inVal > 0.0 && prevValue > 0.0) {
                out = inVal / prevValue;
            }
            else if (inVal < 0.0 && prevValue > 0.0){
                out = inVal / prevValue;
            }
            else if (inVal < 0.0 && prevValue < 0.0) {
                out = - inVal / (prevValue);
            }
            else {
                out = - inVal / (prevValue);
            }
            this->mData[0] = inVal;
            return out;
        }
    };

    template<class T>
    class SignalIntegral : public SignalBase<T> {
    public:
        SignalIntegral() = default;
        ~SignalIntegral() = default;

        void Reset() {
            this->mData[0] = 0.0;
        }

        T Process(T inVal) {
            this->mData[0] += inVal;
            return this->mData[0];
        }
    };

    template<class T>
    class SignalLowpass : public SignalBase<T> {
    public:
        SignalLowpass() {
            this->mData[0] = 1.0;
            this->mData[1] = 0.5;
            Reset();
        }
        ~SignalLowpass() = default;

        void Reset() {
            mFilterState[0] = 0.0;
            mFilterState[1] = 0.0;
        }

        T Process(T inVal) {
            T mCutoff = this->mData[0];
            T mResonance = this->mData[1];
            T cutoff = mCutoff * mCutoff;
            double rc = 1 - mResonance * cutoff;
            mFilterState[0] = rc * mFilterState[0] - cutoff * (mFilterState[1] + inVal);
            mFilterState[1] = rc * mFilterState[1] + cutoff * mFilterState[0];
            return -mFilterState[1];
        }

    protected:
        T mFilterState[2] = { 0.0 };
    };

    template<class T>
    class SignalBandpass : public SignalBase<T> {
    public:
        SignalBandpass() {
            this->mData[0] = 0.98;
            this->mData[1] = 0.01;
            this->mData[2] = 0.5;
            Reset();
        }
        ~SignalBandpass() = default;

        void Reset() {
            mFilterState[0] = 0.0;
            mFilterState[1] = 0.0;
            mFilterState[2] = 0.0;
            mFilterState[3] = 0.0;
        }

        T Process(T inVal) {
            auto mCutoffHigh = this->mData[0];
            auto mCutoffLow = this->mData[1];
            auto mResonance = this->mData[2];

            T fbLp = mResonance + mResonance / (1.0 - mCutoffHigh);
            T fbHp = mResonance + mResonance / (1.0 - mCutoffLow);

            T n1_2 = mFilterState[0] - mFilterState[1];
            T n2_3 = mFilterState[1] - mFilterState[2];
            T n3_4 = mFilterState[2] - mFilterState[3];

            mFilterState[0] += mCutoffHigh * (inVal - mFilterState[0] + fbLp * n1_2);
            mFilterState[1] += mCutoffHigh * n1_2;
            mFilterState[2] += mCutoffLow * (n2_3 + fbHp * n3_4);
            mFilterState[3] += mCutoffLow * n3_4;

            return mFilterState[3] - mFilterState[1];
        }

    protected:
        T mFilterState[4] = { 0.0 };
    };

    template<class T>
    class SignalMovingAverage : public SignalBase<T> {
    public:
        SignalMovingAverage(int32_t filterKernelSize = 50) :
            mFilterStateIndex(0)
        {
            this->mData[0] = 0.999;
            this->mData[1] = static_cast<T>(filterKernelSize);
            mFilterState.resize(filterKernelSize);
            Reset();
        }
        ~SignalMovingAverage() = default;

        void Reset() {
            auto kernelSize = static_cast<int32_t>(this->mData[1]);
            for (int32_t i = 0; i < kernelSize; i++) {
                mFilterState[i] = 0.0;
            }
        }

        void Set(int32_t type, T value) {
            SignalBase<T>::Set(type, value);
            UpdateKernelSize();
        }
        void UpdateKernelSize() {
            auto kernelSize = static_cast<int32_t>(this->mData[1]);
            if (kernelSize != mFilterState.size()) {
                mFilterState.resize(kernelSize);
                Reset();
            }
        }

        T Process(T inVal) {
            T widthModifier = this->mData[0];
            auto kernelSize = static_cast<int32_t>(this->mData[1]);
            int32_t width = 1 + static_cast<int32_t>(widthModifier * (kernelSize - 2));
            mFilterStateIndex = (mFilterStateIndex + 1) % width;
            mFilterState[mFilterStateIndex] = inVal;

            T outVal = 0.0;
            for (int32_t i = 0; i < width; i++) {
                outVal += mFilterState[i];
            }
            return outVal / static_cast<double>(width);
        }

    protected:
        int32_t mFilterStateIndex;
        std::vector<T> mFilterState;
    };


}
