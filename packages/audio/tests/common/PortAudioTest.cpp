#include "testing/Test.h"

#include "audio/PortAudio.h"

#include <thread>


TEST(PortAudio, Setup) {

    l::audio::AudioManager manager;
    if (!manager.Init()) {
        return 0;
    }

    auto stream = manager.GetStream("speaker");

    if (!stream->OpenStream(2048, 20.0f, l::audio::BufferingMode::TRIPLE_BUFFERING)) {
        return 0;
    }
    if (!stream->StartStream()) {
        return 0;
    }

    float sinePhase = 0.0f;
    float freq = 300.0f;
    float fMod = 1.0f;
    float pMod = 0.0f;
    float mPhase = 0.0f;

    float pModPhase = 0.0f;

    auto& buffer = stream->GetWriteBuffer();

    float deltaTime = 1.0f / static_cast<float>(stream->GetSampleRate());
    int32_t loops = 10;
    do {
        if (stream->CanWrite()) {
            for (size_t i = 0; i < stream->GetNumFramesPerPart(); i++) {

                pModPhase += deltaTime * freq * (13.0f/12.0f);
                pModPhase -= floorf(pModPhase);

                fMod = 0.3f * sinf(2.0f * 3.141529f * pModPhase);

                mPhase += deltaTime * freq * (fMod + 1.0f);
                mPhase -= floorf(mPhase);

                float phaseMod = mPhase + pMod;
                phaseMod -= floorf(phaseMod);

                buffer.at(2 * i + 0) = 0.015f * sinf(3.141529f * (mPhase + phaseMod));
                buffer.at(2 * i + 1) = 0.015f * sinf(3.141529f * (mPhase + phaseMod));
            }
            stream->Write();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (loops-- > 0);


    if (!stream->StopStream()) {
        LOG(LogError) << "Failed to stop stream";
    }

    manager.CloseStream("speaker");

    return 0;
}


