# CMakeList.txt: CompilerTheory 的 CMake 项目
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("CompilerTheory")

# ============================================================
#  选项
# ============================================================
option(ENABLE_GUI "Build with Qt6 GUI" ON)

# ============================================================
#  核心编译器源码（GUI 和 CLI 共用）
# ============================================================
set(CORE_SOURCE
	"include/token.h"
	"include/ast.h"
	"include/symtab.h"
	"include/quadruple.h"
	"include/lexer.h"
	"src/lexer.cpp"
	"include/parser.h"
	"src/parser.cpp"
	"include/semantic.h"
	"src/semantic.cpp"
	"include/irgen.h"
	"src/irgen.cpp"
	"src/symtab.cpp"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================
#  GUI 构建
# ============================================================
# CLI 入口
set(CLI_MAIN CompilerTheory.cpp CompilerTheory.h)
if(ENABLE_GUI)
	message(STATUS "Building with Qt6 GUI support")
	set(CMAKE_AUTOUIC ON)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)

	find_package(Qt6 REQUIRED COMPONENTS Widgets Concurrent)

	set(GUI_SOURCE
		"src/GUI/main.cpp"
		"src/GUI/core/compilermodel.h"
		"src/GUI/core/compilermodel.cpp"
		"src/GUI/core/compilerview.h"
		"src/GUI/core/compilerview.cpp"
		"src/GUI/core/compilercontroller.h"
		"src/GUI/core/compilercontroller.cpp"
		"src/GUI/core/mainwindow.h"
		"src/GUI/core/mainwindow.cpp"
		"src/GUI/widgets/welcomepage.h"
		"src/GUI/widgets/welcomepage.cpp"
		"src/GUI/widgets/sidepanel.h"
		"src/GUI/widgets/sidepanel.cpp"
			"src/GUI/widgets/asttreedelegate.h"
			"src/GUI/widgets/asttreedelegate.cpp"
		"src/GUI/widgets/tokencardwidget.h"
		"src/GUI/widgets/tokencardwidget.cpp"
		"src/GUI/resources/resources.qrc"
	)

	add_executable(${PROJECT_NAME} ${GUI_SOURCE} ${CORE_SOURCE})
	target_include_directories(${PROJECT_NAME} PRIVATE include src/GUI src/GUI/core src/GUI/widgets)
	target_link_libraries(${PROJECT_NAME} Qt6::Widgets Qt6::Concurrent)

else()
	# ============================================================
	#  纯 CLI 构建（不做 Qt）
	# ============================================================
	add_executable(${PROJECT_NAME} ${CORE_SOURCE} ${CLI_MAIN})
	target_include_directories(${PROJECT_NAME} PRIVATE include)
endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()
