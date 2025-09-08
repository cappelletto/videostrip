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
#include "../external/args.hxx"

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

    return 0;
}
