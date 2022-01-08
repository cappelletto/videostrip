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
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

static void glfw_error_callback(int error, const char* description)
{
    cout << "Glfw Error [" << error << "] "<< description << endl;
}

using namespace std;
using namespace cv;

/*!
    @fn     int main(int argc, char* argv[])
    @brief  Main function
*/

logger::ConsoleOutput logc; // as a global variable, we are Ok with this

int main(int argc, char *argv[])
{
    logc.warn("main", "Dear Imgui mockup implementation - OpenGL renderer");

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(800, 600, "videostrip-mockup", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    cout << "context created" << endl;


    ImGui::SetNextWindowSize(ImVec2(700,500));
    // Main loop
    static bool window_flag = true;
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("videostrip-gui", &window_flag, ImGuiWindowFlags_MenuBar    | 
                                                     ImGuiWindowFlags_NoTitleBar |
                                                     ImGuiWindowFlags_NoCollapse |
                                                     ImGuiWindowFlags_NoMove     |
                                                     ImGuiWindowFlags_NoResize   );
        // Begin main menu bar
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Load video"))
                {
                    // Open file dialog
                }
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                {
                    window_flag = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Some parameters", "CTRL+Y", false, false)) // Disabled item
                {
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Preferences", "CTRL+P"))
                {
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About"))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // if (ImGui::Begin("videostrip-gui", &window_flag))
        // {

            ImGui::SetCursorPos(ImVec2(23,62.5));
            ImGui::Text("filename: <path/to/filename>");

            ImGui::SetCursorPos(ImVec2(182,123.5));
            ImGui::PushItemWidth(200);
            static float progress16 = 0.0f;
            ImGui::ProgressBar(progress16, ImVec2(0.0f, 0.0f));
            ImGui::PopItemWidth();

            ImGui::SetCursorPos(ImVec2(184.5,94.5));
            ImGui::Button("Button1", ImVec2(57,19)); //remove size argument (ImVec2) to auto-resize

            ImGui::SetCursorPos(ImVec2(248.5,93.5));
            ImGui::Button("Button2", ImVec2(57,19)); //remove size argument (ImVec2) to auto-resize

            ImGui::SetCursorPos(ImVec2(313.5,96.5));
            ImGui::Button("Button3", ImVec2(57,19)); //remove size argument (ImVec2) to auto-resize

            ImGui::SetCursorPos(ImVec2(186,151.5));
            ImGui::Text("Log output (current action)");

            ImGui::SetCursorPos(ImVec2(16,96));
            ImGui::BeginChild(22, ImVec2(158,130), true);

            ImGui::SetCursorPos(ImVec2(33.5,60.5));
            ImGui::Text("video preview");

            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(401,90));
            ImGui::BeginChild(24, ImVec2(151,138), true);

            ImGui::SetCursorPos(ImVec2(44.5,59.5));
            ImGui::Text("logger output");

            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(306.5,62.5));
            ImGui::Button("^ load", ImVec2(50,19)); //remove size argument (ImVec2) to auto-resize

            ImGui::SetCursorPos(ImVec2(26.5,74.5));
            ImGui::Text("VIDEO summary info");

            ImGui::End();
        // }

        // // Edit a color (stored as ~4 floats)
        // ImGui::ColorEdit4("Color", my_color);

        // // Plot some values
        // const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
        // ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

        // // Display contents in a scrolling region
        // ImGui::TextColored(ImVec4(1,1,0,1), "Important Stuff");
        // ImGui::BeginChild("Scrolling");
        // for (int n = 0; n < 50; n++)
        //     ImGui::Text("%04d: Some text", n);
        // ImGui::EndChild();
        // ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
/**/
    // *******************************************************************************************************************
    // *******************************************************************************************************************
    // END OF IMGUI CODE

    int retval = initParser(argc, argv);    // initial argument validation, populates arg parsing structure args
    if (retval != 0)                        // some error ocurred, we have been signaled to stop
        return retval;
    std::ostringstream s;

    // Input file priority: must be defined either by the config.yaml or --input argument
    string inputFileName    = ""; // command arg or config defined
    string outputFileName   = ""; // if empty, output filenames will be the same as the standard. If non-null, will be used as prefix
    string outputFilePath   = ""; // absolut/relative folder path were output will be stored
    int verbosityLevel      = 0;  // verbosity level, 0 - 3

    if (argInput)   inputFileName    = args::get(argInput);   //input file is optional positional argument.
    if (argOutput)  outputFileName   = args::get(argOutput);  //outpu file is optional positional argument.
    if (argVerbose) verbosityLevel   = args::get(argVerbose); // retrieve user defined verbosity level

    if (argDumpInfo)
    {
        cout << "\tOpenCV version:\t" << yellow << CV_VERSION << reset << endl;
        cout << "\tGit commit:\t" << yellow << GIT_COMMIT << reset << endl;
        cout << "\tMode:\t\t" << yellow << CMAKE_BUILD_TYPE << reset << endl;
        cout << cv::getBuildInformation() << std::endl;
        s << "Input file: " << inputFileName << endl;
        s << "Output file: " << outputFileName << endl;
        s << "Verbosity level: " << verbosityLevel << endl;
        logc.info("main", s.str());
        return 0;
    }

    if (inputFileName.empty()){ //not defined as command line argument? let's use config.yaml definition
        logc.error ("main", "Input file missing. Please define it using --input=<filename>");
        return -1;
    }

    return 0;
}
