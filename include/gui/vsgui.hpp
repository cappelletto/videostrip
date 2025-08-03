// vsgui namespace declaration containing all GUI classes
// Current base implementation relies on ImGui library with OpenGL backend for rendering
//
// Note:

#pragma once

#ifndef _VS_GUI_H_
#define _VS_GUI_H_

// OpenGL headers
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
// imgui headers
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

#include "videostrip/vscore.h"
#include "../external/ImGuiFileDialog/ImGuiFileDialog.h"

#include "IconsFontAwesome4.h"

// define collection of const colors for imgui interface
/*
namespace vsgui{
    const ImVec4 COLOR_WHITE = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    const ImVec4 COLOR_BLACK = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    const ImVec4 COLOR_RED = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    const ImVec4 COLOR_GREEN = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    const ImVec4 COLOR_BLUE = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    const ImVec4 COLOR_YELLOW = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    const ImVec4 COLOR_CYAN = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
    const ImVec4 COLOR_MAGENTA = ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
    const ImVec4 COLOR_ORANGE = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
    const ImVec4 COLOR_PURPLE = ImVec4(0.5f, 0.0f, 1.0f, 1.0f);
    const ImVec4 COLOR_BROWN = ImVec4(0.5f, 0.25f, 0.0f, 1.0f);
    const ImVec4 COLOR_GREY = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    const ImVec4 COLOR_LIGHT_GREY = ImVec4(0.75f, 0.75f, 0.75f);
    const ImVec4 COLOR_DARK_GREY = ImVec4(0.25f, 0.25f, 0.25f);
    const ImVec4 COLOR_LIGHT_BLUE = ImVec4(0.5f, 0.5f, 1.0f);
    const ImVec4 COLOR_LIGHT_GREEN = ImVec4(0.5f, 1.0f, 0.5f);
    const ImVec4 COLOR_LIGHT_RED = ImVec4(1.0f, 0.5f, 0.5f);
    const ImVec4 COLOR_LIGHT_YELLOW = ImVec4(1.0f, 1.0f, 0.5f);
    const ImVec4 COLOR_LIGHT_CYAN = ImVec4(0.5f, 1.0f, 1.0f);
    const ImVec4 COLOR_LIGHT_MAGENTA = ImVec4(1.0f, 0.5f, 1.0f);
    const ImVec4 COLOR_LIGHT_ORANGE = ImVec4(1.0f, 0.75f, 0.5f);
    const ImVec4 COLOR_LIGHT_PURPLE = ImVec4(0.75f, 0.5f, 1.0f);
    const ImVec4 COLOR_LIGHT_BROWN = ImVec4(0.75f, 0.5f, 0.25f);
    const ImVec4 COLOR_DARK_BLUE = ImVec4(0.25f, 0.25f, 1.0f);
    const ImVec4 COLOR_DARK_GREEN = ImVec4(0.25f, 1.0f, 0.25f);
    const ImVec4 COLOR_DARK_RED = ImVec4(1.0f, 0.25f, 0.25f);
    const ImVec4 COLOR_DARK_YELLOW = ImVec4(1.0f, 1.0f, 0.25f);
    const ImVec4 COLOR_DARK_CYAN = ImVec4(0.25f, 1.0f, 1.0f);
    const ImVec4 COLOR_DARK_MAGENTA = ImVec4(1.0f, 0.25f, 1.0f);
    const ImVec4 COLOR_DARK_ORANGE = ImVec4(1.0f, 0.5f, 0.25f);
    const ImVec4 COLOR_DARK_PURPLE = ImVec4(0.5f, 0.25f, 1.0f);
    const ImVec4 COLOR_DARK_BROWN = ImVec4(0.5f, 0.25f, 0.125f);
}*/
// let's define the videostrip namespace vs
namespace vs
{

    int drawWindowInfo(vs::VideoFile &v);   // TODO: move as part of vsGui class
    int drawWindowInput(vs::VideoFile &v);

    // class vsgui contains all GUI classes, imgui windows management and rendering and opengl context management
    class vsGui
    {
        public:
            //constructor of vsgui class
            vsGui(){
                //nothing to do
            }
            // empty destructor of class vsGui
            ~vsGui(){
                //nothing to do
            }
            // int Setup();
            // function to initialize the vsgui class. Implicit pointer to the GLFW window 
            int Init();
            // New frame begin-end pair
            void NewFrame(); 
            // function to update the vsgui class
            void Update();
            // function to render the vsgui class
            void Render();

            inline
            int ShouldClose(){ return glfwWindowShouldClose(window); }   // main window has been asked to close

            // function to destroy the vsgui class
            void Destroy(); // can be also called Close()



        private:
            GLFWwindow* window; // pointer to the GLFW window
            ImVec4      clear_color = ImVec4(0.45f, 0.05f, 0.10f, 1.00f);
            ImFont*     current_font;

    };

} // namespace vs
 
#endif // _VS_GUI_H_