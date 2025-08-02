cmake_minimum_required(VERSION 3.16)

# XXX: undocumented https://stackoverflow.com/a/10306476/16079666
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(MIDAS C)

set(CMAKE_SKIP_RPATH TRUE)

# for tail recursion
if(MSVC)
  add_compile_options(/O2)
  set(MSVC_SRC gettimeofday.c)
else()
  add_compile_options(-O3)
endif()

if(MSVC AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.30")
  # for task avoid error C7660: 'task': requires '-openmp:llvm' command line
  # option(s)
  set(OpenMP_RUNTIME_MSVC "llvm")
endif()

add_library(
  midas SHARED
  midas.c
  timeval_diff.c
  raster.c
  recode.c
  accumulate.c
  accumulate_lessmem.c
  accumulate_moremem.c
  outlet_list.c
  outlets.c
  delineate.c
  delineate_lessmem.c
  delineate_moremem.c
  shed_hier.c
  point_list.c
  lfp.c
  lfp_lessmem.c
  lfp_moremem.c
  write_lfp.c
  ${MSVC_SRC})

if(WIN32)
  target_compile_definitions(midas PRIVATE EXPORTS)
endif()

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
  target_link_libraries(midas PUBLIC ${MATH_LIBRARY})
endif()
find_package(OpenMP REQUIRED)
if(OpenMP_C_FOUND)
  target_link_libraries(midas PUBLIC OpenMP::OpenMP_C)
  if(MSVC AND CMAKE_VERSION VERSION_LESS "3.30")
    # CMake < 3.30 doesn't support OpenMP_RUNTIME_MSVC for task
    add_compile_options(-openmp:llvm)
  endif()
endif()
find_package(GDAL REQUIRED)
if(GDAL_FOUND)
  target_link_libraries(midas PUBLIC ${GDAL_LIBRARY})
  target_include_directories(midas PUBLIC ${GDAL_INCLUDE_DIR})
endif()

foreach(NAME mefa meshed melfp)
  add_executable(${NAME} ${NAME}.c)
  target_link_libraries(${NAME} PUBLIC midas)
endforeach()

set(DIST_DIR ${CMAKE_BINARY_DIR}/dist)

if(WIN32)
  add_custom_target(
    copy_dist ALL
    DEPENDS midas mefa meshed melfp
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/include
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:midas>
      $<TARGET_FILE:mefa> $<TARGET_FILE:meshed> $<TARGET_FILE:melfp>
      $<TARGET_LINKER_FILE:midas> ${DIST_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/midas.h ${DIST_DIR}/include
    COMMENT "Copying DLL, executables, LIB, and header to dist/")
else()
  add_custom_target(
    copy_dist ALL
    DEPENDS midas mefa meshed melfp
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}/include
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:mefa>
            $<TARGET_FILE:meshed> $<TARGET_FILE:melfp> ${DIST_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:midas>
            ${DIST_DIR}/lib
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/midas.h ${DIST_DIR}/include
    COMMENT "Copying DLL, executables, LIB, and header to dist/")
endif()
