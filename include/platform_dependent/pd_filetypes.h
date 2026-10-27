#ifndef SVKFW_PD_FILETYPES_H
#define SVKFW_PD_FILETYPES_H

#include "common/terminal.h"

#include <fstream>


namespace Simple {
    namespace File {
        struct ReaderItf {
            virtual ~ReaderItf();

            virtual bool  open(const std::string  &_fpath) = 0;
            virtual bool  read(char *_buf, uint32_t _size) = 0;
            virtual bool isValid() const = 0;
            virtual bool  isOpen() const = 0;
            virtual bool   isEOF() const = 0;
            virtual void   close()       = 0;
        }; // ReaderItf END

        struct WriterItf {
            virtual ~WriterItf();

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

        InvalidationFlags operator|(InvalidationFlags &_flag1, InvalidationFlags _flag2);
        InvalidationFlags operator&(InvalidationFlags &_flag1, InvalidationFlags _flag2);
        InvalidationFlags operator|=(InvalidationFlags &_flag1, InvalidationFlags _flag2);
        InvalidationFlags operator&=(InvalidationFlags &_flag1, InvalidationFlags _flag2);
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

            ReaderWin(const std::string &_fpath = "");
           ~ReaderWin();

            virtual bool open(const std::string &_fpath) override;
            virtual bool read(char *_buf, uint32_t _size) override;
            virtual bool  isOpen() const override;
            virtual bool isValid() const override;
            virtual bool   isEOF() const override;
            virtual void   close()       override;


        // The reason this class exists

            // Reads a single element of type T from a binary file.
            template <class T>
            T readBinary() {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderWin :: readBinary (1)", "Reading a pointer from file - this is most likely an unwanted behaviour\n"));

                T __res_val{};
                bool __res = read(reinterpret_cast<char *>(&__res_val), sizeof(T));
                return __res ? __res_val : T{};
            }

            // Reads a single element of type T from a binary file.
            template <class T>
            bool readBinary(T &_val) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderWin :: readBinary (2)", "Reading a pointer from file - this is most likely an unwanted behaviour\n"));

                T __res_val{};
                bool __res = read(reinterpret_cast<char *>(&__res_val), sizeof(T));
                if (__res) _val = __res_val;
                return __res;

            }

            // Reads an array of type T and length 'len' from binary file.
            // It doesn't handle possible misalignment.
            template <class T>
            bool readBinaryArray(T* _buf, size_t _len) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderWin :: readBinaryArray", "Reading pointers from file - this is most likely an unwanted behaviour\n"));

                return read((char*)_buf, sizeof(T) * _len);
            }
        }; // ReaderWin END

        struct WriterWin : WriterItf {
            HANDLE f_handle = INVALID_HANDLE_VALUE;
            InvalidationFlags f_state = FILE_VALID;

            WriterWin(const std::string &_fpath = "", bool _truncate = false);
           ~WriterWin();

            virtual bool open(const std::string &_fpath, bool _truncate = false) override;
            virtual bool write(const char *_buf, uint32_t _size) override;
            virtual bool  isOpen() const override;
            virtual bool isValid() const override;
            virtual void   close() override;


            // The reason this class exists

            // Writes a single element of type T to binary file. Ineffective.
            template <class T>
            bool writeBinary(const T &_val) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: WriterWin :: writeBinary", "Writing a pointer - this is most likely an unwanted behaviour\n"));

                return write((const char*)&_val, sizeof(T));
            }

            // Writes an array of type T and length 'len' to binary file.
            template <class T>
            bool writeBinaryArray(const T* _buf, size_t _len) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: WriterWin :: writeBinaryArray", "Writing pointers - this is most likely an unwanted behaviour\n"));

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

            ReaderStd(const std::string &_fpath = "");
           ~ReaderStd();

            virtual bool open(const std::string  &_fpath) override;
            virtual bool read(char *_buf, uint32_t _size) override;
            virtual bool   isEOF() const override;
            virtual bool  isOpen() const override;
            virtual bool isValid() const override;
            virtual void   close()       override;


        // The reason this class exists

            // Reads a single element of type T from binary file. Ineffective.
            template <class T>
            T readBinary() {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderStd :: readBinary (1)", "Reading a pointer from file - this is most likely an unwanted behaviour\n"));

                T __res_val{};
                bool __res = read(reinterpret_cast<char *>(&__res_val), sizeof(T));
                return __res ? __res_val : T{};
            }

            // Reads a single element of type T from binary file. Ineffective.
            template <class T>
            bool readBinary(T &_val) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderStd :: readBinary (2)", "Reading a pointer from file - this is most likely an unwanted behaviour\n"));

                T __res_val{};
                bool __res = read(reinterpret_cast<char *>(&__res_val), sizeof(T));
                if (__res) _val = __res_val;
                return __res;

            }

            // Reads an array of type T and length 'len' from binary file.
            // It doesn't handle possible misalignment.
            template <class T>
            bool readBinaryArray(T* _buf, size_t _len) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderStd :: readBinaryArray", "Reading pointers from file - this is most likely an unwanted behaviour\n"));

                return read((char*)_buf, sizeof(T) * _len);
            }
        }; // ReaderStd END

        struct WriterStd : WriterItf {
            std::ofstream f_handle;

            WriterStd(const std::string &_fpath = "", bool _truncate = false);
           ~WriterStd();

            virtual bool open(const std::string &_fpath, bool _truncate = false) override;
            virtual bool write(const char *_buf, uint32_t _size) override;
            virtual bool  isOpen() const override;
            virtual bool isValid() const override;
            virtual void   close()       override;


            // The reason this class exists

            // Writes a single element of type T to binary file. Ineffective.
            template <class T>
            bool writeBinary(const T &_val) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: WriterStd :: writeBinary", "Writing a pointer - this is most likely an unwanted behaviour\n"));

                return write((const char*)&_val, sizeof(T));
            }

            // Writes an array of type T and length 'len' to binary file.
            template <class T>
            bool writeBinaryArray(const T* _buf, size_t _len) {
                if (std::is_reference<T>::value || std::is_pointer<T>::value)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: WriterStd :: writeBinaryArray", "Writing pointers - this is most likely an unwanted behaviour\n"));

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