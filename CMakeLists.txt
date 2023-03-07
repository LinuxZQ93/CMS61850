cmake_minimum_required(VERSION 3.7)
project(CMS61850)
MESSAGE(STATUS "project is " ${CMAKE_PROJECT_NAME})
SET(CMAKE_VERBOSE_MAKEFILE  ON)
#file(GLOB src_dir "./*.c")
SET(CMAKE_SYSTEM_NAME Linux)
#option(platform "platform option" arm)
IF(platform MATCHES "arm")
MESSAGE(STATUS "arm platform build")
SET(CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc")
SET(CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++")
ELSEIF(platform MATCHES "x86")
MESSAGE(STATUS "x86 platform build")
SET(CMAKE_C_COMPILER "gcc")
SET(CMAKE_CXX_COMPILER "g++")
ELSE()
MESSAGE(STATUS "aarch64 platform build")
SET(platform "aarch64")
SET(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
SET(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")
ENDIF()
include_directories("./" "./Include" "${PROJECT_SOURCE_DIR}/Include/BaseInc" "${PROJECT_SOURCE_DIR}/Include/ThirdInc" "${PROJECT_SOURCE_DIR}/Include/ThirdInc/asn1")
link_directories("${PROJECT_SOURCE_DIR}/Lib/ThirdLib/${platform}" "./")
IF(CMAKE_BUILD_TYPE MATCHES "Debug")
    MESSAGE(STATUS "${CMAKE_PROJECT_NAME} Build Debug Program")
    link_directories("${PROJECT_SOURCE_DIR}/Lib/BaseLib/${platform}/Debug"
            "${PROJECT_SOURCE_DIR}/Lib/UtilsLib/${platform}/Debug")
ELSE()
    MESSAGE(STATUS "${CMAKE_PROJECT_NAME} Build Release Program")
    link_directories("${PROJECT_SOURCE_DIR}/Lib/BaseLib/${platform}/Release")
ENDIF()
execute_process(COMMAND rm -rf ${PROJECT_SOURCE_DIR}/out)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out/${platform}/${CMAKE_BUILD_TYPE})
SET(CMAKE_CXX_FLAGS_DEBUG "-g -rdynamic")
SET(CMAKE_CXX_FLAGS_RELEASE "-Os")
SET(SRC_LIST
    ./CMS61850.cpp
    ./SCL/SCLParse.cpp
    ./Service/ServiceManager.cpp
    ./Service/IService.cpp
    ./Common.cpp
    ./APER.cpp
    ./Service/Associate/Associate.cpp
    ./Service/Node/ServerNode.cpp
    ./Service/Data/ServerData.cpp
    ./Service/Data/DataSet.cpp
    ./Service/Report/Report.cpp
    ./Service/Setting/Setting.cpp
    )
#execute_process(COMMAND mkdir ./bin)
execute_process(
	COMMAND	bash "-c" "git rev-list HEAD -n 1 | cut -c 1-7"
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
MESSAGE(STATUS "version is ${GIT_VERSION}")
add_definitions(-std=c++11 -Wall -DLibVersion=\"${GIT_VERSION}\" -DLibName=\"${CMAKE_PROJECT_NAME}\" -DTIXML_USE_STL)
#set(LIBRARY_OUTPUT_PATH ./bin)
#add_library(show SHARED ${SRC_LIST})

#PROJECT(HELLO)
#SET(SRC_LIST main.c)
#MESSAGE(STATUS "This is BINARY dir " ${HELLO_BINARY_DIR})
#MESSAGE(STATUS "This is SOURCE dir "${HELLO_SOURCE_DIR})
IF(lib MATCHES "shared")
MESSAGE(STATUS "build shared lib")
ADD_LIBRARY(${CMAKE_PROJECT_NAME} SHARED ${SRC_LIST})
ELSEIF(lib MATCHES "static")
MESSAGE(STATUS "build static lib")
ADD_LIBRARY(${CMAKE_PROJECT_NAME} STATIC ${SRC_LIST})
ELSE()
MESSAGE(STATUS "build shared lib")
ADD_LIBRARY(${CMAKE_PROJECT_NAME} SHARED ${SRC_LIST})
ENDIF()
target_link_libraries(${CMAKE_PROJECT_NAME} "dl" "pthread" "asn1" "tinyxml" "gmssl")
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND cp ${PROJECT_SOURCE_DIR}/Lib/ThirdLib/${platform}/* ${PROJECT_SOURCE_DIR}/Run/Lib
    COMMAND cp ${PROJECT_SOURCE_DIR}/Run/${platform}/lib* ${PROJECT_SOURCE_DIR}/Run/Lib
    COMMAND cp ${PROJECT_SOURCE_DIR}/Run/${platform}/Main ${PROJECT_SOURCE_DIR}/Run/
    COMMAND cp ${PROJECT_SOURCE_DIR}/out/${platform}/${CMAKE_BUILD_TYPE}/* ${PROJECT_SOURCE_DIR}/Run/Lib
)
