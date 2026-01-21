#ifndef SVKFW_PD_FILETYPES_H
#define SVKFW_PD_FILETYPES_H

#include "common/terminal.h"

#include <fstream>

#ifdef WIN32
#include <Windows.h>

// Note: before Win10 consolas font must be set manually

namespace Simple {
    namespace WinInner {
        // Taken from: https://stackoverflow.com/questions/23471873/change-console-code-page-in-windows-c
        struct UTF8CodePage {
            UTF8CodePage() : old_code_page(GetConsoleOutputCP()) { SetConsoleOutputCP(CP_UTF8); }
            ~UTF8CodePage() { SetConsoleOutputCP(old_code_page); }

            UINT old_code_page;
        } init_utf8_code_page; // UTF8CodePage END
    }; // WinInner END

    // Windows wrappers

    struct FileBinReaderWin {
        HANDLE f_handle = INVALID_HANDLE_VALUE;

        FileBinReaderWin(const std::string &_fpath = "") { if (!_fpath.empty()) open(_fpath); }
        ~FileBinReaderWin() { if (isOpen()) close(); }

        inline void open(const std::string &_fpath)  {
            // Get WCHAR string to open a file with (possibly) Unicode characters
            // for (char ch : _fpath)
            //     printf("%x ", uint8_t(ch));
            int __wchars_num = MultiByteToWideChar(CP_UTF8, 0, _fpath.c_str(), _fpath.size(), NULL, 0);
            WCHAR* wstr = new wchar_t[__wchars_num+1]{};
            MultiByteToWideChar(CP_UTF8, 0, _fpath.c_str(), _fpath.size(), wstr, __wchars_num);
            // printf("\nPathlen: %d\n", __wchars_num);

            f_handle  = CreateFileW(wstr,                  // file to open
                                    GENERIC_READ,          // open for reading
                                    FILE_SHARE_READ,       // share for reading
                                    NULL,                  // default security
                                    OPEN_EXISTING,         // existing file only
                                    FILE_ATTRIBUTE_NORMAL, // normal file
                                    NULL);                 // no attr. template

            SVKFW_ASSERT(f_handle != INVALID_HANDLE_VALUE, std::runtime_error,
                         "FileBinReaderWin :: open", "could not open file");
            delete[] wstr;
        }
        inline void read(char *_buf, uint32_t _size) {
            DWORD __bytes_was_read = 0;
            if (!ReadFile(f_handle, _buf, _size, &__bytes_was_read, nullptr)) {
                throw std::runtime_error(SVKFW_WRAPERR("FileBinReaderWin :: read", "read error"));
            }
            if (__bytes_was_read == 0 && _size)
                printf(SVKFW_WRAPINFO("FileBinReaderWin :: read", "EOF reached"));
        }
        inline bool isOpen() { return f_handle != INVALID_HANDLE_VALUE; }
        inline void  close() { CloseHandle(f_handle); f_handle = INVALID_HANDLE_VALUE; }


        // The reason this class exists

        // Reads a single element of type T from binary file. Ineffective.
        template <class T>
        T readBinary() {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinReaderWin :: readBinary (1)", "Reading a pointer from file - this is most likely an unwanted behaviour"));

            char *__ch_buf[sizeof(T)]{};
            read((char *)__ch_buf, sizeof(T));
            return *reinterpret_cast<T*>(__ch_buf);
        }

        // Reads a single element of type T from binary file. Ineffective.
        template <class T>
        void readBinary(T &_val) {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinReaderWin :: readBinary (2)", "Reading a pointer from file - this is most likely an unwanted behaviour"));

            char *__ch_buf[sizeof(T)]{};
            read((char *)__ch_buf, sizeof(T));
            _val = *reinterpret_cast<T*>(__ch_buf);
        }


        // Reads an array of type T and length 'len' from binary file.
        // It doesn't handle possible misalignment.
        template <class T>
        void readBinaryArray(T* _buf, size_t _len) {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinReaderWin :: readBinaryArray", "Reading pointers from file - this is most likely an unwanted behaviour"));

            read((char*)_buf, sizeof(T) * _len);
        }
    }; // FileBinReaderWin END

    struct FileBinWriterWin {
        HANDLE f_handle = INVALID_HANDLE_VALUE;

        FileBinWriterWin(const std::string &_fpath = "", bool _truncate = false) { if (!_fpath.empty()) open(_fpath, _truncate); }
        ~FileBinWriterWin() { if (isOpen()) close(); }

        inline void open(const std::string &_fpath, bool _truncate = false)  {
            // Get WCHAR stirng to open a file with (possibly) Unicode characters
            int __wchars_num = MultiByteToWideChar(CP_UTF8, 0, _fpath.c_str(), _fpath.size(), NULL, 0);
            WCHAR* wstr = new wchar_t[__wchars_num+1]{};
            MultiByteToWideChar(CP_UTF8, 0, _fpath.c_str(), _fpath.size(), wstr, __wchars_num);

            f_handle  = CreateFileW(wstr,                   // name of the write
                                    GENERIC_WRITE,          // open for writing
                                    0,                      // do not share
                                    NULL,                   // default security
                                    _truncate ? CREATE_ALWAYS : OPEN_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,  // normal file
                                    NULL);                  // no attr. template

            SVKFW_ASSERT(f_handle != INVALID_HANDLE_VALUE, std::runtime_error,
                         "FileBinWriterWin :: open", "could not open file");
            delete[] wstr;
        }
        inline void write(const char *_buf, uint32_t _size) {
            DWORD __bytes_written = 0;
            if (!WriteFile(f_handle, _buf, _size, &__bytes_written, nullptr)) {
                throw std::runtime_error(SVKFW_WRAPERR("FileBinWriterWin :: write", "write error"));
            }
            if (__bytes_written != _size)
                fprintf(svkfwwarn, "%s [%d/%d]\n", SVKFW_WRAPINFO("FileBinWriterWin :: write", "Could not write all bytes:"), __bytes_written, _size);
        }
        inline bool isOpen() { return f_handle != INVALID_HANDLE_VALUE; }
        inline void  close() { CloseHandle(f_handle); f_handle = INVALID_HANDLE_VALUE; }


        // The reason this class exists

        // Writes a single element of type T to binary file. Ineffective.
        template <class T>
        void writeBinary(const T &_val) {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinReaderWin :: writeBinary", "Writing a pointer - this is most likely an unwanted behaviour"));

            write((const char*)&_val, sizeof(T));
        }

        // Writes an array of type T and length 'len' to binary file.
        template <class T>
        void writeBinaryArray(const T* _buf, size_t _len) {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinReaderWin :: writeBinaryArray", "Writing pointers - this is most likely an unwanted behaviour"));

            write((const char*)_buf, sizeof(T) * _len);
        }
    }; // FileBinWriterWin END

    typedef FileBinReaderWin FileBinReader;
    typedef FileBinWriterWin FileBinWriter;
}; // Simple END

#else

namespace Simple {
    // Standard wrappers

    struct FileBinReaderStd {
        std::ifstream f_handle;

        FileBinReaderStd(const std::string &_fpath = "") { if (!_fpath.empty()) open(_fpath); }
        ~FileBinReaderStd() { if (isOpen()) close(); }

        inline void open(const std::string &_fpath)  { f_handle.open(_fpath, std::ios_base::in | std::ios_base::binary); }
        inline void read(char *_buf, uint32_t _size) { f_handle.read(_buf, _size); }
        inline bool isOpen() { return f_handle.isOpen(); }
        inline void  close() {        f_handle. close(); }


        // The reason this class exists

        // Reads a single element of type T from binary file. Ineffective.
        template <class T>
        T readBinary() {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinReaderStd :: readBinary (1)", "Reading a pointer from file - this is most likely an unwanted behaviour"));

            char *__ch_buf[sizeof(T)]{};
            read((char *)__ch_buf, sizeof(T));
            return *reinterpret_cast<T*>(__ch_buf);
        }

        // Reads a single element of type T from binary file. Ineffective.
        template <class T>
        void readBinary(T &_val) {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinReaderStd :: readBinary (2)", "Reading a pointer from file - this is most likely an unwanted behaviour"));

            char *__ch_buf[sizeof(T)]{};
            read((char *)__ch_buf, sizeof(T));
            _val = *reinterpret_cast<T*>(__ch_buf);
        }

        // Reads an array of type T and length 'len' from binary file.
        // It doesn't handle possible misalignment.
        template <class T>
        void readBinaryArray(T* _buf, size_t _len) {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinReaderStd :: readBinaryArray", "Reading pointers from file - this is most likely an unwanted behaviour"));

            read((char*)_buf, sizeof(T) * _len);
        }
    }; // FileBinReaderStd END

    struct FileBinWriterStd {
        std::ofstream f_handle;

        FileBinWriterStd(const std::string &_fpath = "", bool _truncate = false) { if (!_fpath.empty()) open(_fpath, _truncate); }
        ~FileBinWriterStd() { if (isOpen()) close(); }

        inline void open(const std::string &_fpath, bool _truncate = false) { f_handle.open(_fpath, std::ios_base::out | std::ios_base::binary | (_truncate ? std::ios_base::trunc : (std::ios_base::openmode)0) ); }
        inline void write(const char *_buf, uint32_t _size) { f_handle.write(_buf, _size); }
        inline bool isOpen() { return f_handle.isOpen(); }
        inline bool   good() { return f_handle.  good(); }
        inline void  close() {        f_handle. close(); }


        // The reason this class exists

        // Writes a single element of type T to binary file. Ineffective.
        template <class T>
        void writeBinary(const T &_val) {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinWriterStd :: writeBinary", "Writing a pointer - this is most likely an unwanted behaviour"));

            write((const char*)&_val, sizeof(T));
        }

        // Writes an array of type T and length 'len' to binary file.
        template <class T>
        void writeBinaryArray(const T* _buf, size_t _len) {
            if (std::is_reference<T>::value)
                printf(SVKFW_WRAPWARN("FileBinWriterStd :: writeBinaryArray", "Writing pointers - this is most likely an unwanted behaviour"));

            write((const char*)_buf, sizeof(T) * _len);
        }
    }; // FileBinWriterStd END

    typedef FileBinReaderStd FileBinReader;
    typedef FileBinWriterStd FileBinWriter;
}; // Simple END

#endif

#endif