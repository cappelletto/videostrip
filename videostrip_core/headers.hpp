/**
 * @file headers.h
 * @author Jose Cappelletto (cappelletto@gmail.com)
 * @brief Single collection of global libraries required in major modules
 * @version 0.2
 * @date 2020-07-03
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef _PROJECT_HEADERS_H_

#define _PROJECT_HEADERS_H_

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <chrono>
#include <omp.h>
#include <immintrin.h>

# include "../external/args.hxx" //
/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>
// #include "opencv2/core/ocl.hpp"
// #include "opencv2/imgproc.hpp"
// #include "opencv2/highgui.hpp"
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>
// #include <opencv2/features2d.hpp>
// #include "opencv2/calib3d.hpp"
//#include <opencv2/xfeatures2d.hpp>

// CUDA specific libraries
#if USE_GPU
    #include <opencv2/cudafilters.hpp>
    #include "opencv2/cudafeatures2d.hpp"
    #include "opencv2/xfeatures2d/cuda.hpp"
#endif

// escape based colour codes for console output
const std::string red("\033[1;31m");
const std::string green("\033[1;32m");
const std::string yellow("\033[1;33m");
const std::string blue("\033[1;34m");
const std::string purple("\033[1;35m");
const std::string cyan("\033[1;36m");

const std::string light_red("\033[0;31m");
const std::string light_green("\033[0;32m");
const std::string light_yellow("\033[0;33m");
const std::string light_blue("\033[0;34m");
const std::string light_purple("\033[0;35m");
const std::string light_cyan("\033[0;36m");

const std::string reset("\033[0m");
const std::string highlight("\033[30;43m");

#endif // _PROJECT_HEADERS_H_