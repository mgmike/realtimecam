cmake_minimum_required(VERSION 3.10)
project(Dishcam VERSION 1.0)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_executable(Dishcam main.cpp ThreaddedQueue.cpp Yolo.cpp)

# Version and header file.
configure_file(DishcamConfig.h.in DishcamConfig.h)
target_include_directories(Dishcam PUBLIC "${PROJECT_BINARY_DIR}")

# Add a library and make it optional Check out https://cmake.org/cmake/help/latest/guide/tutorial/Adding%20a%20Library.html
# add_library(ThreaddedQueue)
# target_link_libraries(Dishcam PUBLIC ThreaddedQueue)
# target_include_directories(Dishcam PUBLIC 
#                           "${PROJECT_BINARY_DIR}"
#                           "${PROJECT_SOURCE_DIR}/ThreaddedQueue"
#                           )             
# option(USE_TQ "Use Threadded queue" ON)

option(OpenCV_STATIC ON)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)
#find_package(Queue REQUIRED)
#find_package(Mutex REQUIRED)
# include_directories( ${OpenCV_INCLUDE_DIRS} )
# include_directories( ${OpenCV_INCLUDE_DIRS}/opencv2 )

# Set Static Link Runtime Library
if( OpenCV_STATIC )
  foreach( flag_var
           CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
           CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
           CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
           CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO )
    if( ${flag_var} MATCHES "/MD" )
      string( REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}" )
    endif()
  endforeach()
endif()

if( OpenCV_FOUND )
  # Additional Include Directories
  include_directories( ${OpenCV_INCLUDE_DIRS} )


  # Additional Library Directories
  link_directories( ${OpenCV_LIB_DIR} )

  # Additional Dependencies
  target_link_libraries( Dishcam ${OpenCV_LIBS} )
endif()

message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

target_link_libraries(Dishcam ${OpenCV_LIBS})
target_link_libraries(Dishcam Threads::Threads)
#list( APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -pthread `pkg-config --cflags --libs opencv`")