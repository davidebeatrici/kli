cmake_minimum_required(VERSION 3.12)

project("K(eyboard) L(ayout) I(nstaller)"
  VERSION 1.0.0.0
  DESCRIPTION "An application that takes care of installing the specified keyboard layout for you."
  LANGUAGES C
)

configure_file("resources/kli.rc.in" "kli.rc")

add_executable(kli
  "main.c"

  "Config.c"
  "Config.h"
  "Dialog.c"
  "Dialog.h"
  "Input.c"
  "Input.h"
  "Layout.c"
  "Layout.h"
  "Str.c"
  "Str.h"

  "kli.rc"
  "resources/kli.manifest"
)

find_library(LIB_INIH inih REQUIRED)
find_path(INCLUDE_INIH "ini.h")

target_include_directories(kli PRIVATE ${INCLUDE_INIH})
target_link_libraries(kli PRIVATE ${LIB_INIH})

target_link_options(kli PRIVATE "/MANIFESTUAC:level='requireAdministrator'")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Hide console while keeping main() as entry point
  target_link_options(kli PRIVATE
    "/SUBSYSTEM:WINDOWS"
    "/ENTRY:mainCRTStartup"
  )
endif()
