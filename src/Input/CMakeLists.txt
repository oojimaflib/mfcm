add_library(Input STATIC
            FieldGenerator_impl.cpp
	    TimeSeries_impl.cpp
	    )
target_include_directories(Input
			   PUBLIC
			   "${CMAKE_CURRENT_SOURCE_DIR}"
			   "${CMAKE_CURRENT_SOURCE_DIR}/Output"
			   "${CMAKE_CURRENT_BINARY_DIR}"
			   )
target_link_libraries(Input PUBLIC Config Field Raster)
add_sycl_to_target(TARGET Input)

