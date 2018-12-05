cmake_minimum_required(VERSION 2.8.11)
project(aboutdialogClass)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)

add_executable(aboutdialogClass testaboutdialog.cpp)
target_link_libraries(aboutdialogClass antilib Qt5::Test)

add_test (NAME testAboutDialog COMMAND aboutdialogClass)
