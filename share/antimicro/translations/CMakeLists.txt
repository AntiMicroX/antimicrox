# This will ensure that the *.qm files will be stored
# in the right place within the build directory.
file(GLOB antimicro_TRANSLATIONS antimicro_*.ts)

if(USE_QT5)
    QT5_ADD_TRANSLATION(antimicro_QMFILES ${antimicro_TRANSLATIONS})
else()
    QT4_ADD_TRANSLATION(antimicro_QMFILES ${antimicro_TRANSLATIONS})
endif(USE_QT5)

add_custom_target(updateqm DEPENDS ${antimicro_QMFILES})
install(FILES ${antimicro_QMFILES} DESTINATION "share/antimicro/translations")
