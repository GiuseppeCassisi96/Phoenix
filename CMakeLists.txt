cmake_minimum_required(VERSION 3.5.0)
project(Phoenix VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE MINERVA_SOURCES ${PROJECT_SOURCE_DIR}/src/Minerva/*.cpp)
file(GLOB_RECURSE MINERVA_HEADERS ${PROJECT_SOURCE_DIR}/src/Minerva/*.h)
add_executable(Phoenix ${SOURCES} ${MINERVA_SOURCES} ${MINERVA_HEADERS})

include(envPhoenix.cmake)

#Here I set the compiler. I will use g++ compiler for .cpp files 
message(STATUS "using ${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH) 
      message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW "True")
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER  ${MINGW_PATH}/bin/g++.exe)
endif()


#Here I set the Vulkan environment. I set the include and lib path
if(DEFINED VULKAN_PATH)
    set(VULKAN_INCLUDE_DIRS "${VULKAN_PATH}/Include") 
    set(VULKAN_LIBRARIES "${VULKAN_PATH}/Lib") 
    set(VULKAN_FOUND "True")

endif()

#If the Vulkan path isn't defined I throw an error 
if (NOT VULKAN_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using Vulkan include at: ${VULKAN_INCLUDE_DIRS}")
    message(STATUS "Using Vulkan lib at: ${VULKAN_LIBRARIES}")
endif()


#Here I set the GLFW environment. I set the include and lib path
if(DEFINED GLFW_PATH)
    set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include") 
    set(GLFW_LIBRARIES "${GLFW_PATH}/lib-vc2022") 
    set(GLFW_FOUND "True")

endif()

#If the GLFW path isn't defined I throw an error 
if (NOT GLFW_FOUND)
    message(FATAL_ERROR "Could not find GLFW library!")
else()
    message(STATUS "Using GLFW include at: ${GLFW_INCLUDE_DIRS}")
    message(STATUS "Using GLFW lib at: ${GLFW_LIBRARIES}")
endif()

#Here I set the glm environment. I set the include path
if(DEFINED GLM_PATH)
    set(GLM_INCLUDE_DIRS "${GLM_PATH}") 
    set(GLM_FOUND "True")

endif()

#If the glm path isn't defined I throw an error 
if (NOT GLM_FOUND)
    message(FATAL_ERROR "Could not find GLM library!")
else()
    message(STATUS "Using glm include at: ${GLM_INCLUDE_DIRS}")
endif()

#Here I set the stb environment. I set the include path
if(DEFINED STB_PATH)
    set(STB_INCLUDE_DIRS "${STB_PATH}") 
    set(STB_FOUND "True")

endif()

#If the stb path isn't defined I throw an error 
if (NOT STB_FOUND)
    message(FATAL_ERROR "Could not find stb library!")
else()
    message(STATUS "Using stb include at: ${STB_INCLUDE_DIRS}")
endif()


#Here I set the tiny_obj environment. I set the include path
if(DEFINED TINY_OBJ_PATH)
    set(TINY_OBJ_INCLUDE_DIRS "${TINY_OBJ_PATH}") 
    set(TINY_OBJ_FOUND "True")

endif()

#If the tiny_obj path isn't defined I throw an error 
if (NOT TINY_OBJ_FOUND)
    message(FATAL_ERROR "Could not find tiny_obj library!")
else()
    message(STATUS "Using tiny_obj include at: ${TINY_OBJ_INCLUDE_DIRS}")
endif()

if(WIN32)
    if (USE_MINGW)
    target_include_directories(${PROJECT_NAME} PUBLIC
    ${MINGW_PATH}/include
    )
    target_link_directories(${PROJECT_NAME} PUBLIC
    ${MINGW_PATH}/lib
    )
    endif()

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_SOURCE_DIR}/src
        ${VULKAN_INCLUDE_DIRS}
        ${GLFW_INCLUDE_DIRS}
        ${GLM_PATH}
        ${STB_INCLUDE_DIRS}
        ${TINY_OBJ_INCLUDE_DIRS}
        )
    
    target_link_directories(${PROJECT_NAME} PUBLIC
        ${VULKAN_LIBRARIES}
        ${GLFW_LIBRARIES}
    )

    target_link_libraries(${PROJECT_NAME} glfw3 vulkan-1)
endif()