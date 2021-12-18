/**
 * @file mad_test.cpp
 * @author Jose Cappelletto (cappelletto@gmail.com)
 * @brief Measurability Area Detector, extended version of LAD test
 *        Sandbox module for testing core and extended functionalities and integration of Geotiff, OpenCV, CGAL & GDAL
 *        Part of PhD project on predicting landable areas for autonomous vehicles using remotely sensed data
 *        Ocean Perception Lab. University of Southampton, UK. 
 *        Visit: https://oceans.soton.ac.uk
 * @version 3.6-DualEnv [local + Iridis5]
 * @date 2021-11-18
 * 
 * @copyright Copyright (c) 2020-2021
 * 
 */
#include <iostream>
#include <args.hxx>

#include <headers.h>
#include <helper.h>

using namespace std;
using namespace cv;

/*!
    @fn     int main(int argc, char* argv[])
    @brief  Main function
*/

logger::ConsoleOutput logc; // as a global variable, we are Ok with this

int main(int argc, char *argv[])
{

    logc.warn("main", "Alpha version. Sandbox for module building and testing");

    // cout << "\tOpenCV version:\t" << yellow << CV_VERSION << reset << endl;
    // cout << "\tGit commit:\t" << yellow << GIT_COMMIT << reset << endl;
    // cout << "\tMode:\t\t" << yellow << CMAKE_BUILD_TYPE << reset << endl;
    // cout << cv::getBuildInformation() << std::endl;
    return 0;
}
