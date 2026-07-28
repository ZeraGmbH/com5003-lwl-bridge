file(GLOB XML_CONFIGS
    *.xml
)

file(GLOB XSD_SCHEMA
    *.xsd
    )

add_custom_target(com5003-lwl-bridge-configs ALL
    SOURCES ${XSD_SCHEMA} ${XML_CONFIGS}
    COMMAND xmllint --noout --schema ${XSD_SCHEMA} ${XML_CONFIGS}
    )

install(
    FILES ${XML_CONFIGS}
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/zera/com5003lwlbridge
    )

