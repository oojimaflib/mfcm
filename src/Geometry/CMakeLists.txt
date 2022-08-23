find_package(GDAL)
configure_file(GDAL_config.hpp.in GDAL_config.hpp)

add_library(Geometry STATIC
            Geometry_impl.cpp
	    )
target_include_directories(Geometry
			   PUBLIC
			   "${CMAKE_CURRENT_SOURCE_DIR}"
			   "${CMAKE_CURRENT_BINARY_DIR}"
			   )
target_link_libraries(Geometry PUBLIC Config DataArray)
add_sycl_to_target(TARGET Geometry)

			   
