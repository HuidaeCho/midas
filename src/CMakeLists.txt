cmake_minimum_required(VERSION 3.16)

# XXX: undocumented https://stackoverflow.com/a/10306476/16079666
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(MIDAS)

set(CMAKE_SKIP_RPATH TRUE)

# for tail recursion
if(MSVC)
  add_compile_options(/O2)
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
  write_lfp.c)
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
