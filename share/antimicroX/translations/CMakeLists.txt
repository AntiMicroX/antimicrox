# This will ensure that the *.qm files will be stored
# in the right place within the build directory.
file(GLOB antimicroX_TRANSLATIONS antimicroX_*.ts)
list(APPEND antimicroX_TRANSLATIONS antimicroX.ts)
file(GLOB_RECURSE antimicroX_BASE_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE antimicroX_BASE_FORMS ${PROJECT_SOURCE_DIR}/src/*.ui)


    if(UPDATE_TRANSLATIONS)
        message("Update translations - lupdate ON")
        if(TRANS_KEEP_OBSOLETE)
            message("getting rid of old text entries: \"lupdate -noobsolete\" ON")
            QT5_CREATE_TRANSLATION(antimicroX_QMFILES ${antimicroX_BASE_SOURCES}
            ${antimicroX_BASE_FORMS} ${antimicroX_TRANSLATIONS} OPTIONS -Inobsolete)
        else()
            QT5_CREATE_TRANSLATION(antimicroX_QMFILES ${antimicroX_BASE_SOURCES}
            ${antimicroX_BASE_FORMS} ${antimicroX_TRANSLATIONS})
        endif(TRANS_KEEP_OBSOLETE)
    else()
        QT5_ADD_TRANSLATION(antimicroX_QMFILES ${antimicroX_TRANSLATIONS})
    endif(UPDATE_TRANSLATIONS)


add_custom_target(updateqm DEPENDS ${antimicroX_QMFILES})
install(FILES ${antimicroX_QMFILES} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/antimicroX/translations")

set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM true)
