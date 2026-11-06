#include "platform_dependent/pd_filetypes.h"

#include <algorithm>
#include <cctype>
#if (!defined(WIN32))
#include <dirent.h>
#include <sys/stat.h>
#endif

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


namespace Simple {
    namespace File {
        namespace {
            bool extensionMatches(const std::string &_name, const std::vector<std::string> &_extensions) {
                if (_extensions.empty()) return true;
                const size_t __dot = _name.find_last_of('.');
                if (__dot == std::string::npos) return false;
                std::string __ext = _name.substr(__dot);
                for (size_t i = 0u; i < __ext.size(); ++i) __ext[i] = (char)std::tolower((unsigned char)__ext[i]);
                return std::find(_extensions.begin(), _extensions.end(), __ext) != _extensions.end();
            }

            std::string dirPrefix(const std::string &_dir) {
                return _dir.empty() ? std::string() : (_dir[_dir.size() - 1u] == '/' ? _dir : _dir + "/");
            }

            void listFiles_(const std::string &_dir, const std::vector<std::string> &_extensions, bool _recursive,
                            std::vector<std::string> &_out, int _depth);
        } // anonymous

#ifdef WIN32
        namespace {
            void listFiles_(const std::string &_dir, const std::vector<std::string> &_extensions, bool _recursive,
                            std::vector<std::string> &_out, int _depth) {
                if (_depth > 16) return;
                const std::string __prefix  = dirPrefix(_dir);
                const std::string __pattern = __prefix + "*";
                const int __wsize = MultiByteToWideChar(CP_UTF8, 0, __pattern.c_str(), -1, NULL, 0);
                std::vector<wchar_t> __wpattern(size_t(std::max(__wsize, 1)), L'\0');
                MultiByteToWideChar(CP_UTF8, 0, __pattern.c_str(), -1, __wpattern.data(), __wsize);
                WIN32_FIND_DATAW __data;
                HANDLE __find = FindFirstFileW(__wpattern.data(), &__data);
                if (__find == INVALID_HANDLE_VALUE) return;
                do {
                    const int __size = WideCharToMultiByte(CP_UTF8, 0, __data.cFileName, -1, NULL, 0, NULL, NULL);
                    if (__size <= 1) continue;
                    std::string __name(size_t(__size - 1), '\0');
                    WideCharToMultiByte(CP_UTF8, 0, __data.cFileName, -1, &__name[0], __size, NULL, NULL);
                    if (__name == "." || __name == "..") continue;
                    if (__data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                        if (_recursive) listFiles_(__prefix + __name, _extensions, _recursive, _out, _depth + 1);
                    } else if (extensionMatches(__name, _extensions)) _out.push_back(__prefix + __name);
                } while (FindNextFileW(__find, &__data));
                FindClose(__find);
            }
        } // anonymous
#else
        namespace {
            void listFiles_(const std::string &_dir, const std::vector<std::string> &_extensions, bool _recursive,
                            std::vector<std::string> &_out, int _depth) {
                if (_depth > 16) return;
                const std::string __prefix = dirPrefix(_dir);
                DIR *__dirp = opendir(_dir.empty() ? "." : _dir.c_str());
                if (!__dirp) return;
                while (dirent *__entry = readdir(__dirp)) {
                    const std::string __name = __entry->d_name;
                    if (__name == "." || __name == "..") continue;
                    const std::string __path = __prefix + __name;
                    struct stat __st;
                    if (stat(__path.c_str(), &__st) != 0) continue;
                    if (S_ISDIR(__st.st_mode)) {
                        if (_recursive) listFiles_(__path, _extensions, _recursive, _out, _depth + 1);
                    } else if (extensionMatches(__name, _extensions)) _out.push_back(__path);
                }
                closedir(__dirp);
            }
        } // anonymous
#endif

        std::vector<std::string> listFiles(const std::string &_dir, const std::vector<std::string> &_extensions, bool _recursive) {
            std::vector<std::string> __res;
            std::string __dir = _dir;
            std::replace(__dir.begin(), __dir.end(), '\\', '/');
            listFiles_(__dir, _extensions, _recursive, __res, 0);
            std::sort(__res.begin(), __res.end());
            return __res;
        }
    }; // File END
}; // Simple END
