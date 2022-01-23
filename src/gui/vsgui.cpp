#include <iostream>
// #include "IconsFontAwesome4.h"
#include "imgui_internal.h"
#include "gui/vsgui.hpp"



static void glfw_error_callback(int error, const char* description)
{
    std::cout << "Glfw Error [" << error << "] "<< description << std::endl;
}

int vs::vsGui::Init(){

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

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()){
        std::cout << "Failed to initialize GLFW" << std::endl;
        logc.error ("main", "Failed to initialize GLFW");
        return 1;
    }
    // Create window with graphics context
    window = glfwCreateWindow(800, 600, "videostrip-mockup", NULL, NULL);   //<---- main window properties can be predefined in the namespace
    if (window == NULL){
        logc.error("vsgui","Failed to create GLFW window");
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Enable Keyboard Controls
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    auto& style = ImGui::GetStyle();
    // if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 8.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        style.Colors[ImGuiCol_TabActive].x = 0.9f;
        style.Colors[ImGuiCol_TabActive].y = 0.2f;
        style.Colors[ImGuiCol_TabActive].z = 0.0f;
    }

    // // Main
    style.WindowPadding            = ImVec2(9.00f, 6.00f);
    style.FramePadding             = ImVec2(6.00f, 2.00f);
    style.ItemSpacing              = ImVec2(5.00f, 4.00f);
    style.ItemInnerSpacing         = ImVec2(3.00f, 5.00f);
    style.FrameBorderSize          = 1.0f;
    style.IndentSpacing            = 12.00f;
    style.ScrollbarSize            = 15.00f;
    style.GrabMinSize              = 16.00f;

    // Borders
    style.PopupBorderSize          = 0.00f;
    style.TabBorderSize            = 1.00f;

    // Rounding
    style.WindowRounding           = 6.00f;
    style.FrameRounding            = 6.00f;
    style.GrabRounding             = 6.00f;

    // Push back ImGui style

    // customize fonts
    ImFont* newfont = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16.0f);
    if (newfont) {
        // set font to default
        io.FontDefault = newfont;
    }
    // allows awesome-icons
    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.0f;
    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFont* iconfont = io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/font-awesome/fontawesome-webfont.ttf", 16.0f, &config, icon_ranges);
    if (!newfont) {
        // set font to default
        logc.error("vsgui","Failed to load font <font-awesome>");
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    return 0;
}

void vs::vsGui::Destroy(){
    // Imgui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // OpenGL cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

}

void vs::vsGui::NewFrame(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // this could be configurable
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
 

}

void vs::vsGui::Render(){
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}


int vs::drawWindowInput(vs::VideoFile &video){
            // ----------- FIRST WINDOW: INPUT FILE AND OUTPUT FOLDER
        ImGui::Begin("file-loader", NULL,   ImGuiWindowFlags_MenuBar    |
                                        //  ImGuiWin
                                        //  ImGuiWindowFlags_NoTitleBar |
                                            ImGuiWindowFlags_NoCollapse //|
                                        //  ImGuiWindowFlags_NoMove     //|
                                        //  ImGuiWindowFlags_NoResize
                                            );


            // Check if video.filename is empty (if empty is also invalid)
            ImGui::Text("Input video");  ImGui::SameLine();
            ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
            if (video.filename.empty()) {
                char _str[] = "<none selected>";
                ImGui::InputText("##video_file_path", 
                                _str,
                                sizeof(_str));
            }
            else {
                // check if currently specified video is valid
                if (video.isValid()){
                    // push greeen colour
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    ImGui::InputText("##video_file_path", 
                                    const_cast<char*>(video.filename.c_str()), 
                                    sizeof(video.filename.c_str()));
                }
                else{
                    // push red color
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    ImGui::InputText("##video_file_path", 
                                    const_cast<char*>(video.filename.c_str()), 
                                    sizeof(video.filename.c_str()));
                    // ImGui::SameLine();
                    // ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Invalid video file");
                }
                ImGui::PopStyleColor();
            }

            ImGui::SameLine(); // we add a button to load a new video file. Ths will launch a modal dialog ImgGuiDialog::FileDialog()
            if (ImGui::Button(ICON_FA_FILE_VIDEO_O " Load video file")){
                // char *file_filter = "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md";
                const char *file_filter = "Video files{.avi,.mov,.mp4}";    // TODO: Fix problem when using ".*" as file extension            
                ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey", "Choose File", file_filter, ".");
            }

            // Now the output folder path
            ImGui::Text("Output folder");  ImGui::SameLine();
            if (video.output_folder.empty()) {
                char _str[] = "<none selected>";
                ImGui::InputText("##output_folder_path", _str, sizeof(_str));
            }
            else {
                ImGui::InputText("##output_folder_path",
                                const_cast<char*>(video.output_folder.c_str()),
                                sizeof(video.output_folder.c_str()));
            }
            ImGui::SameLine(); // we add a button to load a new video file. Ths will launch a modal dialog ImgGuiDialog::FileDialog()
            if (ImGui::Button(ICON_FA_FOLDER_OPEN " Select folder")){
                // char *file_filter = "Source files{.cpp,.h,.hpp},Image files{.png,.gif,.jpg,.jpeg},.md";
                const char *file_filter = "Video files{.avi,.mov,.mp4}";    // TODO: Fix problem when using ".*" as file extension            
                ImGuiFileDialog::Instance()->OpenModal("ChooseDirDlgKey", "Choose a directory", nullptr, ".");
            }

            ImGui::PopItemWidth();

            // ------------ CHECK OPENED DIALOG BOXES//
            // check if file dialogbox displayed(if called)
            if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) 
            {
                // action if OK
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                    // then we validate the new user provided filename and path
                    if (filePathName.empty() || filePath.empty()) {
                        logc.error("main", "Invalid file path or name");
                        return -1;
                    }
                    else{
                        video.filename = filePathName;
                        // video.output_folder 
                        video.peekFile(); // implicit call to check if the file is valid. Sets internal is_valid flag
                        if (video.isValid()){
                            std::ostringstream ss;
                            ss << "Valid video file: " << video.filename;
                            logc.info("main", ss);
                        }

                    }
                }
                // close
                ImGuiFileDialog::Instance()->Close();
            }


            if (ImGuiFileDialog::Instance()->Display("ChooseDirDlgKey")) 
            {
                // action if OK
                if (ImGuiFileDialog::Instance()->IsOk())
                {
                    // std::string outputPath = ImGuiFileDialog::Instance()->GetFilePathName();
                    std::string outputPath = ImGuiFileDialog::Instance()->GetCurrentPath();
                    // then we validate the new user provided filename and path
                    if (outputPath.empty()) {
                        logc.error("main", "Invalid output folder path or name");
                        return -1;
                    }
                    else{
                        video.output_folder = outputPath;
                        std::ostringstream ss;
                        ss << "Selected new output folder: " << video.output_folder;
                        logc.info("main", ss);
                    }
                }
                // close
                ImGuiFileDialog::Instance()->Close();
            }

        ImGui::End();
        return 0;
}