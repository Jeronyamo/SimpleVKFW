#ifndef SVKFW_PD_TERMINAL_H
#define SVKFW_PD_TERMINAL_H


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
        struct ConsoleSupportANSICodes {
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

#endif
