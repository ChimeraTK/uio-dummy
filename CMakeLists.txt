cmake_minimum_required(VERSION 3.16)
project("uio-dummy")

include(GNUInstallDirs)

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 03)
set(${PROJECT_NAME}_PATCH_VERSION 00)
set(CMAKE_PROJECT_VERSION ${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}.${${PROJECT_NAME}_PATCH_VERSION})

set(DKMS_DEST ${CMAKE_INSTALL_PREFIX}/src/uio-dummy-${CMAKE_PROJECT_VERSION})


install(DIRECTORY DESTINATION ${DKMS_DEST})

configure_file(${CMAKE_SOURCE_DIR}/dkms.conf.in dkms.conf @ONLY)
install(FILES
  "${CMAKE_BINARY_DIR}/dkms.conf"
  DESTINATION ${DKMS_DEST}
)

install(FILES
  "${CMAKE_SOURCE_DIR}/Makefile.kernel"
  DESTINATION ${DKMS_DEST}
  RENAME "Makefile"
)

configure_file(${CMAKE_SOURCE_DIR}/version.h.in version.h @ONLY)
file(GLOB uio-dummy_sources ${CMAKE_SOURCE_DIR}/*.h ${CMAKE_SOURCE_DIR}/*.c ${CMAKE_BINARY_DIR}/version.h)

install(FILES
  ${uio-dummy_sources}
  DESTINATION ${DKMS_DEST}
)

install(FILES
  "${CMAKE_SOURCE_DIR}/10-uio-dummy.rules"
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/udev/rules.d
)

install(FILES
  "${CMAKE_SOURCE_DIR}/uio-dummy.conf"
  DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/modules-load.d
)

configure_file(${CMAKE_SOURCE_DIR}/dkms-run.sh.in dkms-run.sh @ONLY)
install(CODE
  "execute_process(COMMAND ${CMAKE_COMMAND} -E env CMAKE_PROJECT_VERSION=${CMAKE_PROJECT_VERSION} BINDIR=${CMAKE_BINARY_DIR} bash ${CMAKE_BINARY_DIR}/dkms-run.sh)"
)
