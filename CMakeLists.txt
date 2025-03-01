cmake_minimum_required(VERSION 3.0.0)
project(sample VERSION 0.1.0)

set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "If the test suite for Assimp is built in addition to the library." FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "Disable Assimp's export functionality." FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "If the supplementary tools for Assimp are built in addition to the library." FORCE)

add_definitions(-DASSIMP_BUILD_NO_X_IMPORTER 
                -DASSIMP_BUILD_NO_AMF_IMPORTER 
                -DASSIMP_BUILD_NO_3DS_IMPORTER
                -DASSIMP_BUILD_NO_MD3_IMPORTER
                -DASSIMP_BUILD_NO_MDL_IMPORTER
                -DASSIMP_BUILD_NO_MD2_IMPORTER
                -DASSIMP_BUILD_NO_PLY_IMPORTER
                -DASSIMP_BUILD_NO_ASE_IMPORTER
                -DASSIMP_BUILD_NO_OBJ_IMPORTER
                -DASSIMP_BUILD_NO_HMP_IMPORTER
                -DASSIMP_BUILD_NO_SMD_IMPORTER
                -DASSIMP_BUILD_NO_MDC_IMPORTER
                -DASSIMP_BUILD_NO_MD5_IMPORTER
                -DASSIMP_BUILD_NO_STL_IMPORTER
                -DASSIMP_BUILD_NO_LWO_IMPORTER
                -DASSIMP_BUILD_NO_DXF_IMPORTER
                -DASSIMP_BUILD_NO_NFF_IMPORTER
                -DASSIMP_BUILD_NO_RAW_IMPORTER
                -DASSIMP_BUILD_NO_SIB_IMPORTER
                -DASSIMP_BUILD_NO_OFF_IMPORTER
                -DASSIMP_BUILD_NO_AC_IMPORTER
                -DASSIMP_BUILD_NO_BVH_IMPORTER
                -DASSIMP_BUILD_NO_IRRMESH_IMPORTER
                -DASSIMP_BUILD_NO_IRR_IMPORTER
                -DASSIMP_BUILD_NO_Q3D_IMPORTER
                -DASSIMP_BUILD_NO_B3D_IMPORTER
                -DASSIMP_BUILD_NO_COLLADA_IMPORTER
                -DASSIMP_BUILD_NO_TERRAGEN_IMPORTER
                -DASSIMP_BUILD_NO_CSM_IMPORTER
                -DASSIMP_BUILD_NO_3D_IMPORTER
                -DASSIMP_BUILD_NO_LWS_IMPORTER
                -DASSIMP_BUILD_NO_OGRE_IMPORTER
                -DASSIMP_BUILD_NO_OPENGEX_IMPORTER
                -DASSIMP_BUILD_NO_MS3D_IMPORTER
                -DASSIMP_BUILD_NO_COB_IMPORTER
                -DASSIMP_BUILD_NO_Q3BSP_IMPORTER
                -DASSIMP_BUILD_NO_NDO_IMPORTER
                -DASSIMP_BUILD_NO_IFC_IMPORTER
                -DASSIMP_BUILD_NO_XGL_IMPORTER
                -DASSIMP_BUILD_NO_ASSBIN_IMPORTER
                -DASSIMP_BUILD_NO_C4D_IMPORTER
                -DASSIMP_BUILD_NO_3MF_IMPORTER
                -DASSIMP_BUILD_NO_X3D_IMPORTER
                -DASSIMP_BUILD_NO_MMD_IMPORTER
                -DASSIMP_BUILD_NO_M3D_IMPORTER
                -DASSIMP_BUILD_NO_STEP_IMPORTER
                -DASSIMP_BUILD_NO_BLEND_IMPORTER)

add_definitions(-DNO_ASSIMP)
add_subdirectory(thirdparty/glfw)
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/assimp)
add_subdirectory(thirdparty/glew/build/cmake)

set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui_impl_opengl3.cpp
)
add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/thirdparty/imgui)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)

include_directories(
    thirdparty/glfw/include 
    thirdparty/glm 
    thirdparty/glew/include 
    thirdparty/assimp/include
    ${CMAKE_SOURCE_DIR}/volpe
    ${OPENGL_INCLUDE_DIRS}
)

file(GLOB_RECURSE SAMPLEFW_SOURCE_FILES 
    ${CMAKE_SOURCE_DIR}/samplefw/*.c
    ${CMAKE_SOURCE_DIR}/samplefw/*.cpp)

file(GLOB_RECURSE SAMPLEFW_HEADER_FILES 
    ${CMAKE_SOURCE_DIR}/samplefw/*.h
    ${CMAKE_SOURCE_DIR}/samplefw/*.hpp)

file(GLOB_RECURSE VOLPE_SOURCE_FILES 
    ${CMAKE_SOURCE_DIR}/volpe/*.c
    ${CMAKE_SOURCE_DIR}/volpe/*.cpp)

file(GLOB_RECURSE VOLPE_HEADER_FILES 
    ${CMAKE_SOURCE_DIR}/volpe/*.h
    ${CMAKE_SOURCE_DIR}/volpe/*.hpp)

file(GLOB_RECURSE SOURCE_FILES 
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.cpp)
    
file(GLOB_RECURSE HEADER_FILES 
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.hpp)

add_executable(sample 
    ${SAMPLEFW_SOURCE_FILES}
    ${SAMPLEFW_HEADER_FILES}
    ${VOLPE_SOURCE_FILES}
    ${VOLPE_HEADER_FILES}
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

target_link_libraries(sample assimp glfw glew imgui ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
