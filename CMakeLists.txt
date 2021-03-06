project(address-book-service C CXX)

cmake_minimum_required(VERSION 2.8.9)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

include(FindPkgConfig)
# Standard install paths
include(GNUInstallDirs)

find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Versit REQUIRED)
find_package(Qt5Contacts REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(LibPhoneNumber REQUIRED)

find_program(DBUS_RUNNER dbus-test-runner)
find_program(INTLTOOL_MERGE_EXECUTABLE intltool-merge)
find_program(INTLTOOL_EXTRACT_EXECUTABLE intltool-extract)

add_definitions(-DQT_NO_KEYWORDS)

pkg_check_modules(GLIB REQUIRED glib-2.0>=2.32)
pkg_check_modules(GIO REQUIRED gio-2.0>=2.32)
pkg_check_modules(EDATASERVER REQUIRED libedataserver-1.2>=3.8)
pkg_check_modules(FOLKS REQUIRED folks>=0.9.0)
pkg_check_modules(FOLKS_EDS REQUIRED folks-eds)
pkg_check_modules(GLIB_ACCOUNTS REQUIRED libaccounts-glib)
pkg_check_modules(MESSAGING_MENU REQUIRED messaging-menu)
pkg_check_modules(URL_DISPATCHER REQUIRED url-dispatcher-1)
pkg_check_modules(AccountsQt5 REQUIRED accounts-qt5)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)

if(EDATASERVER_VERSION VERSION_LESS "3.16")
    set(EVOLUTION_API_3_17 "0")
    set(EVOLUTION_ADDRESSBOOK_SERVICE_NAME "org.gnome.evolution.dataserver.AddressBook6")
    set(EVOLUTION_SOURCE_SERVICE_NAME "org.gnome.evolution.dataserver.Sources3")
elseif(EDATASERVER_VERSION VERSION_LESS "3.24")
    set(EVOLUTION_API_3_17 "1")
    set(EVOLUTION_ADDRESSBOOK_SERVICE_NAME "org.gnome.evolution.dataserver.AddressBook9")
    set(EVOLUTION_SOURCE_SERVICE_NAME "org.gnome.evolution.dataserver.Sources4")
else()
    set(EVOLUTION_API_3_17 "1")
    set(EVOLUTION_ADDRESSBOOK_SERVICE_NAME "org.gnome.evolution.dataserver.AddressBook9")
    set(EVOLUTION_SOURCE_SERVICE_NAME "org.gnome.evolution.dataserver.Sources5")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
add_definitions(-std=c++11)

# I18n
set(PACKAGE ${CMAKE_PROJECT_NAME})
set(GETTEXT_PACKAGE "address-book-service")
add_definitions(-DGETTEXT_PACKAGE="${GETTEXT_PACKAGE}"
                -DGETTEXT_LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}")


# Coverage tools
OPTION(ENABLE_COVERAGE "Build with coverage analysis support" OFF)
if(ENABLE_COVERAGE)
    message(STATUS "Using coverage flags")
    find_program(COVERAGE_COMMAND gcov)
    if(NOT COVERAGE_COMMAND)
        message(FATAL_ERROR "gcov command not found")
    endif()
    SET(CMAKE_C_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
    include(${CMAKE_SOURCE_DIR}/cmake/lcov.cmake)
endif()

# Address Sanitizer
OPTION(ENABLE_ADDRSANITIZER "Build with address sanitizer support" OFF)
if(ENABLE_ADDRSANITIZER)
    message(STATUS "Using address sanitizer flags")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()


configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
               IMMEDIATE @ONLY)
add_custom_target(uninstall "${CMAKE_COMMAND}"
                  -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

enable_testing()

add_custom_target(check)
add_custom_command(TARGET check
		   COMMAND ctest -V
		   WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(common)
add_subdirectory(eds-extension)
add_subdirectory(lib)
add_subdirectory(src)
add_subdirectory(contacts)
add_subdirectory(updater)
add_subdirectory(upstart)
add_subdirectory(dbus)
add_subdirectory(po)
#add_subdirectory(eds-test)

OPTION(ENABLE_TESTS "Build tests" ON)
if(ENABLE_TESTS)
    add_subdirectory(3rd_party)
    add_subdirectory(tests)
endif()
set(TEST_DATA_DIR "${CMAKE_SOURCE_DIR}/tests/data")

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
