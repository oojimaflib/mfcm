find_package(GDAL)
configure_file(GDAL_config.hpp.in GDAL_config.hpp)

add_library(Raster STATIC
            Raster_impl.cpp
	    RasterFormat_impl.cpp
	    )
target_include_directories(Raster
			   PUBLIC
			   "${CMAKE_CURRENT_SOURCE_DIR}"
			   "${CMAKE_CURRENT_BINARY_DIR}"
			   )
target_link_libraries(Raster PUBLIC Config Field)
add_sycl_to_target(TARGET Raster)

			   
