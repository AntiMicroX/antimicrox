add_custom_target(manpage)
add_custom_command(TARGET manpage PRE_BUILD
    COMMAND gzip -k -f "${PROJECT_SOURCE_DIR}/other/antimicro.1" VERBATIM
)
install(FILES antimicro.1.gz DESTINATION "share/man/man1")
