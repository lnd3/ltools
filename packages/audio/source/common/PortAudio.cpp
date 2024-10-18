#include "audio/PortAudio.h"

#include "logging/LoggingAll.h"

#include <stdio.h>
#include <math.h>
#include "portaudio.h"


namespace l::audio {

#define SAMPLE_RATE         (44100)
#define PA_SAMPLE_TYPE      paFloat32

    static int gNumNoInputs = 0;
    static int audioCallback(const void* inputBuffer, void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData)
    {
        float* in = (float*)inputBuffer;
        float* out = (float*)outputBuffer;
        (void)statusFlags;

        AudioStreamData* audioStreamData = nullptr;

        if (userData == nullptr) {
            for (uint32_t i = 0; i < framesPerBuffer; i++) {
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

                float* outbuffer = audioStreamData->GetCurrentOutputPosition();
                float* inbuffer = audioStreamData->GetCurrentInputPosition();

                audioStreamData->NextPart();
                audioStreamData->NextPartCanBeWritten();

                if (outbuffer != nullptr) {
                    for (uint32_t i = 0; i < framesPerBuffer; i++) {
                        *out++ = *outbuffer++;
                        *out++ = *outbuffer++;
                    }
                }
                else {
                    for (uint32_t i = 0; i < framesPerBuffer; i++) {
                        *out++ = 0;
                        *out++ = 0;
                    }
                }
                if (inbuffer != nullptr) {
                    for (uint32_t i = 0; i < framesPerBuffer; i++) {
                        *inbuffer++ = *in++;
                        *inbuffer++ = *in++;
                    }
                }
            }
        }

        return paContinue;
    }

    bool AudioStream::OpenStream(int32_t dacFramesPerBufferPart, float latencyMs, BufferingMode mode, ChannelMode channel) {
        auto maxDevices = Pa_GetDeviceCount();
        for (int32_t index = 0; index < maxDevices; index++) {
            auto deviceInfo = Pa_GetDeviceInfo(index);
            LOG(LogInfo) << "Audio device " << index << ": " << deviceInfo->name;
        }

        mInputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
        mOutputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
        if (mOutputParameters.device == paNoDevice) {
            LOG(LogError) << "Error: No default output device.";
            return false;
        }

        switch (channel) {
        case ChannelMode::MONO:
            mOutputParameters.channelCount = 1;
            mInputParameters.channelCount = 1;
            break;
        case ChannelMode::STEREO:
        default:
            mOutputParameters.channelCount = 2;
            mInputParameters.channelCount = 2;
            break;
        }

        mInputParameters.sampleFormat = PA_SAMPLE_TYPE;
        mOutputParameters.sampleFormat = PA_SAMPLE_TYPE;
        PaTime defaultLatency = Pa_GetDeviceInfo(mOutputParameters.device)->defaultLowOutputLatency;
        ASSERT(latencyMs >= 0.0f && latencyMs < 1000.0f) << "Latency is allowed between 0ms to 1000ms";
        if (latencyMs > 0.0f) {
            mOutputParameters.suggestedLatency = latencyMs / 1000.0f;

            //auto inputInfo = Pa_GetDeviceInfo(mInputParameters.device);
            mInputParameters.suggestedLatency = latencyMs / 1000.0f; // inputInfo->defaultHighInputLatency;
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
        mInputBufferInterleaved.resize(mAudioStreamData.mTotalBufferSize);
        mAudioStreamData.mOutputBuffer = mOutputBufferInterleaved.data();
        mAudioStreamData.mInputBuffer = mInputBufferInterleaved.data();


        auto err = Pa_OpenStream(
            &mPaStream,
            &mInputParameters,
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
        if (mReadBuffer.size() != bufferSize) {
            mReadBuffer.resize(bufferSize);
        }

        return true;
    }

    std::vector<float>& AudioStream::GetWriteBuffer() {
        return mWriteBuffer;
    }

    std::vector<float>& AudioStream::GetReadBuffer() {
        return mReadBuffer;
    }

    bool AudioStream::CanWrite() {
        return mAudioStreamData.CanWrite();
    }

    void AudioStream::Write() {
        float* buffer = mAudioStreamData.GetCurrentOutputPosition();

        float* outPtr = mWriteBuffer.data();
        for (int i = 0; i < mAudioStreamData.mDacFramesPerBufferPart; i++) {
            *buffer++ = *outPtr++;
            *buffer++ = *outPtr++;
        }
    }

    void AudioStream::Read() {
        float* buffer = mAudioStreamData.GetCurrentInputPosition();

        float* inPtr = mReadBuffer.data();
        for (int i = 0; i < mAudioStreamData.mDacFramesPerBufferPart; i++) {
            *inPtr++ = *buffer++;
            *inPtr++ = *buffer++;
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

    AudioStream* AudioManager::GetAudioStream(std::string_view name) {
        auto h = std::hash<std::string_view>{}(name);
        if (!mStreams.contains(h)) {
            mStreams.insert_or_assign(h, std::make_unique<AudioStream>());
        }
        return mStreams.at(h).get();
    }

    void AudioManager::CloseOutStream(std::string_view name) {
        auto h = std::hash<std::string_view>{}(name);
        mStreams.erase(h);
    }

}

