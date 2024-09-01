#include "audio/Audio.h"

#include <math.h>

#ifdef USE_AUDIO_MA
#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION

#include "various/miniaudio.h"


namespace l::audio {
    float GetFrequencyFromNote(float note) {
        return 440.0f * powf(2.0f, (note - 49.0f) / 12.0f);
    }


#define DEVICE_FORMAT       ma_format_f32
#define DEVICE_CHANNELS     2
#define DEVICE_SAMPLE_RATE  48000

    ma_device_config deviceConfig;
    ma_device device;

    void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
    {
        ma_waveform* pSineWave;

        MA_ASSERT(pDevice->playback.channels == DEVICE_CHANNELS);

        pSineWave = (ma_waveform*)pDevice->pUserData;
        MA_ASSERT(pSineWave != NULL);

        ma_waveform_read_pcm_frames(pSineWave, pOutput, frameCount, NULL);

        (void)pInput;   /* Unused. */
    }


    bool Init() {
        ma_waveform sineWave;
        ma_waveform_config sineWaveConfig;

        deviceConfig = ma_device_config_init(ma_device_type_playback);
        deviceConfig.playback.format = DEVICE_FORMAT;
        deviceConfig.playback.channels = DEVICE_CHANNELS;
        deviceConfig.sampleRate = DEVICE_SAMPLE_RATE;
        deviceConfig.dataCallback = data_callback;
        deviceConfig.pUserData = &sineWave;

        if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
            printf("Failed to open playback device.\n");
            return false;
        }
        printf("Device Name: %s\n", device.playback.name);

        sineWaveConfig = ma_waveform_config_init(device.playback.format, device.playback.channels, device.sampleRate, ma_waveform_type_sine, 0.1, 13000);
        ma_waveform_init(&sineWaveConfig, &sineWave);

        if (ma_device_start(&device) != MA_SUCCESS) {
            printf("Failed to start playback device.\n");
            ma_device_uninit(&device);
            return false;
        }
        return true;
    }

    void Deinit() {
        ma_device_uninit(&device);
    }
}

#else


namespace l::audio {
    float GetFrequencyFromNote(float note) {
        return 440.0f * powf(2.0f, (note - 49.0f) / 12.0f);
    }

    bool Init() {


        return true;
    }

    void Deinit() {

    }

}
#endif

