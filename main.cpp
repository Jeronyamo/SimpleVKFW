#include "common/argparser.h"
#include "core/engine.h"
#include "apps/editor_app.h"
#include "apps/game_app.h"

#include <cstdio>


// Arguments:
//   --editor, -e       the editor (the default)
//   --game,   -g       the game
//   --frames, -f N     quit after N frames (tests, benchmarks)
int main(int argc, char **argv) {
    Simple::ArgParser args{argc, argv};
    args.addArgument("editor", "e", {  });
    args.addArgument("game",   "g", {  });
    args.addArgument("frames", "f", {1u});
    args.parseAllArguments();

    namespace Core  = Simple::Core;
    namespace Event = Simple::Event;

    // The engine runs the app its event asks for (one at a time: the event of another app is ignored while one runs)
    Core::Engine engine;
    engine.registerApp(Event::EV_TYPE_RUN_EDITOR, []() -> Core::AppItf* { return new Simple::Apps::EditorApp(); });
    engine.registerApp(Event::EV_TYPE_RUN_GAME,   []() -> Core::AppItf* { return new Simple::Apps::GameApp();   });
    if (args.hasArgumentParsed("frames")) engine.max_frames = args.castArgument<uint32_t>("frames");

    engine.events.write(args.hasArgumentParsed("game") ? Event::EV_TYPE_RUN_GAME : Event::EV_TYPE_RUN_EDITOR);
    const int __code = engine.run();

    if (__code == 0) printf(SVKFW_WRAPINFO("main", "Success\n"));
    return __code;
}
