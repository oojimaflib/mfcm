add_library(DataArray STATIC DataArray_impl.cpp)
target_include_directories(DataArray
			   PUBLIC
			   "${CMAKE_CURRENT_SOURCE_DIR}/.."
			   ${CMAKE_CURRENT_SOURCE_DIR}
			   )
add_sycl_to_target(TARGET DataArray)

			   
