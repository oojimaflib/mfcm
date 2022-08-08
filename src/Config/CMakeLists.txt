find_package(Boost 1.78 REQUIRED
	     COMPONENTS
	     filesystem
	     program_options
	     )

add_library(Config STATIC Config_impl.cpp TimeParser_impl.cpp TimeParameters_impl.cpp)
target_include_directories(Config
			   INTERFACE
			   ${CMAKE_CURRENT_SOURCE_DIR}
			   )
target_link_libraries(Config
                      Boost::filesystem
		      Boost::program_options
                      )

			   
