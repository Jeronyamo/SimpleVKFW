#ifndef SVKFW_FORK_H
#define SVKFW_FORK_H

// Note: linux-specific, no acceptable way to make it work on Windows.
// TODO: (?)

#include <unistd.h>


namespace Simple {
    typedef enum ForkPipeDescriptorsUsed {
        FORK_CHILD_R = 1,
        FORK_CHILD_W = 2,
        FORK_CHILD_RW = 3,
        FORK_CHILD_R = 4,
        FORK_CHILD_W = 8,
        FORK_CHILD_RW = 12
    } ForkPipeDescriptorsUsed; // ForkPipeDescriptorsUsed END


    struct Pipe {
        int pipe_fdescriptors[2]{0,0};

        Pipe(bool _get_fdescriptors = true) {
            
        }

        void getDescriptors() {
            // _pipe();
        }
    }; // Pipe END
}; // Simple END

#endif