#include "common/memory.h"

namespace Simple {

    void FreeDeleter::operator()(void *ptr) const { free(ptr); }
}; // Simple END
