cmake_minimum_required(VERSION 3.7)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(NAME SdEngine)

project(${NAME} VERSION 0.0.1)

# 1. Find Vulkan
find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")

# 2. Set GLFW & GLM path
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)

# 3. Create executable
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

if (WIN32)
	message(STATUS "Creating build for Windows")
	target_include_directories(${PROJECT_NAME} PUBLIC
		${PROJECT_SOURCE_DIR}/src
		${PROJECT_SOURCE_DIR}/external/vma/include
		${Vulkan_INCLUDE_DIRS}
    )
	
	target_link_directories(${PROJECT_NAME} PUBLIC
		${Vulkan_LIBRARIES}
	)
	
	# Link with GLFW
	target_link_libraries(${PROJECT_NAME} glfw Vulkan::Vulkan)
	
	# Add GLM
	target_link_libraries(${PROJECT_NAME} glm::glm)
	
endif()

# 4. Set the output path for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


#======================COMPILE SHADERS======================#

# 1. Find shader files
find_program(GLSL_VALIDATOR glslangValidator)
file(GLOB_RECURSE GLSL_SOURCE_FILES
  "${PROJECT_SOURCE_DIR}/shaders/*.frag"
  "${PROJECT_SOURCE_DIR}/shaders/*.vert"
)

# 2. Compile
foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)
add_dependencies(${PROJECT_NAME} Shaders)