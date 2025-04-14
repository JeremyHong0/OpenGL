cmake_minimum_required(VERSION 3.7)
project(HGraphics)

set(CMAKE_CXX_STANDARD 17)

# sources
file(GLOB VENDORS_SOURCES 3rd-party/glad/src/glad.c)

file(GLOB SRC_FILES
        src/*.cpp
        )

file(GLOB HEADER_FILES
        src/include/*.h
        )

file(GLOB IMGUI_FILES
        3rd-party/imgui/*.cpp
        3rd-party/imgui/*.h
        )

# source groups
source_group("Sources" FILES ${SRC_FILES})
source_group("Headers" FILES ${HEADER_FILES})
source_group("Vendors" FILES ${VENDORS_SOURCES})
source_group("Imgui" FILES ${IMGUI_FILES})

# executable
add_executable(
  ${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES} ${VENDORS_SOURCES} ${IMGUI_FILES})

# link libraries
target_link_directories(${PROJECT_NAME} PUBLIC lib)
target_link_libraries(${PROJECT_NAME} glfw3 assimp-vc143-mtd ${GLFW_LIBRARIES} ${GLAD_LIBRARIES} )
target_compile_definitions(${PROJECT_NAME} PRIVATE -DGLFW_INCLUDE_NONE
                           -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE src/include
          3rd-party/glad/include/
          3rd-party/glfw/include/
          3rd-party/glm/
          3rd-party/stb/
          3rd-party/assimp/include/
          3rd-party/imgui/)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HGraphics)


add_custom_command(
                TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/lib/assimp-vc143-mtd.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/build_MSVC
                )