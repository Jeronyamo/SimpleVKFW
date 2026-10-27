#ifndef SVKFW_MEMORY_H
#define SVKFW_MEMORY_H

#include <cstdlib>


namespace Simple {
// Safe memory deallocation.
    template <typename T>
    inline void safeDelete(T *&_ptr) {
        if (_ptr != nullptr)
            delete _ptr;
        _ptr = nullptr;
    }
// Safe memory array deallocation.
    template <typename T>
    inline void safeDeleteArr(T *&_ptr) {
        if (_ptr != nullptr)
            delete[] _ptr;
        _ptr = nullptr;
    }
// Safe memory array deallocation. Checks if array size is not zero.
    template <typename T>
    inline void safeDeleteArr(T *&_ptr, size_t _size) {
        if (_size && _ptr != nullptr)
            delete[] _ptr;
        _ptr = nullptr;
    }

// Safe void memory deallocation (with explicit pointer cast).
    template <typename T>
    inline void safeDeleteVoid(const void *&_ptr) {
        if (_ptr != nullptr)
            delete (T*) _ptr;
        _ptr = nullptr;
    }
// Safe void memory array deallocation (with explicit pointer cast).
    template <typename T>
    inline void safeDeleteVoidArr(const void *&_ptr) {
        if (_ptr != nullptr)
            delete[] (T*) _ptr;
        _ptr = nullptr;
    }

// A deleter for unique_ptr/shared_ptr pointers, when memory is provided by malloc.
    struct FreeDeleter { void operator()(void *ptr) const { free(ptr); } };
}; // Simple END

#endif