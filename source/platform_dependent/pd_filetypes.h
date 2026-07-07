#ifndef SVKFW_PD_FILETYPES_H
#define SVKFW_PD_FILETYPES_H

#include "common/terminal.h"

#include <fstream>


namespace Simple {
    namespace File {
        struct ReaderItf {
            virtual ~ReaderItf() {}

            virtual bool  open(const std::string  &_fpath) = 0;
            virtual bool  read(char *_buf, uint32_t _size) = 0;
            virtual bool isValid() const = 0;
            virtual bool  isOpen() const = 0;
            virtual void   close()       = 0;
        }; // ReaderItf END

        struct WriterItf {
            virtual ~WriterItf() {}

            virtual bool  open(const std::string  &_fpath, bool _truncate = false) = 0;
            virtual bool write(const char *_buf, uint32_t _size) = 0;
            virtual bool isValid() const = 0;
            virtual bool  isOpen() const = 0;
            virtual void   close()       = 0;
        }; // WriterItf END

        enum InvalidationFlags : uint32_t {
            FILE_VALID           =      0,
            FILE_INVALID_BIT_ERR = 1 << 0,
            FILE_INVALID_BIT_EOF = 1 << 1,
        };

        InvalidationFlags operator|(InvalidationFlags &_flag1, InvalidationFlags _flag2) {
            return (InvalidationFlags) (uint32_t(_flag1) | uint32_t(_flag2));
        }
        InvalidationFlags operator&(InvalidationFlags &_flag1, InvalidationFlags _flag2) {
            return (InvalidationFlags) (uint32_t(_flag1) & uint32_t(_flag2));
        }
        InvalidationFlags operator|=(InvalidationFlags &_flag1, InvalidationFlags _flag2) {
            _flag1 = _flag1 | _flag2;
            return _flag1;
        }
        InvalidationFlags operator&=(InvalidationFlags &_flag1, InvalidationFlags _flag2) {
            _flag1 = _flag1 & _flag2;
            return _flag1;
        }
    }; // File END
}; // Simple END


#ifdef WIN32
#include <Windows.h>

// Note: before Win10 consolas font must be set manually

namespace Simple {
    // Windows wrappers

    namespace File {
        struct ReaderWin : ReaderItf {
            HANDLE f_handle = INVALID_HANDLE_VALUE;
            InvalidationFlags f_state = FILE_VALID;

            ReaderWin(const std::string &_fpath = "") { if (!_fpath.empty()) open(_fpath); }
        ~ReaderWin() { if (isOpen()) close(); }

            virtual bool open(const std::string &_fpath) override {
                // Get WCHAR string to open a file with (possibly) Unicode characters
                int __wchars_num = MultiByteToWideChar(CP_ACP, 0, _fpath.c_str(), _fpath.size(), NULL, 0);
                WCHAR* wstr = new wchar_t[__wchars_num+1]{};
                MultiByteToWideChar(CP_UTF8, 0, _fpath.c_str(), _fpath.size(), wstr, __wchars_num);

                if (isOpen()) close();
                f_handle  = CreateFileW(wstr,                  // file to open
                                        GENERIC_READ,          // open for reading
                                        FILE_SHARE_READ,       // share for reading
                                        NULL,                  // default security
                                        OPEN_EXISTING,         // existing file only
                                        FILE_ATTRIBUTE_NORMAL, // normal file
                                        NULL);                 // no attr. template

                SVKFW_WASSERT(f_handle != INVALID_HANDLE_VALUE, "File :: ReaderWin :: open", "could not open file - possibly it doesn't exist");
                delete[] wstr;
                return isOpen();
            }
            virtual bool read(char *_buf, uint32_t _size) override {
                DWORD __bytes_was_read = 0;

                if (!ReadFile(f_handle, _buf, _size, &__bytes_was_read, nullptr)) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderWin :: read", "read error"));
                    f_state |= FILE_INVALID_BIT_ERR;
                }
                else if (__bytes_was_read < _size) {
                    // printf(SVKFW_WRAPINFO("File :: ReaderWin :: read", "EOF reached"));
                    f_state |= FILE_INVALID_BIT_EOF;
                }
                return isValid();
            }
            virtual inline bool  isOpen() const override { return f_handle !=  INVALID_HANDLE_VALUE; }
            virtual inline bool isValid() const override { return isOpen() && f_state == FILE_VALID; }
            virtual inline void   close()       override { CloseHandle(f_handle); f_handle = INVALID_HANDLE_VALUE; }


        // The reason this class exists

            // Reads a single element of type T from a binary file.
            template <class T>
            T readBinary() {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderWin :: readBinary (1)", "Reading a pointer from file - this is most likely an unwanted behaviour"));

                char *__ch_buf[sizeof(T)]{};
                bool __res = read((char *)__ch_buf, sizeof(T));
                return __res ? *reinterpret_cast<T*>(__ch_buf) : T{};
            }

            // Reads a single element of type T from a binary file.
            template <class T>
            bool readBinary(T &_val) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderWin :: readBinary (2)", "Reading a pointer from file - this is most likely an unwanted behaviour"));

                char *__ch_buf[sizeof(T)]{};
                bool __res = read((char *)__ch_buf, sizeof(T));
                if (__res) _val = *reinterpret_cast<T*>(__ch_buf);
                return __res;
            }

            // Reads an array of type T and length 'len' from binary file.
            // It doesn't handle possible misalignment.
            template <class T>
            bool readBinaryArray(T* _buf, size_t _len) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    printf(SVKFW_WRAPWARN("File :: ReaderWin :: readBinaryArray", "Reading pointers from file - this is most likely an unwanted behaviour"));

                return read((char*)_buf, sizeof(T) * _len);
            }
        }; // ReaderWin END

        struct WriterWin : WriterItf {
            HANDLE f_handle = INVALID_HANDLE_VALUE;
            InvalidationFlags f_state = FILE_VALID;

            WriterWin(const std::string &_fpath = "", bool _truncate = false) { if (!_fpath.empty()) open(_fpath, _truncate); }
        ~WriterWin() { if (isOpen()) close(); }

            virtual bool open(const std::string &_fpath, bool _truncate = false) override {
                // Get WCHAR string to open a file with (possibly) Unicode characters
                int __wchars_num = MultiByteToWideChar(CP_UTF8, 0, _fpath.c_str(), _fpath.size(), NULL, 0);
                WCHAR* wstr = new wchar_t[__wchars_num+1]{};
                MultiByteToWideChar(CP_UTF8, 0, _fpath.c_str(), _fpath.size(), wstr, __wchars_num);

                if (isOpen()) close();
                f_handle  = CreateFileW(wstr,                   // name of the write
                                        GENERIC_WRITE,          // open for writing
                                        0,                      // do not share
                                        NULL,                   // default security
                                        _truncate ? CREATE_ALWAYS : OPEN_ALWAYS,
                                        FILE_ATTRIBUTE_NORMAL,  // normal file
                                        NULL);                  // no attr. template

                SVKFW_WASSERT(f_handle != INVALID_HANDLE_VALUE, "File :: WriterWin :: open", "could not open file - possibly it doesn't exist");
                delete[] wstr;
                return isOpen();
            }
            virtual bool write(const char *_buf, uint32_t _size) override {
                DWORD __bytes_written = 0;
                if (!WriteFile(f_handle, _buf, _size, &__bytes_written, nullptr)) {
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: WriterWin :: write", "write error"));
                    f_state |= FILE_INVALID_BIT_ERR;
                }
                if (__bytes_written != _size) {
                    fprintf(svkfwwarn, "%s [%d/%d]\n", SVKFW_WRAPINFO("File :: WriterWin :: write", "Could not write all bytes:"), __bytes_written, _size);
                    f_state |= FILE_INVALID_BIT_EOF; // TODO: What does that mean?
                }
                return isValid();
            }
            virtual inline bool  isOpen() const override { return f_handle !=  INVALID_HANDLE_VALUE; }
            virtual inline bool isValid() const override { return isOpen() && f_state == FILE_VALID; }
            virtual inline void   close() override { CloseHandle(f_handle); f_handle = INVALID_HANDLE_VALUE; }


            // The reason this class exists

            // Writes a single element of type T to binary file. Ineffective.
            template <class T>
            bool writeBinary(const T &_val) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    printf(SVKFW_WRAPWARN("File :: WriterWin :: writeBinary", "Writing a pointer - this is most likely an unwanted behaviour"));

                return write((const char*)&_val, sizeof(T));
            }

            // Writes an array of type T and length 'len' to binary file.
            template <class T>
            bool writeBinaryArray(const T* _buf, size_t _len) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    printf(SVKFW_WRAPWARN("File :: WriterWin :: writeBinaryArray", "Writing pointers - this is most likely an unwanted behaviour"));

                return write((const char*)_buf, sizeof(T) * _len);
            }
        }; // WriterWin END

        typedef ReaderWin FileReader;
        typedef WriterWin FileWriter;
    }; // File END
}; // Simple END

#endif

namespace Simple {
    // Standard wrappers

    namespace File {
        struct ReaderStd : ReaderItf {
            std::ifstream f_handle;

            ReaderStd(const std::string &_fpath = "") { if (!_fpath.empty()) open(_fpath); }
        ~ReaderStd() { if (isOpen()) close(); }

            virtual inline bool open(const std::string  &_fpath) override {
                f_handle.open(_fpath, std::ios_base::in | std::ios_base::binary);
                return  isOpen();
            }
            virtual inline bool read(char *_buf, uint32_t _size) override {
                f_handle.read(_buf, _size);
                return isValid();
            }
            virtual inline bool  isOpen() const override { return f_handle.is_open(); }
            virtual inline bool isValid() const override { return f_handle.   good(); }
            virtual inline void   close()       override {        f_handle.  close(); }


        // The reason this class exists

            // Reads a single element of type T from binary file. Ineffective.
            template <class T>
            T readBinary() {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    printf(SVKFW_WRAPWARN("File :: ReaderStd :: readBinary (1)", "Reading a pointer from file - this is most likely an unwanted behaviour"));

                char *__ch_buf[sizeof(T)]{};
                bool __res = read((char *)__ch_buf, sizeof(T));
                return __res ? *reinterpret_cast<T*>(__ch_buf) : T{};
            }

            // Reads a single element of type T from binary file. Ineffective.
            template <class T>
            bool readBinary(T &_val) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    printf(SVKFW_WRAPWARN("File :: ReaderStd :: readBinary (2)", "Reading a pointer from file - this is most likely an unwanted behaviour"));

                char *__ch_buf[sizeof(T)]{};
                bool __res = read((char *)__ch_buf, sizeof(T));
                if (__res) _val = *reinterpret_cast<T*>(__ch_buf);
                return __res;
            }

            // Reads an array of type T and length 'len' from binary file.
            // It doesn't handle possible misalignment.
            template <class T>
            bool readBinaryArray(T* _buf, size_t _len) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    printf(SVKFW_WRAPWARN("File :: ReaderStd :: readBinaryArray", "Reading pointers from file - this is most likely an unwanted behaviour"));

                return read((char*)_buf, sizeof(T) * _len);
            }
        }; // ReaderStd END

        struct WriterStd : WriterItf {
            std::ofstream f_handle;

            WriterStd(const std::string &_fpath = "", bool _truncate = false) { if (!_fpath.empty()) open(_fpath, _truncate); }
        ~WriterStd() { if (isOpen()) close(); }

            virtual inline bool open(const std::string &_fpath, bool _truncate = false) override {
                f_handle.open(_fpath, std::ios_base::out | std::ios_base::binary | (_truncate ? std::ios_base::trunc : (std::ios_base::openmode)0) );
                return  isOpen();
            }
            virtual inline bool write(const char *_buf, uint32_t _size) override {
                f_handle.write(_buf, _size);
                return isValid();
            }
            virtual inline bool  isOpen() const override { return f_handle.is_open(); }
            virtual inline bool isValid() const override { return f_handle.   good(); }
            virtual inline void   close()       override {        f_handle.  close(); }


            // The reason this class exists

            // Writes a single element of type T to binary file. Ineffective.
            template <class T>
            bool writeBinary(const T &_val) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    printf(SVKFW_WRAPWARN("File :: WriterStd :: writeBinary", "Writing a pointer - this is most likely an unwanted behaviour"));

                return write((const char*)&_val, sizeof(T));
            }

            // Writes an array of type T and length 'len' to binary file.
            template <class T>
            bool writeBinaryArray(const T* _buf, size_t _len) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    printf(SVKFW_WRAPWARN("File :: WriterStd :: writeBinaryArray", "Writing pointers - this is most likely an unwanted behaviour"));

                return write((const char*)_buf, sizeof(T) * _len);
            }
        }; // WriterStd END

#if (!defined(WIN32))
        typedef ReaderStd FileReader;
        typedef WriterStd FileWriter;
#endif
    }; // File END
}; // Simple END

#endif