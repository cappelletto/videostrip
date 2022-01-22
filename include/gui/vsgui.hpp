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


// let's define the videostrip namespace vs
namespace vs
{

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