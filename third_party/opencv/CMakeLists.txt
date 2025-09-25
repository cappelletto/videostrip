cmake_minimum_required(VERSION 3.20)
project(videostrip_opencv_superbuild LANGUAGES C CXX)

include(ExternalProject)

# ---- Version knobs ----
set(OPENCV_VERSION 4.12.0 CACHE STRING "OpenCV version to build")
set(OPENCV_WITH_CONTRIB OFF CACHE BOOL "Build OpenCV contrib modules")

# IMPORTANT: do NOT build opencv_world unless you also build highgui.
set(OPENCV_BUILD_WORLD OFF CACHE BOOL "Build opencv_world for fewer .so files")

# ----- Vendor prefix (parent can pre-set OPENCV_VENDOR_PREFIX) via CMake invocation -----
if(NOT DEFINED OPENCV_VENDOR_PREFIX)
  # CMAKE_CURRENT_LIST_DIR == <repo>/third_party/opencv
  get_filename_component(_third_party_dir "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
  set(OPENCV_VENDOR_PREFIX "${_third_party_dir}/_install/opencv-${OPENCV_VERSION}")
endif()
file(MAKE_DIRECTORY "${OPENCV_VENDOR_PREFIX}")

# ---- Repos/URLs ----
set(OPENCV_GIT_URL          https://github.com/opencv/opencv.git)
set(OPENCV_CONTRIB_GIT_URL  https://github.com/opencv/opencv_contrib.git)

# ---- Lean options (no LAPACK/BLAS -> no cblas_*; no GUI) ----
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
  -DWITH_GTK=OFF    # no highgui in BUILD_LIST
  -DWITH_FFMPEG=ON
  -DWITH_AVIF=OFF
)

# This is for size reduction: disable some extra modules/dependencies
# CI jobs we running out of memory building OpenCV
list(APPEND OPENCV_CMAKE_ARGS
  -DBUILD_OPENEXR=OFF
  -DBUILD_OPENJPEG=OFF
  -DBUILD_WEBP=OFF
  -DBUILD_TIFF=OFF
  -DBUILD_JASPER=OFF
  -DBUILD_ZLIB=OFF        # use system zlib
  -DWITH_IMGCODEC_HDR=OFF # optional extra diet
  -DWITH_IMGCODEC_SUNRASTER=OFF
  -DWITH_IMGCODEC_PXM=OFF
  -DWITH_IMGCODEC_PFM=OFF

  # keep JPEG+PNG (needed by videostrip)
  -DWITH_JPEG=ON  -DBUILD_JPEG=ON
  -DWITH_PNG=ON   -DBUILD_PNG=ON

  # build-time/size reductions
  # TODO: implement MARCH/CPU settings via discovery or if LEAN build is set
  -DCPU_BASELINE=AVX2      # single baseline (adjust if older CPU needed)
  -DCPU_DISPATCH=          # avoid multi-dispatch variants
)

if(OPENCV_WITH_CONTRIB)
  list(APPEND OPENCV_CMAKE_ARGS
    -DOPENCV_EXTRA_MODULES_PATH=${CMAKE_BINARY_DIR}/opencv_contrib/modules
  )
endif()

ExternalProject_Add(opencv_src
  GIT_REPOSITORY   ${OPENCV_GIT_URL}
  GIT_TAG          ${OPENCV_VERSION}
  UPDATE_COMMAND   ""
  PATCH_COMMAND    ""
  CMAKE_ARGS       ${OPENCV_CMAKE_ARGS}
  BUILD_BYPRODUCTS ${OPENCV_VENDOR_PREFIX}/lib/cmake/opencv4/OpenCVConfig.cmake
  INSTALL_DIR      ${OPENCV_VENDOR_PREFIX}
  TEST_COMMAND     ""
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

add_custom_target(opencv_vendor ALL DEPENDS opencv_src)
file(WRITE "${CMAKE_BINARY_DIR}/OpenCVVendorDir.cmake"
  "set(VENDOR_OPENCV_DIR \"${OPENCV_VENDOR_PREFIX}/lib/cmake/opencv4\")\n"
)
