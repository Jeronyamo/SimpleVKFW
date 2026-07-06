#ifndef SVKFW_PLAYER_H
#define SVKFW_PLAYER_H

#include "data/filetypes.h"
#include "sound/sampler.h"


namespace Simple {
    namespace Audio {
        namespace Sample {
            struct SmpAudioFile : SmpItf {
                SmpAudioFile(const std::string) {}
            };
        };
    }; // Audio END

}; // Simple END

#endif