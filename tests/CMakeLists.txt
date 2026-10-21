# Exercise the production profile reader without requiring physical controllers.
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
set(profile_test_sources)
foreach(source IN LISTS antimicrox_SOURCES antimicrox_HEADERS_MOC)
    list(APPEND profile_test_sources "${PROJECT_SOURCE_DIR}/${source}")
endforeach()

# Generated-file properties are directory-scoped with older CMake policies.
set_source_files_properties(${antimicrox_FORMS_HEADERS} ${antimicrox_RESOURCES_RCC}
    PROPERTIES GENERATED TRUE)

add_executable(ProfileXmlTests
    testgamecontrollersetxml.cpp
    ${profile_test_sources}
    ${antimicrox_FORMS_HEADERS}
    ${antimicrox_RESOURCES_RCC}
)
target_include_directories(ProfileXmlTests PRIVATE ${SDL2_INCLUDE_DIRS}/SDL2)
target_link_libraries(ProfileXmlTests
    Qt${QT_VERSION_MAJOR}::Test
    ${QT_LIBS}
    ${X11_LIBS}
    ${SDL2_LIBRARIES}
    ${EXTRA_LIBS}
    ${WIN_LIBS}
)
add_test(NAME ProfileXmlTests COMMAND ProfileXmlTests)
set_tests_properties(ProfileXmlTests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;SDL_VIDEODRIVER=dummy"
    TIMEOUT 60
)
