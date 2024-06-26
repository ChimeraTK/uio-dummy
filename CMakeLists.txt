cmake_minimum_required(VERSION 3.16)
project("uio-dummy")

set(${PROJECT_NAME}_MAJOR_VERSION 00)
set(${PROJECT_NAME}_MINOR_VERSION 01)
set(${PROJECT_NAME}_PATCH_VERSION 02)
include(cmake/set_version_numbers.cmake)

find_program(DKMS dkms)
find_program(UNAME uname REQUIRED)
find_program(MAKE make REQUIRED)

include(GNUInstallDirs)

set(UDEV_RULES_PATH
    "${CMAKE_INSTALL_FULL_SYSCONFDIR}/udev/rules.d"
    CACHE PATH "Installation folder for udev rules")
set(DKMS_TARGET_PATH
    "${CMAKE_INSTALL_PREFIX}/src/${PROJECT_NAME}-${${PROJECT_NAME}_VERSION}"
    CACHE PATH "Installation folder for DKMS sources")
set(MODULES_LOAD_PATH
    "${CMAKE_INSTALL_FULL_SYSCONFDIR}/modules-load.d"
    CACHE PATH "Installation folder for automatic module load files")
execute_process(COMMAND ${UNAME} -r OUTPUT_VARIABLE KERNEL_VERSION_RAW)
string(STRIP "${KERNEL_VERSION_RAW}" KERNEL_VERSION)

# First check if we can build for the currently running kernel version
set(KERNEL_SRC_CANDIDATE "/lib/modules/${KERNEL_VERSION}/build")
if (NOT EXISTS ${KERNEL_SRC_CANDIDATE})
    message(STATUS "Cannot build module for the currently running kernel, trying to find a suitable version")
    # Find all potential kernel modules
    file(GLOB MODULES LIST_DIRECTORIES true /lib/modules/*/build)

    # Sort and reverse list to have the latest version as first 
    list(SORT MODULES)
    if (MODULES)
        list(GET MODULES -1 KERNEL_SRC_CANDIDATE)

        string(REPLACE "/" ";" NEWEST_MODULE_PATH ${KERNEL_SRC_CANDIDATE})
        list(GET NEWEST_MODULE_PATH 3 KERNEL_VERSION)
    else()
        message(STATUS "No kernel source found, not building module")
        set(KERNEL_VERSION kernel-NOTFOUND)
    endif()
endif()
set(KERNEL_SRC ${KERNEL_SRC_CANDIDATE}
        CACHE PATH "Folder that contains the kernel development parts")

message(STATUS "Putting udev rules into ${UDEV_RULES_PATH}")
message(STATUS "Putting DKMS sources into ${DKMS_TARGET_PATH}")
message(STATUS "Putting module load configuration into ${MODULES_LOAD_PATH}")
message(STATUS "Using kernel build environment from ${KERNEL_SRC}")

configure_file(Makefile.in kbuild/Makefile @ONLY)
configure_file(dkms.conf.in dkms.conf @ONLY)
configure_file(version.h.in kbuild/version.h @ONLY)
file(COPY uio-dummy.c DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/kbuild)

install(FILES uio-dummy.c DESTINATION ${DKMS_TARGET_PATH})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kbuild/Makefile
              ${CMAKE_CURRENT_BINARY_DIR}/dkms.conf
              ${CMAKE_CURRENT_BINARY_DIR}/kbuild/version.h
        DESTINATION ${DKMS_TARGET_PATH})
install(FILES uio-dummy.rules DESTINATION ${UDEV_RULES_PATH})
install(FILES uio-dummy.conf DESTINATION ${MODULES_LOAD_PATH})

if (KERNEL_VERSION)
    add_custom_command(
      OUTPUT uio-dummy.ko
      COMMAND ${MAKE} KERNEL_SRC=${KERNEL_SRC}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/kbuild
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kbuild/Makefile
      VERBATIM)
    add_custom_target(module ALL DEPENDS uio-dummy.ko)
endif()

if (DKMS)
  add_custom_target(
    dkms-install COMMAND ${DKMS} install -m ${PROJECT_NAME} -v
                         ${${PROJECT_NAME}_VERSION} -k ${KERNEL_VERSION})

  add_custom_target(
    dkms-remove COMMAND ${DKMS} remove -m ${PROJECT_NAME} -v
                      ${${PROJECT_NAME}_VERSION} -k ${KERNEL_VERSION})
else()
    message(WARNING "DKMS executable not found, disabling DKMS targets")
endif()
