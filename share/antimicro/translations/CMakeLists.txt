# This will ensure that the *.qm files will be stored
# in the right place within the build directory.
file(GLOB antimicro_TRANSLATIONS antimicro_*.ts)
list(APPEND antimicro_TRANSLATIONS antimicro.ts)
file(GLOB_RECURSE antimicro_BASE_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE antimicro_BASE_FORMS ${PROJECT_SOURCE_DIR}/src/*.ui)


    if(UPDATE_TRANSLATIONS)
            message("Update translations - lupdate ON")
            QT5_CREATE_TRANSLATION(antimicro_QMFILES ${antimicro_BASE_SOURCES}
                ${antimicro_BASE_FORMS} ${antimicro_TRANSLATIONS})
    else()
        QT5_ADD_TRANSLATION(antimicro_QMFILES ${antimicro_TRANSLATIONS})
    endif(UPDATE_TRANSLATIONS)


add_custom_target(updateqm DEPENDS ${antimicro_QMFILES})
install(FILES ${antimicro_QMFILES} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/antimicro/translations")

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)
