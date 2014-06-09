#include <stdint.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "capture/capture.h"
#include "capture/screen_shot_saver.h"
#include "capture/somagic_analyzer.h"
#include "capture/somagic_source.h"
#include "gui/bounding_box_drawer.h"
#include "gui/box.h"
#include "gui/main_window.h"

#include "gui/screen.h"

#include <iostream>

DEFINE_bool(save_screenshot, false, "save screenshot");
DEFINE_bool(draw_result, true, "draw analyzer result");

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    SDL_Init(SDL_INIT_VIDEO);

    {
        SomagicSource somagicSource(argv[0]);
        if (!somagicSource.ok()) {
            fprintf(stderr, "Failed to load %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }

        SomagicAnalyzer somagicAnalyzer;
        Capture capture(&somagicSource, &somagicAnalyzer);

        unique_ptr<AnalyzerResultDrawer> analyzerResultDrawer;
        if (FLAGS_draw_result)
            analyzerResultDrawer.reset(new AnalyzerResultDrawer(&capture));

        unique_ptr<ScreenShotSaver> saver;
        if (FLAGS_save_screenshot)
            saver.reset(new ScreenShotSaver);

        MainWindow mainWindow(720, 480, Box(0, 0, 720, 480));
        mainWindow.addDrawer(&capture);
        if (saver.get())
            mainWindow.addDrawer(saver.get());
        if (analyzerResultDrawer.get())
            mainWindow.addDrawer(analyzerResultDrawer.get());

        somagicSource.start();
        capture.start();

        mainWindow.runMainLoop();
    }

    SDL_Quit();

    return 0;
}
