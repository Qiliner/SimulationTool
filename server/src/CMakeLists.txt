# CMakeList.txt: server的 CMake 项目，在此处包括源代码并定义

# 项目特定的逻辑。

#设置输出目录

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

# 将源代码添加到此项目的可执行文件。

add_executable (server main.cpp ModelDesignerServer.cpp Database.cpp ZipUtils.cpp TaskObjectManager.cpp)

#boost库
if(NOT DEFINED BOOST_ROOT)
    set(BOOST_ROOT "D:/lin/DevTool/boost_1_80_0")
endif()
find_package(Boost REQUIRED COMPONENTS system filesystem)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(server ${Boost_LIBRARIES})
endif()

find_package(Boost REQUIRED COMPONENTS filesystem)

target_link_libraries(server Boost::filesystem sqlite3)

target_link_libraries(server
    "$<$<CONFIG:Debug>:TaskServerd>"
    "$<$<CONFIG:Release>:TaskServer>"
)
if(MSVC)
    target_link_directories(server PRIVATE "../external/lib/win64-msvc")
    target_compile_options(server PRIVATE /bigobj /utf-8)
    add_definitions(-D_UNICODE -DUNICODE)
    add_compile_options(/std:c++17)
endif()

#添加头文件

include_directories(
  "../external/include" 
"../external/include/cpp-httplib" 
"../external/include/rapidjson" 
"../external/include/rapidxml"
"../external/include/sqlite3"
"../external/include/websocketpp"
"../external/include/TaskServer")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET server PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。

#拷贝库至输出目录

file(COPY "../external/lib/win64-msvc/sqlite3.dll" DESTINATION ${CMAKE_SOURCE_DIR}/output${CMAKE_BUILD_TYPE})
file(COPY "../external/lib/win64-msvc/TaskServer.dll" DESTINATION ${CMAKE_SOURCE_DIR}/output${CMAKE_BUILD_TYPE})
file(COPY "../external/lib/win64-msvc/TaskServerd.dll" DESTINATION ${CMAKE_SOURCE_DIR}/output/${CMAKE_BUILD_TYPE})
#拷贝boost库至输出目录
file(COPY ${BOOST_ROOT}/lib64-msvc-14.2/boost_filesystem-vc142-mt-x64-1_80.dll DESTINATION ${CMAKE_SOURCE_DIR}/output/${CMAKE_BUILD_TYPE})
