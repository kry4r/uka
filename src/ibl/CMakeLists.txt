set(LIB_NAME ibl)
file(GLOB SOURCES
*.hpp
*.cpp)
add_library(${LIB_NAME} STATIC ${SOURCES})