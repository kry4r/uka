set(LIB_NAME src)
file(GLOB SOURCES
*.cpp
*.hpp)
add_library(${LIB_NAME} STATIC ${SOURCES})