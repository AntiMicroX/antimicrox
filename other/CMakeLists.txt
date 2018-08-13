add_subdirectory(appdata)

add_custom_target(manpage)
add_custom_command(TARGET manpage PRE_BUILD
    COMMAND gzip -c "${PROJECT_SOURCE_DIR}/other/antimicro.1" > "antimicro.1.gz" VERBATIM
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/antimicro.1.gz" DESTINATION "share/man/man1")
