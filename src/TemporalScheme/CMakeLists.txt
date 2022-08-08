add_library(TemporalScheme STATIC
            RungeKutta_impl.cpp
	    )
target_include_directories(TemporalScheme
			   PUBLIC
			   "${CMAKE_CURRENT_SOURCE_DIR}"
			   "${CMAKE_CURRENT_BINARY_DIR}"
			   )
target_link_libraries(TemporalScheme PUBLIC Field Config)
add_sycl_to_target(TARGET TemporalScheme)

