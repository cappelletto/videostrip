// Core definitions for videostrip module
// This can be vconverted into an external library that can be called by either the CLI or GUI based frontend

// Let's define the videostrip namespace vs

#include "videostrip/vscore.h"

namespace vs{
    logger::ConsoleOutput logc; // single translation unit allowed to define logc (global variable within the namespace scope)
}

void vs::VideoFile::showInfo(){
    // print the video file information
    std::cout << "filename: " << filename << std::endl;
    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;
    std::cout << "fps: " << fps << std::endl;
    std::cout << "num_frames: " << num_frames << std::endl;
    std::cout << "video_duration: " << video_duration << std::endl;
}

int vs::VideoFile::peekFile(){
    // given the filename, peek the file and populate the object. <filename> is the implicit argument
    if (filename.c_str() == NULL){
        is_valid = false;
        return -1;
    }
    return vs::VideoFile::peekFile(filename);
}

int vs::VideoFile::peekFile(std::string inputfile){
    // test if inputfile is valid. If not, invalidate the object (or maybe just warn about new one being invalid)
    if (inputfile.c_str() == NULL){
        is_valid = false; // optional
        return -1;
    }

    //<create the capture object
    cv::VideoCapture capture(inputfile);
    if (! capture.isOpened()) {
        //error while opening the video input
        std::ostringstream ss;
        ss << red << "Unable to open video file: " << inputfile << std::endl;
        logc.error ("vscore", ss);
        is_valid = false;
        return -1;
    }
    //now we retrieve and print info about input video
    width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    fps = capture.get(cv::CAP_PROP_FPS);
    num_frames = capture.get(cv::CAP_PROP_FRAME_COUNT);
    // compute the duration of the video using the number of frames and the fps
    video_duration = (time_t) num_frames / fps; // fractional seconds // <--- integer conversion problem
    is_valid = true;
    filename = inputfile; // we update the filename with the input file
    return 0; // everything ok so far
}
