#include "main/fork.h"
#ifndef WIN32
namespace Simple {

    Pipe::Pipe(bool _get_fdescriptors) {

    }

    void Pipe::getDescriptors() {
        // _pipe();
    }
}; // Simple END
#endif