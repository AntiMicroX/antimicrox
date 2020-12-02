# This will ensure that the *.qm files will be stored
# in the right place within the build directory.
file(GLOB antimicrox_TRANSLATIONS antimicrox_*.ts)
list(APPEND antimicrox_TRANSLATIONS antimicrox.ts)
file(GLOB_RECURSE antimicrox_BASE_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE antimicrox_BASE_FORMS ${PROJECT_SOURCE_DIR}/src/*.ui)


if (UPDATE_TRANSLATIONS)
    message("Update translations - lupdate ON")
    if (TRANS_KEEP_OBSOLETE)
        message("getting rid of old text entries: \"lupdate -noobsolete\" ON")
        QT5_CREATE_TRANSLATION(antimicrox_QMFILES ${antimicrox_BASE_SOURCES}
                ${antimicrox_BASE_FORMS} ${antimicrox_TRANSLATIONS} OPTIONS -Inobsolete)
    else ()
        QT5_CREATE_TRANSLATION(antimicrox_QMFILES ${antimicrox_BASE_SOURCES}
                ${antimicrox_BASE_FORMS} ${antimicrox_TRANSLATIONS})
    endif (TRANS_KEEP_OBSOLETE)
else ()
    QT5_ADD_TRANSLATION(antimicrox_QMFILES ${antimicrox_TRANSLATIONS})
endif (UPDATE_TRANSLATIONS)


add_custom_target(updateqm DEPENDS ${antimicrox_QMFILES})
install(FILES ${antimicrox_QMFILES} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/antimicrox/translations")

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)
