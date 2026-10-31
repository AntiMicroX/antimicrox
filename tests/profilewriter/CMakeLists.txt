cmake_minimum_required(VERSION 3.16)
project(ProfileWriterTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
enable_testing()
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)

add_executable(ProfileWriterTests
    testxmlconfigwriter.cpp
    ../../src/xmlconfigwriter.cpp
    ../../src/xmlconfigwriter.h
    ../../src/xml/inputdevicexml.h
)
target_include_directories(ProfileWriterTests PRIVATE ../../src)
target_link_libraries(ProfileWriterTests PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME ProfileWriterTests COMMAND ProfileWriterTests -o -,txt -o profile-writer-results.txt,txt)
set_tests_properties(ProfileWriterTests PROPERTIES TIMEOUT 30)
