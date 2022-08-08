add_library(Field STATIC
            Field_impl.cpp
	    FieldOperators_impl.cpp
	    )
target_link_libraries(Field PUBLIC DataArray Mesh)
target_include_directories(Field
			   INTERFACE
			   "${CMAKE_CURRENT_SOURCE_DIR}"
			   DataArray
			   Mesh
			   )
add_sycl_to_target(TARGET Field)

			   
