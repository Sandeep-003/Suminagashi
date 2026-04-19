#include "app.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

static void MainLoop(void* arg)
{
    static_cast<SuminagashiApp*>(arg)->DrawFrame();
}
#endif

int main(void)
{
    SuminagashiApp& app = GetApp();
    app.Initialize();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(MainLoop, &app, 0, 1);
    return 0;
#else
    while (!WindowShouldClose())
    {
        app.DrawFrame();
    }

    app.Shutdown();
    return 0;
#endif
}