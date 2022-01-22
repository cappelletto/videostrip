// Core definitions for videostrip module
// This can be vconverted into an external library that can be called by either the CLI or GUI based frontend

#pragma once

#include <iostream>
#include <ctime>

// Let's define the videostrip namespace vs

namespace vs{

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
            time_t  video_duration;

        private:
            bool is_valid;
    };

}