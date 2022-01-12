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

logger::ConsoleOutput logc; // as a global variable, we are Ok with this

/*!
    @fn     int main(int argc, char* argv[])
    @brief  Main function
*/

int main(int argc, char *argv[])
{
    vs::vsGui         gui;
    vs::VideoFileInfo video ("fakename.vdo", 1920, 1080, 30, 100);
    logc.warn("main", "Dear Imgui mockup implementation - OpenGL renderer");

    int retval = gui.Init();;   // OpenGL context setup
    if (retval) {
        logc.error("main", "OpenGL + imgui context setup failed");
        return retval;
    }

    // Our state
    ImGui::SetNextWindowSize(ImVec2(700,500));
    // Main loop
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

        ImGui::Begin("videostrip-gui", NULL, ImGuiWindowFlags_MenuBar    |
                                                    //  ImGuiWin
                                                    //  ImGuiWindowFlags_NoTitleBar |
                                                     ImGuiWindowFlags_NoCollapse //|
                                                    //  ImGuiWindowFlags_NoMove     //|
                                                    //  ImGuiWindowFlags_NoResize
                                                       );

        ImGui::SetCursorPos(ImVec2(20,50));
        ImGui::Text("Filename: %s", video.filename.c_str());
        ImGui::SetCursorPos(ImVec2(20,70));
        ImGui::Text("Framerate: %d", video.fps);
        ImGui::SetCursorPos(ImVec2(20,90));
        ImGui::Text("Total frames: %d", video.num_frames);
        ImGui::SetCursorPos(ImVec2(20,120));
        // ImGui::Text("Total frames: %d");

        ImGui::Text("%s among %d items", ICON_FA_SEARCH, 42);
        ImGui::Button(ICON_FA_SEARCH " Search");

        ImGui::End();

        // *********************************************** >> Next window
        ImGui::Begin("file-summary", NULL);
        // ImGui::Begin("file-summary", &window_flag);
        ImGui::SetCursorPos(ImVec2(15,15));
        ImGui::Text("Video:");
        ImGui::SetCursorPos(ImVec2(15,40));
        ImGui::Text("640x480@30fps");
        ImGui::SetCursorPos(ImVec2(15,65));
        ImGui::Text("Duration: 00:11:42.06");
        ImGui::End();

        // *********************************************** >> Next window
        ImGui::Begin("another-name", NULL);
        ImGui::SetCursorPos(ImVec2(10,10));
        ImGui::Text("Scroll bar or log info");
        ImGui::End();

        // Rendering
        gui.Render();
    }

    // Cleanup
    gui.Destroy();

    return 0;
}
