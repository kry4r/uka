set(LIB_NAME src)
file(GLOB SOURCES
*.cpp
*.hpp)
find_package(Vulkan REQUIRED)
add_library(${LIB_NAME} STATIC ${SOURCES})
target_link_libraries(${LIB_NAME} Vulkan::Vulkan ktx glm tinygltf)