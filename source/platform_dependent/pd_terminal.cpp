#include "platform_dependent/pd_terminal.h"

namespace Simple {

    ArgHandler::ArgHandler() {}

    ArgHandler::ArgHandler(int _argc, char** _argv) : argc{_argc}, argv{_argv} { processArgs(); }

    ArgHandler::~ArgHandler() { clear(); }

    void ArgHandler::clear() {
        if (flag) {
            for (int i = 0; i < argc; ++i)
                delete[] argv[i];
            delete[] argv;
        }
        argv = nullptr;
        argc = 0;
        flag = 0;
    }

    void ArgHandler::initArgs(int _argc, char** _argv) {
        clear();
        argc = _argc;
        argv = _argv;
        processArgs();
    }

    char* ArgHandler::getArg(int _arg_id) { return _arg_id < argc ? argv[_arg_id] : nullptr; }

    void ArgHandler::processArgs() {
#ifdef WIN32
        clear();
        flag = 1;

        LPWSTR *__cmd_line_w = CommandLineToArgvW(GetCommandLineW(), &argc);
        argv = new char*[argc]{};

        for (int i = 0; i < argc; ++i) {
            int arg_size = WideCharToMultiByte(CP_UTF8, 0, __cmd_line_w[i], -1, nullptr, 0, nullptr, nullptr);
            argv[i] = new char[arg_size]{};
            WideCharToMultiByte(CP_UTF8, 0, __cmd_line_w[i], -1, argv[i], arg_size, nullptr, nullptr);
        }
        LocalFree(__cmd_line_w);
#endif
    }
}; // Simple END
