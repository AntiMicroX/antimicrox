add_custom_target(appdata)
add_custom_command(TARGET appdata PRE_BUILD
    COMMAND msgfmt "${PROJECT_SOURCE_DIR}/other/appdata/PO_files/en.po" -o "${PROJECT_SOURCE_DIR}/other/appdata/PO_files/en.mo" VERBATIM
    COMMAND itstool -i "${PROJECT_SOURCE_DIR}/other/appdata/appdata.its" -j "${PROJECT_SOURCE_DIR}/other/appdata/antimicro.appdata.xml.in" -o "antimicro.appdata.xml" "${PROJECT_SOURCE_DIR}/other/appdata/PO_files/*.mo" VERBATIM
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/antimicro.appdata.xml" DESTINATION "share/appdata")
