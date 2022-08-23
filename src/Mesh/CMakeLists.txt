add_library(Mesh STATIC
            Mesh_impl.cpp
	    )
target_link_libraries(Mesh PUBLIC DataArray Config Geometry)
target_include_directories(Mesh
			   INTERFACE
			   "${CMAKE_CURRENT_SOURCE_DIR}"
			   DataArray
			   Config
			   Geometry
			   )
add_sycl_to_target(TARGET Mesh)

			   
