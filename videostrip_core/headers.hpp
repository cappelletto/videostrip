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
#include <cmath>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <immintrin.h>
#include <omp.h>
#include <sstream>
#include <stdexcept>
#include <vector>

# include "../external/args.hxx" //
/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>
#include <opencv2/video.hpp>
#include <opencv2/videoio.hpp>

#endif // _PROJECT_HEADERS_H_