cmake_minimum_required(VERSION 3.9)
project(OpenGL C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(IMGUI 		"Enable GUI" OFF)
option(ANDROID		"Build for android (automatically build with gradle)" OFF)
option(OPTIMIZE		"Optimize a lot" OFF)
option(MOLD		"Use mold" OFF)
# TODO: option(VENDOR 		"Vendor libraries" OFF)

# Misc
option(ADDRESS	 	"Use address sanitizer" OFF)
option(LEAK		"Use leak sanitizer" OFF)
option(UNDEFINED	"Use undefined sanitizer" OFF)
option(SANITIZE 	"Build with all sanitizer support if available" OFF)
option(CHECKS 		"Run checks" OFF)
option(IWYU 		"Run include what you use" OFF)
option(CLANG_TIDY 	"Run clang tidy" OFF)
option(CPPCHECK 	"Run cppcheck" OFF)

set(SRC
# Sources
src/main.cpp
src/game.cpp
src/utils.cpp

src/components/cameraComponent.cpp
src/components/component.cpp
src/components/drawComponent.cpp
src/components/meshComponent.cpp
src/components/modelComponent.cpp
src/components/movementComponent.cpp

src/opengl/cubemap.cpp
src/opengl/mesh.cpp
src/opengl/renderer.cpp
src/opengl/shader.cpp
src/opengl/texture.cpp
src/opengl/framebuffer.cpp
src/opengl/ubo.cpp

src/managers/glManager.cpp
src/managers/shaderManager.cpp
src/managers/textureManager.cpp
src/managers/eventManager.cpp
src/managers/assimpIO.cpp

src/actors/actor.cpp
src/actors/player.cpp
src/actors/world.cpp
src/actors/sun.cpp
src/actors/cube.cpp

src/ui/buttonComponent.cpp
src/ui/backgroundComponent.cpp
src/ui/controlUI.cpp
src/ui/mainMenu.cpp
src/ui/UIComponent.cpp
src/ui/UIScreen.cpp

# Headers
include/game.hpp
include/utils.hpp

include/components/cameraComponent.hpp
include/components/component.hpp
include/components/drawComponent.hpp
include/components/meshComponent.hpp
include/components/modelComponent.hpp
include/components/movementComponent.hpp

include/opengl/cubemap.hpp
include/opengl/mesh.hpp
include/opengl/renderer.hpp
include/opengl/shader.hpp
include/opengl/texture.hpp
include/opengl/types.hpp
include/opengl/framebuffer.hpp
include/opengl/ubo.hpp

include/managers/glManager.hpp
include/managers/shaderManager.hpp
include/managers/textureManager.hpp
include/managers/eventManager.hpp
include/managers/assimpIO.hpp

include/actors/actor.hpp
include/actors/player.hpp
include/actors/world.hpp
include/actors/sun.hpp
include/actors/cube.hpp

include/ui/buttonComponent.hpp
include/ui/backgroundComponent.hpp
include/ui/controlUI.hpp
include/ui/mainMenu.hpp
include/ui/UIComponent.hpp
include/ui/UIScreen.hpp

# External
src/third_party/stb_image.c
include/third_party/stb_image.h

# Glad
src/third_party/glad.c 

include/third_party/glad/glad.h 
include/third_party/KHR/khrplatform.h 
)

set(IMGUI_SRC
# ImGUI 
external/imgui/imgui.cpp
external/imgui/imgui_demo.cpp
external/imgui/imgui_draw.cpp
external/imgui/imgui_tables.cpp
external/imgui/imgui_widgets.cpp
external/imgui/backends/imgui_impl_sdl3.cpp
external/imgui/backends/imgui_impl_opengl3.cpp
)

set(ASSETS_EMBED_WEB
--preload-file=assets@assets
)

if(ANDROID STREQUAL ON) 
	message("Building android apk - experimental")
	message("Progress bar doesn't work, please wait till the end")
	message("If this doesn't work, go to android folder and run ./gradlew installDebug")
	# FIXME: No installRelease?

	set(NAMESPACE com.cyao.opengl)
	set(BUILD_NAME OpenGL)

	add_subdirectory(android)

	return()
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are:
		 Debug Release"
      	FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	message("Debug enabled")
	set(DEBUG TRUE)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Check platforms
if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	set(ANDROID TRUE)
	set(BUILD_NAME main) # Android needs this as libmain
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(MACOSX TRUE)
	set(BUILD_NAME ${PROJECT_NAME})

	message("Building for MacOS, to build universal package set \"-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64?\"")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS TRUE)
	set(BUILD_NAME ${PROJECT_NAME})
elseif(EMSCRIPTEN)
	set(WEB TRUE)
	set(BUILD_NAME ${PROJECT_NAME})

	set(CMAKE_EXECUTABLE_SUFFIX .js) # Making javascript from C++ :sunglasses:
	configure_file(cmake/index.html.in ${CMAKE_BINARY_DIR}/index.html)
else()
	set(BUILD_NAME ${PROJECT_NAME})
endif()

#
# 1. Include packages
#

# SDL
if(NOT ANDROID) # Android projects include themself using subdirectory
	if(WEB)
		find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-static)
	else()
		find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared) # TODO: static build option
	endif()

	if(NOT SDL3_SDL3_FOUND)
		message(FATAL_ERROR "Failed to find sdl3")
	endif()

	target_include_directories(${SDL3_SDL_INCLUDE_DIRS})
	target_link_libraries(${BUILD_NAME} PRIVATE SDL3::SDL3) # Static library?
endif()

# Assimp
if(ANDROID)
	# FIXME: Non-hardcoded lib
	add_library(assimp::assimp SHARED IMPORTED) # STATIC instad of SHARED for static
	set_target_properties(assimp::assimp PROPERTIES
	  IMPORTED_LOCATION "/usr/local/lib/libassimp.so"
	  INTERFACE_INCLUDE_DIRECTORIES "/usr/local/include"
	)

	target_link_libraries(${BUILD_NAME} PRIVATE assimp::assimp)
else()
	find_package(assimp REQUIRED CONFIG)

	if (NOT assimp_FOUND)
		message(FATAL_ERROR "Assimp not found")
	endif()

	link_directories(${ASSIMP_LIBRARY_DIR})
	include_directories(${ASSIMP_INCLUDE_DIRS})
	target_link_libraries(${BUILD_NAME} PRIVATE ${ASSIMP_LIBRARIES})
endif()

# Freetype
# TODO:
if (ANDROID)
	message("Building for Android")

	add_library(${BUILD_NAME} SHARED ${SRC})

	target_compile_options(${BUILD_NAME} PRIVATE -DGLES -DANDROID)

	target_link_libraries(${BUILD_NAME} PRIVATE SDL3::SDL3)

	# TODO: Freetype
elseif(MACOSX)
	message("Building for MacOS")
	find_package(OpenGL REQUIRED)

	if(DEBUG)
		add_executable(${BUILD_NAME} ${SRC})
	else()
		#Package into a application bundle
		add_executable(${BUILD_NAME} MACOSX_BUNDLE ${SRC})            
		set_target_properties(${BUILD_NAME} PROPERTIES
		    BUNDLE True
		    MACOSX_BUNDLE_ICON_FILE ${CMAKE_SOURCE_DIR}/cmake/${CMAKE_PROJECT_NAME}.ico
		    MACOSX_BUNDLE_GUI_IDENTIFIER org.cyao.${BUILD_NAME}
	   	    MACOSX_BUNDLE_BUNDLE_NAME ${BUILD_NAME}
		    MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
		    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
		    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/info.plist.in
		)
	
		file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/${BUILD_NAME}.app/Contents/Resources)

		target_link_options(${BUILD_NAME} PRIVATE -headerpad_max_install_names)

		file(COPY cmake/mac-patch-dylib.sh DESTINATION ${CMAKE_BINARY_DIR}) # Helper script
	endif()

	target_include_directories(${BUILD_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})
	target_include_directories(${BUILD_NAME} PRIVATE ${SDL_INCLUDE_DIRS})

	target_link_libraries(${BUILD_NAME} PRIVATE SDL3::SDL3)
	target_link_libraries(${BUILD_NAME} PRIVATE ${OPENGL_LIBRARIES})
elseif(WINDOWS)
	message("Building for windows")

	add_executable(${BUILD_NAME} WIN32 ${SRC})

	if(NOT MSVC)
		target_compile_options(${BUILD_NAME} PRIVATE -D__WINDOWS__)
	endif()

	target_link_libraries(${BUILD_NAME} PRIVATE SDL3::SDL3) # SDL3_image::SDL3_image)
else()
	message("Building for linux")

	add_executable(${BUILD_NAME} ${SRC})

	target_link_libraries(${BUILD_NAME} PRIVATE SDL3::SDL3) # SDL3_image::SDL3_image)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT ANDROID)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

# Strip debug symbols
if(NOT DEBUG AND NOT ANDROID AND NOT WEB)
	if (WINDOWS)
		add_custom_command(TARGET ${BUILD_NAME} POST_BUILD
			COMMAND ${CMAKE_STRIP} ${BUILD_NAME}.exe)
	elseif(MACOSX AND NOT DEBUG)
		add_custom_command(TARGET ${BUILD_NAME} POST_BUILD
			COMMAND ${CMAKE_STRIP} ${BUILD_NAME}.app/Contents/MacOS/${BUILD_NAME})
	else()
		add_custom_command(TARGET ${BUILD_NAME} POST_BUILD
			COMMAND ${CMAKE_STRIP} ${BUILD_NAME})
	endif()
endif()

# Set flags
if(MSVC)
	if(DEBUG)
		target_compile_options(${BUILD_NAME} PRIVATE /W4 /WX /MDd /Zi /Ob0 /Od /RTC1 /W3 /GR /EHsc)
		target_compile_definitions(${BUILD_NAME} PRIVATE /DWIN32 /D__WINDOWS__ /DDEBUG /D_DEBUG)
	else()
		target_compile_options(${BUILD_NAME} PRIVATE /MD /O2 /Ob2)
		target_compile_definitions(${BUILD_NAME} PRIVATE /D__WINDOWS__ /DWIND32 /DEIGEN_NO_DEBUG /DNDEBUG)
	endif()
else()
	if(DEBUG)
		target_compile_options(${BUILD_NAME} PRIVATE -Wall -Wextra -Werror -g -O0 -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wpointer-arith -Wwrite-strings -Wno-unknown-warning-option)
		target_compile_definitions(${BUILD_NAME} PRIVATE -DDEBUG -D_DEBUG)
	else()
		target_compile_options(${BUILD_NAME} PRIVATE -O3)
		target_compile_definitions(${BUILD_NAME} PRIVATE -DEIGEN_NO_DEBUG -DNDEBUG)
	endif()

	if (OPTIMIZE STREQUAL ON)
		target_compile_options(${BUILD_NAME} PRIVATE -march=native -Ofast -fopenmp -fno-math-errno)
	endif()
endif()

if(MOLD STREQUAL ON) 
	target_link_options(${BUILD_NAME} PRIVATE -fuse-ld=mold)
endif()

if(IMGUI STREQUAL ON AND CHECKS STREQUAL OFF)
	message("Enabling IMGUI")

	if(EXISTS external/imgui/backends)
		message(FATAL_ERROR "Please run git submodule update --remote --init external/imgui")
	endif()

	target_sources(${BUILD_NAME} PRIVATE ${IMGUI_SRC})
	include_directories(external/imgui)

	if(MSVC)
		target_compile_definitions(${BUILD_NAME} PRIVATE /DIMGUI)
	else()
		target_compile_definitions(${BUILD_NAME} PRIVATE -DIMGUI)
	endif()
endif()

if(WEB)
	target_compile_options(${BUILD_NAME} PRIVATE -sUSE_SDL=3 -DGLES -sNO_DISABLE_EXCEPTION_CATCHING)
	target_compile_options(${BUILD_NAME} PRIVATE ${ASSETS_EMBED_WEB})
	target_link_options(${BUILD_NAME} PRIVATE -sUSE_SDL=3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES2 -sFULL_ES3 -sALLOW_MEMORY_GROWTH -sNO_DISABLE_EXCEPTION_CATCHING)
	target_link_options(${BUILD_NAME} PRIVATE ${ASSETS_EMBED_WEB})

	if(DEBUG)
		target_compile_options(${BUILD_NAME} PRIVATE -gsource-map)
		target_link_options(${BUILD_NAME} PRIVATE -gsource-map)
		ADD_CUSTOM_TARGET(link_target1 ALL COMMAND ${CMAKE_COMMAND} -E create_symlink ../src ${CMAKE_BINARY_DIR}/src)
		ADD_CUSTOM_TARGET(link_target2 ALL COMMAND ${CMAKE_COMMAND} -E create_symlink ../include ${CMAKE_BINARY_DIR}/include)
	endif()
endif()

target_include_directories(include)

# NOTE: Checks
if(CHECKS STREQUAL ON)
	set(IWYU ON)
	set(CLANG_TIDY ON)
	set(CPPCHECK ON)
	set(SANITIZE ON)
	set(CMAKE_BUILD_TYPE Debug)
endif()

if(IWYU STREQUAL ON)
	message("Running IWYU")
	find_program(IWYU_EXE NAMES include-what-you-use iwyu REQUIRED)
	set(IWYU_COMMAND "${IWYU_EXE}" "-Xiwyu" "--mapping_file=${CMAKE_SOURCE_DIR}/.iwyu.imp")
	set_target_properties(${BUILD_NAME} PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${IWYU_COMMAND}")
endif()
if(CLANG_TIDY STREQUAL ON)
	message("Running clang tidy")
	find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}" "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy;--use-color")
	set_target_properties(${BUILD_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()
if(CPPCHECK STREQUAL ON)
	message("Running cppcheck")
	find_program(CPPCHECK_EXE NAMES cppcheck REQUIRED)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	set(CPPCHECK_COMMAND "${CPPCHECK_EXE}" "--enable=all" "--inconclusive" "--force" "--inline-suppr" "--suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-ignore" "-i${CMAKE_SOURCE_DIR}/external/")
	set_target_properties(${BUILD_NAME} PROPERTIES CXX_CPPCHECK "${CPPCHECK_COMMAND}")
endif()
if(ADDRESS STREQUAL ON)
	message("Sanitizer enabled")
	target_compile_options(${BUILD_NAME} PRIVATE -fsanitize=address -DADDRESS --coverage -g -fno-omit-frame-pointer -O0 -fno-optimize-sibling-calls)
	target_link_options(${BUILD_NAME} PRIVATE -fsanitize=address --coverage -g -O0)
endif()
if(LEAK STREQUAL ON)
	message("Sanitizer enabled")
	target_compile_options(${BUILD_NAME} PRIVATE -fsanitize=leak --coverage -g -fno-omit-frame-pointer -O0 -fno-optimize-sibling-calls)
	target_link_options(${BUILD_NAME} PRIVATE -fsanitize=leak --coverage -g -O0)
endif()
if(UNDEFINED STREQUAL ON)
	message("Sanitizer enabled")
	target_compile_options(${BUILD_NAME} PRIVATE -fsanitize=undefined -DADDRESS -g -fno-omit-frame-pointer -O0 -fno-optimize-sibling-calls)
	target_link_options(${BUILD_NAME} PRIVATE -fsanitize=undefined -g -O0)
endif()

#Check LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT LTOSupported OUTPUT error)

if(LTOSupported AND NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	if(NOT DEBUG)
		message(STATUS "IPO / LTO enabled")
		set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
		set_property(TARGET ${BUILD_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
else()
	message(STATUS "IPO / LTO not supported: <${error}>")
endif()

if(DEBUG)
	execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ../assets ${CMAKE_BINARY_DIR}/assets)
else()
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
endif()

