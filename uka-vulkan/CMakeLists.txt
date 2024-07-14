set(LIB_NAME uka_vulkan)

file(GLOB SOURCES
*.hpp
*.cpp)
add_library(${LIB_NAME} STATIC ${SOURCES})
set(Vulkan_LIBRARY /usr/lib/x86_64-linux-gnu/libvulkan.so)
find_package(Vulkan REQUIRED)
target_link_libraries(${LIB_NAME} Vulkan::Vulkan ktx glm tinygltf)
