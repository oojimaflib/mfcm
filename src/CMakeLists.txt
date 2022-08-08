cmake_minimum_required(VERSION 3.10)

find_package(hipSYCL REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(mfcm VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(mfcm_config.hpp.in mfcm_config.hpp)

add_subdirectory(Config)
add_subdirectory(DataArray)
add_subdirectory(Field)
add_subdirectory(Geometry)
add_subdirectory(Input)
add_subdirectory(Mesh)
add_subdirectory(Raster)
add_subdirectory(SaintVenant)
add_subdirectory(SpatialDerivative)
add_subdirectory(TemporalScheme)

add_executable(mfcm
               mfcm.cpp sycl.cpp
	       )

target_link_libraries(mfcm PUBLIC
                      Config
		      DataArray
		      Field
		      Geometry
		      Input
		      Mesh
		      Raster
		      SaintVenant
		      SpatialDerivative
		      TemporalScheme
		      )
target_include_directories(mfcm PUBLIC
			   "${PROJECT_BINARY_DIR}"
			   )
add_sycl_to_target(TARGET mfcm)

