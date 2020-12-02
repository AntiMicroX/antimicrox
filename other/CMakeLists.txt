add_subdirectory(appdata)

add_custom_target(manpage)
add_custom_command(TARGET manpage PRE_BUILD
        COMMAND gzip -c -9 -n "${PROJECT_SOURCE_DIR}/other/antimicrox.1" > "antimicrox.1.gz" VERBATIM
        )
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/antimicrox.1.gz" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/man/man1")
