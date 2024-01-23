# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.17)
project ("FbxParser" LANGUAGES CXX VERSION 1.0.0)

# START:初始化OBV-CMAKE环境
macro (mac_obv_cmake project_type)
	if(NOT DEFINED OBV_CMAKE_URL)
		set(OBV_CMAKE_URL "https://gitee.com/buer021/obv-cmake/raw/master/src/ObvCmakeFunctions.cmake")
	endif()
	set(OBV_CMAKE_FILE "${CMAKE_BINARY_DIR}/../ObvCmakeFunctions.cmake")
	if (NOT EXISTS "${OBV_CMAKE_FILE}")
		file(DOWNLOAD ${OBV_CMAKE_URL} "${OBV_CMAKE_FILE}")
	endif()
	include("${OBV_CMAKE_FILE}")
	mac_init_obv_cmake(${CMAKE_CURRENT_SOURCE_DIR} "${project_type}")
endmacro()
mac_obv_cmake("dependencies")
# END:初始化OBV-CMAKE环境

# 包含子项目。
add_subdirectory ("src")#添加一个项目文件夹到project