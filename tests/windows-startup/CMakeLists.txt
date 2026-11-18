cmake_minimum_required(VERSION 3.16)
project(WindowsStartupTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
enable_testing()
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

add_executable(testwinstartup testwinstartup.cpp ../../src/winstartup.cpp)
target_include_directories(testwinstartup PRIVATE ../../src)
target_link_libraries(testwinstartup PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Test
    taskschd uuid ole32 oleaut32 advapi32)
add_test(NAME WindowsStartup COMMAND testwinstartup -o test-results.txt,txt)
