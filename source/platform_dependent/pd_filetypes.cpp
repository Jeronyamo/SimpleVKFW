#include "platform_dependent/pd_filetypes.h"

namespace Simple {
    namespace File {

        ReaderItf::~ReaderItf() {}

        WriterItf::~WriterItf() {}

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
namespace Simple {
    namespace File {

        ReaderWin::ReaderWin(const std::string &_fpath) { if (!_fpath.empty()) open(_fpath); }

        ReaderWin::~ReaderWin() { if (isOpen()) close(); }

        bool ReaderWin::open(const std::string &_fpath) {
            // Get WCHAR string to open a file with (possibly) Unicode characters
            int __wchars_num = MultiByteToWideChar(CP_UTF8, 0, _fpath.c_str(), _fpath.size(), NULL, 0);
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

            SVKFW_WASSERT(f_handle != INVALID_HANDLE_VALUE, "File :: ReaderWin :: open", "could not open file - possibly it doesn't exist\n");
            delete[] wstr;
            return isOpen();
        }

        bool ReaderWin::read(char *_buf, uint32_t _size) {
            DWORD __bytes_was_read = 0;

            if (!ReadFile(f_handle, _buf, _size, &__bytes_was_read, nullptr)) {
                fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderWin :: read", "read error\n"));
                f_state |= FILE_INVALID_BIT_ERR;
            }
            else if (__bytes_was_read < _size) {
                if (__bytes_was_read > 0)
                    fprintf(svkfwwarn, SVKFW_WRAPWARN("File :: ReaderWin :: read", "%d/%d bytes, EOF reached\n"), __bytes_was_read, _size);
                f_state |= FILE_INVALID_BIT_EOF;
            }
            return isValid();
        }

        bool  ReaderWin::isOpen() const { return f_handle !=  INVALID_HANDLE_VALUE; }

        bool ReaderWin::isValid() const { return isOpen() && f_state == FILE_VALID; }

        bool   ReaderWin::isEOF() const { return f_state  &   FILE_INVALID_BIT_EOF; }

        void   ReaderWin::close() { CloseHandle(f_handle); f_handle = INVALID_HANDLE_VALUE; }

        WriterWin::WriterWin(const std::string &_fpath, bool _truncate) { if (!_fpath.empty()) open(_fpath, _truncate); }

        WriterWin::~WriterWin() { if (isOpen()) close(); }

        bool WriterWin::open(const std::string &_fpath, bool _truncate) {
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

            SVKFW_WASSERT(f_handle != INVALID_HANDLE_VALUE, "File :: WriterWin :: open", "could not open file - possibly it doesn't exist\n");
            delete[] wstr;
            return isOpen();
        }

        bool WriterWin::write(const char *_buf, uint32_t _size) {
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

        bool  WriterWin::isOpen() const { return f_handle !=  INVALID_HANDLE_VALUE; }

        bool WriterWin::isValid() const { return isOpen() && f_state == FILE_VALID; }

        void   WriterWin::close() { CloseHandle(f_handle); f_handle = INVALID_HANDLE_VALUE; }
    }; // File END
}; // Simple END
#endif

namespace Simple {
    namespace File {

        ReaderStd::ReaderStd(const std::string &_fpath) { if (!_fpath.empty()) open(_fpath); }

        ReaderStd::~ReaderStd() { if (isOpen()) close(); }

        bool ReaderStd::open(const std::string  &_fpath) {
            f_handle.open(_fpath, std::ios_base::in | std::ios_base::binary);
            return  isOpen();
        }

        bool ReaderStd::read(char *_buf, uint32_t _size) {
            f_handle.read(_buf, _size);
            return isValid();
        }

        bool   ReaderStd::isEOF() const { return f_handle.    eof(); }

        bool  ReaderStd::isOpen() const { return f_handle.is_open(); }

        bool ReaderStd::isValid() const { return f_handle.   good(); }

        void   ReaderStd::close() {        f_handle.  close(); }

        WriterStd::WriterStd(const std::string &_fpath, bool _truncate) { if (!_fpath.empty()) open(_fpath, _truncate); }

        WriterStd::~WriterStd() { if (isOpen()) close(); }

        bool WriterStd::open(const std::string &_fpath, bool _truncate) {
            f_handle.open(_fpath, std::ios_base::out | std::ios_base::binary | (_truncate ? std::ios_base::trunc : (std::ios_base::openmode)0) );
            return  isOpen();
        }

        bool WriterStd::write(const char *_buf, uint32_t _size) {
            f_handle.write(_buf, _size);
            return isValid();
        }

        bool  WriterStd::isOpen() const { return f_handle.is_open(); }

        bool WriterStd::isValid() const { return f_handle.   good(); }

        void   WriterStd::close() {        f_handle.  close(); }
    }; // File END
}; // Simple END
