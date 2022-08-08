add_library(SaintVenant STATIC
            Solver_impl.cpp
	    )
target_include_directories(SaintVenant
			   PUBLIC
			   "${CMAKE_CURRENT_SOURCE_DIR}"
			   "${CMAKE_CURRENT_BINARY_DIR}"
			   )
target_link_libraries(SaintVenant PUBLIC Config Mesh Input Field SpatialDerivative)
add_sycl_to_target(TARGET SaintVenant)

