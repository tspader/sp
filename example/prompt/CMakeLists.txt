cmake_minimum_required(VERSION 3.20)

project(prompt_demo LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(PROMPT_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/prompt.c")
set(PROMPT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(PROMPT_ARTIFACT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")

file(MAKE_DIRECTORY "${PROMPT_ARTIFACT_DIR}")

add_executable(prompt "${PROMPT_SOURCE}")
target_include_directories(prompt PRIVATE "${PROMPT_INCLUDE_DIR}")
target_compile_options(prompt PRIVATE -Wall -O2)
target_link_libraries(prompt PRIVATE m)
set_target_properties(prompt PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROMPT_ARTIFACT_DIR}"
)

find_program(ZIG_EXECUTABLE zig)

function(add_prompt_cross_target name target_triple)
  set(static_flag "")
  if(ARGC GREATER 2)
    set(static_flag "-static")
  endif()

  set(output "${PROMPT_ARTIFACT_DIR}/prompt-${name}")
  add_custom_command(
    OUTPUT "${output}"
    COMMAND "${ZIG_EXECUTABLE}" cc
      -target "${target_triple}"
      -I"${PROMPT_INCLUDE_DIR}"
      -std=c99
      -Wall
      -O2
      ${static_flag}
      "${PROMPT_SOURCE}"
      -o "${output}"
      -lm
    DEPENDS
      "${PROMPT_SOURCE}"
      "${PROMPT_INCLUDE_DIR}/sp.h"
      "${PROMPT_INCLUDE_DIR}/sp/prompt.h"
    VERBATIM
  )

  string(REPLACE "-" "_" cmake_target "prompt_${name}")
  add_custom_target("${cmake_target}" DEPENDS "${output}")
endfunction()

if(ZIG_EXECUTABLE)
  add_prompt_cross_target("x64-linux-gnu" "x86_64-linux-gnu")
  add_prompt_cross_target("arm64-linux-gnu" "aarch64-linux-gnu")
  add_prompt_cross_target("x64-linux-musl" "x86_64-linux-musl" STATIC)
  add_prompt_cross_target("arm64-linux-musl" "aarch64-linux-musl" STATIC)
  add_prompt_cross_target("x64-macos" "x86_64-macos")
  add_prompt_cross_target("arm64-macos" "aarch64-macos")

  add_custom_target(
    prompt_linux
    DEPENDS
      prompt_x64_linux_gnu
      prompt_arm64_linux_gnu
      prompt_x64_linux_musl
      prompt_arm64_linux_musl
  )

  add_custom_target(
    prompt_macos
    DEPENDS
      prompt_x64_macos
      prompt_arm64_macos
  )

  add_custom_target(prompt_cross DEPENDS prompt_linux prompt_macos)
else()
  message(STATUS "zig not found; cross targets will be skipped")

  add_custom_target(
    prompt_linux
    COMMAND ${CMAKE_COMMAND} -E echo "zig not found; skipping prompt_linux"
    VERBATIM
  )

  add_custom_target(
    prompt_macos
    COMMAND ${CMAKE_COMMAND} -E echo "zig not found; skipping prompt_macos"
    VERBATIM
  )

  add_custom_target(prompt_cross DEPENDS prompt_linux prompt_macos)
endif()
