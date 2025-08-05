#pragma once
// Public API header for videostrip_core

// Core definitions for videostrip module
// This can be vconverted into an external library that can be called by either the CLI or GUI based frontend


#ifndef _VS_CORE_H_
#define _VS_CORE_H_

#include "headers.hpp"
#include "helper.hpp"
#include <iostream>
// #include <ctime>

// Let's define the videostrip namespace vs

namespace vs{
}

namespace vs{

    extern logger::ConsoleOutput logc; // global variable, but resolved in only one translation unit otherwise linker will complain

    // Structure that can hold video duration in hour, minutes and seconds
    typedef struct _vd{
        int hours;
        int minutes;
        int seconds;
    }vd;

    // Function that converts a a video duration in seconds (int) to a vd structure
    vd duration_to_vd(long int);

    // Object to store the video file information
    class VideoFile{
        public:
            VideoFile(){
                // create as empty invalid object
                is_valid = false;
                filename = "";
                output_folder="";
                width = height = fps = num_frames = 0;
            }
            VideoFile(std::string inputfile){
                is_valid = false;       //invalidate any preloaded video file info
                peekFile(inputfile);    // peek data from provided file
            }

            ~VideoFile(){
                // nothing to do, as no memory has been allocated so far
            }

            int     peekFile(); // given the filename, peek the file and populate the object. <filename> is the implicit argument
            int     peekFile(std::string filename); // given the filename, peek the file and populate the object.

            void    showInfo(); // dump the video file information to the console (could be a file or string)
            inline
            bool    isValid(){ return is_valid; }; // getter method for is_valid

            std::string filename;   // filename of the video file
            std::string filepath;   // full filepath including filename (?)
            std::string output_folder; // this should be a member of the pipeline, not the video
            int     width;
            int     height;
            float   fps;
            int     num_frames;
            vd      video_duration;
 
        private:
            bool is_valid;
    };

}

#endif