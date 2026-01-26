#ifndef SVKFW_SOUND_H
#define SVKFW_SOUND_H

#include "sound/samples.h"
#include "data/filetypes.h"


namespace Simple {
    namespace Sound {
        struct SmpFile : Sample::SmpItf {
                virtual float sample() { return   0.f; }
                virtual bool  finite() { return false; }
                virtual bool  isdone() { return false; }
                virtual void   start() {}
                virtual void   pause() {}
                virtual void   reset() {}
        }; // SmpFile END
    }; // Sound END
}; // Simple END

#endif