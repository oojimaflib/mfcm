add_library(SpatialDerivative STATIC
            SpatialDerivative_impl.cpp
	    )
target_include_directories(SpatialDerivative
			   PUBLIC
			   "${CMAKE_CURRENT_SOURCE_DIR}"
			   "${CMAKE_CURRENT_BINARY_DIR}"
			   )
target_link_libraries(SpatialDerivative PUBLIC Field Mesh)
add_sycl_to_target(TARGET SpatialDerivative)

