cmake_minimum_required(VERSION 3.20)
project(videostrip_opencv_superbuild LANGUAGES C CXX)

include(ExternalProject)

# ---- Version knobs you can adjust in one place ----
set(OPENCV_VERSION 4.12.0 CACHE STRING "OpenCV version to build")
set(OPENCV_WITH_CONTRIB OFF CACHE BOOL "Build OpenCV contrib modules")
set(OPENCV_BUILD_WORLD ON CACHE BOOL "Build opencv_world for fewer .so files")

# ---- Install prefix inside the repo (vendored) ----
# We use a fixed path under source to make it easy to reference & cache in CI.
set(OPENCV_VENDOR_PREFIX "${CMAKE_SOURCE_DIR}/../_install/opencv-${OPENCV_VERSION}")
file(MAKE_DIRECTORY "${OPENCV_VENDOR_PREFIX}")

# ---- Repos/URLs ----
set(OPENCV_GIT_URL          https://github.com/opencv/opencv.git)
set(OPENCV_CONTRIB_GIT_URL  https://github.com/opencv/opencv_contrib.git)

# ---- Build options (lean: disable LAPACK/BLAS to avoid cblas_* deps) ----
set(OPENCV_CMAKE_ARGS
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_INSTALL_PREFIX=${OPENCV_VENDOR_PREFIX}
  -DBUILD_LIST=core,imgproc,imgcodecs,videoio,features2d,flann,calib3d
  -DBUILD_TESTS=OFF
  -DBUILD_PERF_TESTS=OFF
  -DBUILD_EXAMPLES=OFF
  -DBUILD_JAVA=OFF
  -DBUILD_opencv_world=$<IF:$<BOOL:${OPENCV_BUILD_WORLD}>,ON,OFF>
  -DWITH_LAPACK=OFF
  -DWITH_OPENCL=OFF
  -DWITH_IPP=OFF
  -DWITH_TBB=OFF
  -DWITH_CUDA=OFF
  -DWITH_MSMF=OFF
  -DWITH_GTK=ON
  -DWITH_FFMPEG=ON
)

if(OPENCV_WITH_CONTRIB)
  list(APPEND OPENCV_CMAKE_ARGS
    -DOPENCV_EXTRA_MODULES_PATH=${CMAKE_BINARY_DIR}/opencv_contrib/modules
  )
endif()

# ---- ExternalProject to fetch & install OpenCV ----
ExternalProject_Add(opencv_src
  GIT_REPOSITORY   ${OPENCV_GIT_URL}
  GIT_TAG          ${OPENCV_VERSION}
  UPDATE_COMMAND   ""
  PATCH_COMMAND    ""
  CMAKE_ARGS       ${OPENCV_CMAKE_ARGS}
  BUILD_BYPRODUCTS ${OPENCV_VENDOR_PREFIX}/lib/cmake/opencv4/OpenCVConfig.cmake
  INSTALL_DIR      ${OPENCV_VENDOR_PREFIX}
  TEST_COMMAND     "" # skip tests
)

if(OPENCV_WITH_CONTRIB)
  ExternalProject_Add_Step(opencv_src fetch_contrib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/opencv_contrib
    COMMAND git clone --depth 1 --branch ${OPENCV_VERSION} ${OPENCV_CONTRIB_GIT_URL} ${CMAKE_BINARY_DIR}/opencv_contrib
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDEES download
    DEPENDERS configure
  )
endif()

# ---- Expose a "done" target and a hint file for consumers ----
add_custom_target(opencv_vendor ALL DEPENDS opencv_src)
file(WRITE "${CMAKE_BINARY_DIR}/OpenCVVendorDir.cmake"
  "set(VENDOR_OPENCV_DIR \"${OPENCV_VENDOR_PREFIX}/lib/cmake/opencv4\")\n"
)
