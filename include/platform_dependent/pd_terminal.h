#ifndef SVKFW_PD_TERMINAL_H
#define SVKFW_PD_TERMINAL_H

#include <cstdio>


//  Assuming ANSI codes are supported -
//  Linux, Mac or newer Windows
#ifdef SVKFW_CM_ANSI_ESC  

// examples:
//  printf(CSISEQ(SGR_COL_BG_BLACK ";" SGR_COL_FG_WHITE) "White text on black background\n");
//  printf(CSISEQ(SGR_DEFAULT)); printf(CSISEQ("")); // explicit|implicit default terminal style
//  printf(CSISEQ(SGR_COL_FG_SET("255", "0", "0")) "Red text on default background (RGB24)");
#define CSISEQ(_ANSI_CODES) "\033[" _ANSI_CODES "m"
#define CSISEQ_BEG "\033["
#define CSISEQ_END "m"


// ANSI is supported since November update (build 10586)
// ANSI + Windows -> Anniversary update (build 14393) or newer
// (November update had limited functionality)
#ifdef SVKFW_CM_WIN

#include <Windows.h>

namespace Simple {
    namespace WinInner {
        // Enables ANSI escape code support for the terminal on Windows 10|11
        // Note: the object is 'static' and stays in the header, so every program that includes it runs the constructor
        //       (a definition in svkfw_lib could be dropped by the linker when nothing else in its .cpp is used)
        static struct ConsoleSupportANSICodes {
            ConsoleSupportANSICodes() {
                // Set output mode to handle virtual terminal sequences
                HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                if (hOut == INVALID_HANDLE_VALUE)
                    printf("[Warning]: WinInner :: ConsoleSupportANSICodes - GetStdHandle, code %d\n", GetLastError());

                DWORD dwMode = 0;
                if (!GetConsoleMode(hOut, &dwMode))
                    printf("[Warning]: WinInner :: ConsoleSupportANSICodes - GetConsoleMode, code %d\n", GetLastError());

                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                if (!SetConsoleMode(hOut, dwMode))
                    printf("[Warning]: WinInner :: ConsoleSupportANSICodes - SetConsoleMode, code %d\n", GetLastError());
            }
        } init_console_support_ansi_codes; // ConsoleSupportANSICodes END
    }; // WinInner END
}; // Simple END
#endif // END: SVKFW_CM_WIN - new versions of Windows support ANSI escape codes
// END: SVKFW_CM_ANSI_ESC - Linux, Mac or newer Windows;
#else // if not SVKFW_CM_ANSI_ESC - older Windows|other platforms. TODO?

#define CSISEQ(_ANSI_CODES) ""
#define CSISEQ_BEG ""
#define CSISEQ_END ""


// for Windows API version: SetConsoleScreenBufferInfoEx, CONSOLE_FONT_INFOEX etc.:
// https://learn.microsoft.com/en-us/windows/console/console-functions

#endif



// Handling unicode - terminal and command line arguments - on different platforms (converts everything to utf-8)

#ifdef WIN32
#include <Windows.h>

namespace Simple {
    namespace WinInner {
        // Taken from: https://stackoverflow.com/questions/23471873/change-console-code-page-in-windows-c
        // Note: 'static' and in the header for the same reason as ConsoleSupportANSICodes. Each translation unit has its own
        //       object, they are destroyed in reverse order, so the first one restores the original code page last
        static struct UTF8CodePage {
            UTF8CodePage() : old_code_page(GetConsoleOutputCP()) { SetConsoleOutputCP(CP_UTF8); }
           ~UTF8CodePage() { SetConsoleOutputCP(old_code_page); }

            UINT old_code_page;
        } init_utf8_code_page; // UTF8CodePage END
    }; // WinInner END
}; // Simple END

#endif

namespace Simple {
    struct ArgHandler {
        int    argc = 0;
        int    flag = 0; // if !=0, deletes argv arrays
        char **argv = nullptr;

        ArgHandler();
        ArgHandler(int _argc, char** _argv);
       ~ArgHandler();

        void clear();

        void initArgs(int _argc, char** _argv);

        char* getArg(int _arg_id);

        void processArgs();
    }; // ArgHandler END
}; // Simple END

#endif
