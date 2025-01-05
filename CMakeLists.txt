cmake_minimum_required(VERSION 3.16)

# 设置项目名称
project(MMDLP_GUI)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt 安装路径
set(CMAKE_PREFIX_PATH "D:/opencv/build/x64/vc16/lib")
find_package(OpenCV REQUIRED)

# 查找 Qt 包
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(BOOST_ROOT "C:/src/vcpkg/installed/x64-windows")
set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
find_package(Boost REQUIRED COMPONENTS asio system)

# 定义库路径
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hardware/innfos_sdk/include)
set(LIB_DIR_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/hardware/innfos_sdk/lib/release)
set(LIB_DIR_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/hardware/innfos_sdk/lib/debug)
set(PROJECTOR_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hardware/projector_api/include)
set(PROJECTOR_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hardware/projector_api/lib)

# 包含头文件路径
include_directories(${INCLUDE_DIR})
include_directories(${PROJECTOR_INCLUDE_DIR})

# 链接库文件路径
link_directories(${LIB_DIR_RELEASE})
link_directories(${LIB_DIR_DEBUG})

# 添加包含路径
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/hardware
    ${CMAKE_CURRENT_SOURCE_DIR}/interface
)


# 查找源文件
file(GLOB SOURCES
    "main.cpp"
    "hardware/*.cpp"
    "interface/*.cpp"
)

file(GLOB HEADERS
    "hardware/*.h"
    "interface/*.h"
)

file(GLOB UI_FILES
    "interface/*.ui"
)

# 自动处理 Qt 文件
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 设置运行时输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加可执行文件
add_executable(MMDLP_GUI ${SOURCES} ${HEADERS})

# 设置编译选项
add_compile_options(-pthread -DASIO_STANDALONE -DASIO_HAS_STD_CHRONO)

# 链接 Qt 和自定义库
if(WIN32)
    target_link_libraries(MMDLP_GUI
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        debug actuatorControllerd optimized actuatorController
        ${PROJECTOR_LIB_DIR}/LibUSB3DPrinter-x64.lib
    )
    # 添加库文件到目标文件夹
    add_custom_command(TARGET MMDLP_GUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIB_DIR_RELEASE}/actuatorController.dll"
        "${LIB_DIR_DEBUG}/actuatorControllerd.dll"
        $<TARGET_FILE_DIR:MMDLP_GUI>
    )
    add_custom_command(TARGET MMDLP_GUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECTOR_LIB_DIR}/LibUSB3DPrinter-x64.dll"
    $<TARGET_FILE_DIR:MMDLP_GUI>
)
elseif(UNIX)
    target_link_libraries(MMDLP_GUI
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        ActuatorController
        LibUSB3DPrinter-x64
        pthread
    )
endif()


if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    target_link_libraries(MMDLP_GUI ${Boost_LIBRARIES})
endif()

target_link_libraries(MMDLP_GUI ${OpenCV_LIBS})

target_include_directories(MMDLP_GUI PRIVATE ${OpenCV_INCLUDE_DIRS})