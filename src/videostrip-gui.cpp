/**
 * @file videotrip-gui.cpp
 * @author Jose Cappelletto (cappelletto@gmail.com)
 * @brief [videostrip] as stand-alone module for video processing. Rework from scracth, based on the original uwimgproc/videostrip.cpp
 * @version 0.1 GUI Mockup implementation
 * @date 2022-01-11
 * 
 * @copyright Copyright (c) 2020-2022
 * 
 */
#include <iostream>
#include <args.hxx>

#include <headers.h>
#include <helper.h>
#include <options.h>
// OpenGL headers
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
// imgui headers
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "videostrip/vscore.h"
#include "gui/vsgui.hpp"

using namespace std;
using namespace cv;

using namespace vs;


/*!
    @fn     int main(int argc, char* argv[])
    @brief  Main function
*/

// TODO: Create FIFO queue for console output (window/gui). Could be a replica of the logger::ConsoleOutput class

int main(int argc, char *argv[])
{
    vs::vsGui       gui;
    vs::VideoFile   video;
    // vs::log.warn("main", "Dear Imgui mockup implementation - OpenGL renderer");

    int retval = gui.Init();;   // OpenGL context setup
    if (retval) {
        // vs::logc.error("main", "OpenGL + imgui context setup failed");
        return retval;
    }

    // Our state
    ImGui::SetNextWindowSize(ImVec2(750,400));
    // Main loop

    // let's populate with some data for testing purposes
    if (video.peekFile("/home/cappelletto/Videos/fran01.mp4") == -1) {
        vs::logc.error("main", "Video file peek failed");
        return -1;
    }
    else{
        vs::logc.info("main", "Video file peek success");
    }

    static bool window_flag = true;
    while (!gui.ShouldClose() && window_flag)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        gui.NewFrame();

        int r = drawWindowInput(video);


        // ImGui::End();

        // // *********************************************** >> Next window
        // ImGui::Begin("another-name", NULL);
        //  // open Dialog Simple
        // if (ImGui::Button("Open File Dialog")){
        //     // char *file_filter = "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md";
        //     const char *file_filter = ".*,Video files{.avi,.mov,.mp4}";            
        //     ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey", "Choose File", file_filter, ".");
        //  }
        // // display
        // if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) 
        // {
        //     // action if OK
        //     if (ImGuiFileDialog::Instance()->IsOk())
        //     {
        //     std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
        //     std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        //     // action
        //     }
            
        //     // close
        //     ImGuiFileDialog::Instance()->Close();
        // }

        // Rendering
        gui.Render();
    }

    // Cleanup
    gui.Destroy();
    vs::logc.info("main", "Exiting");
    return 0;
}
