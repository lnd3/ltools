#include "audio/PortAudio.h"

#include "logging/LoggingAll.h"

#include <stdio.h>
#include <math.h>
#include "portaudio.h"


namespace l::audio {

#define SAMPLE_RATE         (44100)
#define PA_SAMPLE_TYPE      paFloat32

    static int gNumNoInputs = 0;
    static int audioCallback(const void*, void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData)
    {
        float* out = (float*)outputBuffer;
        unsigned int i;
        (void)statusFlags;

        AudioStreamData* audioStreamData = nullptr;

        if (userData == nullptr) {
            for (i = 0; i < framesPerBuffer; i++) {
                *out++ = 0;  /* left - silent */
                *out++ = 0;  /* right - silent */
            }
            gNumNoInputs += 1;
        }
        else {
            //paUserData = reinterpret_cast<PAUserData*>(userData);
            audioStreamData = reinterpret_cast<AudioStreamData*>(userData);
            if (audioStreamData != nullptr) {
                audioStreamData->mDacOutputTimeAtLastCallback = timeInfo->outputBufferDacTime;
                ASSERT(audioStreamData->mDacFramesPerBufferPart == static_cast<int32_t>(framesPerBuffer));

                float* buffer = audioStreamData->GetCurrentBufferPosition();
                
                audioStreamData->NextPart();
                audioStreamData->NextPartCanBeWritten();

                for (i = 0; i < framesPerBuffer; i++) {
                    *out++ = *buffer++;
                    *out++ = *buffer++;
                }
            }
        }

        return paContinue;
    }

    bool AudioStream::OpenStream(int32_t dacFramesPerBufferPart, float latencyMs, BufferingMode mode, ChannelMode channel) {

        mOutputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
        if (mOutputParameters.device == paNoDevice) {
            LOG(LogError) << "Error: No default output device.";
            return false;
        }

        switch (channel) {
        case ChannelMode::MONO:
            mOutputParameters.channelCount = 1;
            break;
        case ChannelMode::STEREO:
        default:
            mOutputParameters.channelCount = 2;
            break;
        }

        mOutputParameters.sampleFormat = PA_SAMPLE_TYPE;
        PaTime defaultLatency = Pa_GetDeviceInfo(mOutputParameters.device)->defaultLowOutputLatency;
        ASSERT(latencyMs >= 0.0f && latencyMs < 1000.0f) << "Latency is allowed between 0ms to 1000ms";
        if (latencyMs > 0.0f) {
            mOutputParameters.suggestedLatency = latencyMs / 1000.0f;
            LOG(LogInfo) << "Port Audio set latency ms: " << static_cast<int32_t>(latencyMs);
        }
        else {
            LOG(LogInfo) << "Port Audio recommended latency ms: " << static_cast<int32_t>(defaultLatency * 1000.0f);
            mOutputParameters.suggestedLatency = defaultLatency;
        }
        mOutputParameters.hostApiSpecificStreamInfo = NULL;

        switch (mode) {
        case BufferingMode::SINGLE_BUFFERING:
            mAudioStreamData.mNumBufferParts = 1;
            break;
        default:
        case BufferingMode::DOUBLE_BUFFERING:
            mAudioStreamData.mNumBufferParts = 2;
            break;
        case BufferingMode::TRIPLE_BUFFERING:
            mAudioStreamData.mNumBufferParts = 3;
            break;
        }
        
        mAudioStreamData.mSampleRate = SAMPLE_RATE;
        mAudioStreamData.mDacFramesPerBufferPart = dacFramesPerBufferPart;
        int32_t totalNumFrames = mAudioStreamData.mNumBufferParts * mAudioStreamData.mDacFramesPerBufferPart;
        mAudioStreamData.mTotalNumFrames = totalNumFrames;
        mAudioStreamData.mTotalBufferSize = totalNumFrames * mOutputParameters.channelCount;
        mAudioStreamData.mNumChannels = static_cast<int32_t>(mOutputParameters.channelCount);

        mOutputBufferInterleaved.resize(mAudioStreamData.mTotalBufferSize);
        mAudioStreamData.mBuffer = mOutputBufferInterleaved.data();

        auto err = Pa_OpenStream(
            &mPaStream,
            NULL,
            &mOutputParameters,
            SAMPLE_RATE,
            mAudioStreamData.mDacFramesPerBufferPart,
            0, /* paClipOff, */
            audioCallback,
            &mAudioStreamData);

        if (err != paNoError) {
            LOG(LogError) << "Failed to open stream: " << err;
            return false;
        }
        return true;
    }


    bool AudioStream::StartStream() {
        auto err = Pa_StartStream(mPaStream);
        if (err != paNoError) {
            LOG(LogError) << "Failed to start stream: " << err;
            return false;
        }

        auto bufferSize = static_cast<size_t>(GetPartTotalSize());
        if (mWriteBuffer.size() != bufferSize) {
            mWriteBuffer.resize(bufferSize);
        }

        return true;
    }

    std::vector<float>& AudioStream::GetWriteBuffer() {
        return mWriteBuffer;
    }

    bool AudioStream::CanWrite() {
        return mAudioStreamData.CanWrite();
    }

    void AudioStream::Write() {
        float* buffer = mAudioStreamData.GetCurrentBufferPosition();

        float* outPtr = mWriteBuffer.data();
        for (int i = 0; i < mAudioStreamData.mDacFramesPerBufferPart; i++) {
            *buffer++ = *outPtr++;
            *buffer++ = *outPtr++;
        }
    }

    bool AudioStream::StopStream() {
        if (mPaStream == nullptr) {
            return false;
        }
        auto err = Pa_CloseStream(mPaStream);
        if (err != paNoError) {
            LOG(LogError) << "Failed to close stream: " << err;
            return false;
        }
        return true;
    }

    int32_t AudioStream::GetPartTotalSize() {
        return mAudioStreamData.GetPartTotalSize();
    }

    int32_t AudioStream::GetNumFramesPerPart() {
        return mAudioStreamData.GetNumFramesPerPart();
    }

    int32_t AudioStream::GetSampleRate() {
        return mAudioStreamData.GetSampleRate();
    }

    AudioManager::~AudioManager() {
        Pa_Terminate();
    }

    bool AudioManager::Init() {
        auto err = Pa_Initialize();
        if (err != paNoError) {
            LOG(LogError) << "Failed to initialize port audio.";
            return false;
        }
        return true;
    }

    AudioStream* AudioManager::GetStream(std::string_view name) {
        auto h = std::hash<std::string_view>{}(name);
        if (!mStreams.contains(h)) {
            mStreams.insert_or_assign(h, std::make_unique<AudioStream>());
        }
        return mStreams.at(h).get();
    }

    void AudioManager::CloseStream(std::string_view name) {
        auto h = std::hash<std::string_view>{}(name);
        mStreams.erase(h);
    }

}

