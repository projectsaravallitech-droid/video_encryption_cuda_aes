cmake_minimum_required(VERSION 3.20)
project(InternshipProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Provide the EXACT path to the OpenCV install folder
set(OpenCV_DIR "C:/opencv/build/install")

# 2. Find OpenCV
# This will pull in the Release libraries you built
find_package(OpenCV REQUIRED)

# 3. Status messages to verify paths in the Output Window
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenCV Headers: ${OpenCV_INCLUDE_DIRS}")

# 4. Collect all your cpp files (As requested, you'll manage the main() conflicts)
file(GLOB SRC_FILES "src/*.cpp")

# 5. Create executable
add_executable(app ${SRC_FILES})

# 6. Include OpenCV headers 
# We add the hardcoded path as a fallback to satisfy IntelliSense immediately
target_include_directories(app SYSTEM PRIVATE 
    ${OpenCV_INCLUDE_DIRS}
    "C:/opencv/build/install/include"
)

# 7. Link OpenCV
# Since you're using Release, ensure the VS Configuration (Top Bar) is set to Release
target_link_libraries(app PRIVATE ${OpenCV_LIBS})