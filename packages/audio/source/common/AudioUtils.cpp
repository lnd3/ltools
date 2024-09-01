#include "audio/AudioUtils.h"

#include "logging/LoggingAll.h"

#include <math.h>

namespace l::audio {

    float GetFrequencyFromNote(float note) {
        return 440.0f * powf(2.0f, (note - 49.0f) / 12.0f);
    }
}
