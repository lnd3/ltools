#include "audio/Audio.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION

#include "various/miniaudio.h"


namespace l::audio {
    float GetFrequencyFromNote(float note) {
        return 440.0f * powf(2.0f, (note - 49.0f) / 12.0f);
    }


}
