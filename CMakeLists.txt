cmake_minimum_required(VERSION 3.7)

project(asteroids-emu)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

find_package(SDL2 REQUIRED)

add_executable(
  ${PROJECT_NAME}
  "getopt/getopt.c"
  cpu6502.cpp
  dvg.cpp
  input.cpp
  main.cpp
  memory.cpp
  mmio.cpp
  renderer.cpp
  registers.cpp)

target_link_libraries(${PROJECT_NAME} SDL2::Main)

# Build rom header (romcode.h)
if(WIN32)
  find_program(POWERSHELL_PATH NAMES powershell)

  add_custom_command(
    OUTPUT "romcode.h" PRE_BUILD
    COMMAND
      ${POWERSHELL_PATH} "-NoProfile"
      "${CMAKE_CURRENT_SOURCE_DIR}/roms/generate-romcode.ps1" "-Output"
      "${CMAKE_CURRENT_BINARY_DIR}/romcode.h"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/roms"
    COMMENT "Generating romcode.h"
    VERBATIM)

  add_custom_target(ROMCODE_H ALL
                    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/romcode.h")
endif()

# Copy SDL dll to target directory
if(WIN32)
  # Assume that SDL2.dll is in same folder as SDL2_LIBRARY e.g.
  # "C:/Users/jeffm/scoop/apps/sdl2/current/lib"
  cmake_path(REMOVE_FILENAME SDL2_LIBRARY OUTPUT_VARIABLE SDL2_DLL_DIR)

  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL2_DLL_DIR}/SDL2.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()
