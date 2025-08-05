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

#endif // _PROJECT_HEADERS_H_