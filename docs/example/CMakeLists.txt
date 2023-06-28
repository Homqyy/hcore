cmake_minimum_required(VERSION 3.0.0)

project(example VERSION 0.1.0 LANGUAGES C)

# Get all .c files in the directory
file(GLOB EXAMPLE_C_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Traverse each .c file and generate the corresponding executable file
foreach(source_file ${EXAMPLE_C_FILES})
    # Get the base name of the file (without path and extension)
    get_filename_component(file_base_name ${source_file} NAME_WE)

    # Generate an executable program for each .c file
    add_executable(${file_base_name} ${source_file})

    target_link_libraries(${file_base_name} PRIVATE hcore)

    # Set the include directory of the executable program
    target_include_directories(${file_base_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

    # Set the output directory of the executable program
    set_target_properties(${file_base_name} PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    
    # Set the postfix of the executable program
    set_target_properties(${file_base_name} PROPERTIES 
        DEBUG_POSTFIX "_d")
endforeach()