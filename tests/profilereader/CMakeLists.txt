cmake_minimum_required(VERSION 3.16)
project(ProfileReaderTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
enable_testing()
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Test)
add_executable(ProfileReaderTests
    testxmlconfigreader.cpp
    ../../src/xmlconfigreader.cpp
    ../../src/xmlconfigreader.h
    ../../src/xmlconfigmigration.h
    ../../src/xml/inputdevicexml.h
)
target_include_directories(ProfileReaderTests PRIVATE ../../src)
target_link_libraries(ProfileReaderTests PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Test)
add_test(NAME ProfileReaderTests COMMAND ProfileReaderTests -o -,txt -o profile-reader-results.txt,txt)
set_tests_properties(ProfileReaderTests PROPERTIES TIMEOUT 30)
