# CMakeList.txt: template 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.17)
set(EXECUTABLE_NAME FbxParser)#项目名称
# 将源代码添加到此项目的可执行文件。
func_add_code_path_to_executable(${EXECUTABLE_NAME} "${CMAKE_CURRENT_SOURCE_DIR}" "shared")#添加代码文件夹
set_target_properties(${EXECUTABLE_NAME} PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES)#采用c++20
add_definitions("-D _CRT_SECURE_NO_WARNINGS")#添加宏定义
add_definitions("-DPROJECT_VERSION=\"${PROJECT_VERSION}\"")#添加宏定义
if(MSVC)
	mac_add_link_options(/DEBUG /INCREMENTAL:NO /OPT:ICF)#添加编译命令
endif()
if(MSVC)
	func_add_3rdparty_inc_libs(${EXECUTABLE_NAME} "FBXSDK/include" "FBXSDK/lib/vs2022/x64/debug" "FBXSDK/lib/vs2022/x64/release" "libfbxsdk;libxml2-md;zlib-md")
else()
	func_add_3rdparty_inc_libs(${EXECUTABLE_NAME} "FBXSDK/include_linux" "FBXSDK/lib/gcc/x64/debug" "FBXSDK/lib/gcc/x64/release" "libfbxsdk;")
	func_add_3rdparty_inc_libs(${EXECUTABLE_NAME} "libxml2/include" "libxml2/bin/linux64/Release" "libxml2/bin/linux64/Release")
	target_link_libraries(${EXECUTABLE_NAME} PUBLIC stdc++fs dl pthread)
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Shell/fbx_lod.sh" "${BIN_OUTPUT_PATH}/fbx_lod.sh" @ONLY)
endif()
