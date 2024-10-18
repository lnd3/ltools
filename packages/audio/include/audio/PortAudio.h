#pragma once

#include <mutex>
#include <atomic>
#include <condition_variable>
#include <semaphore>
#include <vector>
#include <unordered_map>

#include "../../include/portaudio.h"

namespace l::audio {

    struct AudioStreamData {
        double mDacOutputTimeAtLastCallback = 0.0;
        int32_t mSampleRate = 0;
        int32_t mNumBufferParts = 0;
        int32_t mTotalNumFrames = 0;
        int32_t mTotalBufferSize = 0;
        int32_t mFramePosition = 0;
        int32_t mDacFramesPerBufferPart = 0;
        int32_t mNumChannels = 0;
        float* mOutputBuffer = nullptr;
        float* mInputBuffer = nullptr;
        std::binary_semaphore mDacWriteReady{ 0 };

        float* GetCurrentOutputPosition() {
            auto framePos = mFramePosition;
            return mOutputBuffer + framePos * mNumChannels;
        }

        float* GetCurrentInputPosition() {
            auto framePos = mFramePosition;
            return mInputBuffer + framePos * mNumChannels;
        }

        void NextPart() {
            // go to the part just before the one being written
            auto framesAhead = mDacFramesPerBufferPart * (mNumBufferParts - 1);
            mFramePosition = (mFramePosition + framesAhead) % mTotalNumFrames;
        }

        void NextPartCanBeWritten() {
            mDacWriteReady.release();
        }

        bool CanWrite() {
            return mDacWriteReady.try_acquire();
        }

        int32_t GetPartTotalSize() {
            return mDacFramesPerBufferPart * mNumChannels;
        }

        int32_t GetNumFramesPerPart() {
            return mDacFramesPerBufferPart;
        }

        int32_t GetSampleRate() {
            return mSampleRate;
        }
    };

    typedef void PaStream;

    enum class ChannelMode {
        MONO,
        STEREO
    };

    enum class BufferingMode {
        SINGLE_BUFFERING = 0,
        DOUBLE_BUFFERING,
        TRIPLE_BUFFERING
    };

    class AudioStream {
    public:
        AudioStream() = default;
        ~AudioStream() = default;

        bool OpenStream(int32_t dacFramesPerBufferPart, float latencyMs = 0.0f, BufferingMode mode = BufferingMode::DOUBLE_BUFFERING, ChannelMode channel = ChannelMode::STEREO);
        bool StartStream();
        std::vector<float>& GetWriteBuffer();
        std::vector<float>& GetReadBuffer();
        bool CanWrite();
        void Write();
        void Read();
        bool StopStream();
        int32_t GetPartTotalSize();
        int32_t GetNumFramesPerPart();
        int32_t GetSampleRate();
    protected:
        PaStreamParameters mInputParameters{ 0 };
        PaStreamParameters mOutputParameters{ 0 };
        PaStream* mPaStream = nullptr;

        AudioStreamData mAudioStreamData;
        std::vector<float> mOutputBufferInterleaved;
        std::vector<float> mInputBufferInterleaved;
        std::vector<float> mWriteBuffer;
        std::vector<float> mReadBuffer;
    };

    class AudioManager {
    public:
        AudioManager() = default;
        ~AudioManager();

        bool Init();
        AudioStream* GetAudioStream(std::string_view name);
        void CloseOutStream(std::string_view name);
    protected:
        std::unordered_map<size_t, std::unique_ptr<AudioStream>> mStreams;
    };

}
