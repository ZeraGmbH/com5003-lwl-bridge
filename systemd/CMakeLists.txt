set(SYSTEMD_UNITS
    com5003lwlbridge.service
    )

add_custom_target(com5003-lwl-bridge-systemd-units ALL
    DEPENDS ${SYSTEMD_UNITS}
    SOURCES ${SYSTEMD_UNITS}
    COMMAND echo "Install systemd units..."
    )

install(
    FILES ${SYSTEMD_UNITS}
    DESTINATION ${SYSTEMD_UNIT_INSTALL_PATH}
    )

