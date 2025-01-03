cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(
	OpenGL
	LANGUAGES C CXX
	VERSION 1.0
	DESCRIPTION "OpenGL Game Engine"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds not allowed! Create a build directory and run CMake from there.")
endif()

option(IMGUI 		"Enable GUI" OFF)
option(ANDROID		"Build for android (automatically build with gradle)" OFF)
option(OPTIMIZE		"Optimize a lot" OFF)
option(MOLD		"Use mold" OFF)
# TODO: option(VENDOR 		"Vendor libraries" OFF)

option(HIDPI		"High DPI" OFF)

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
src/components.cpp
src/main.cpp
src/game.cpp
src/registers.cpp
src/utils.cpp

src/components/inventory.cpp
src/components/playerInventory.cpp
src/components/crafting.cpp
src/components/noise.cpp
src/components/furnace.cpp

src/opengl/mesh.cpp
src/opengl/shader.cpp
src/opengl/texture.cpp
src/opengl/framebuffer.cpp
src/opengl/ubo.cpp

src/managers/glManager.cpp
src/managers/shaderManager.cpp
src/managers/textureManager.cpp
src/managers/eventManager.cpp
src/managers/localeManager.cpp
src/managers/entityManager.cpp
src/managers/systemManager.cpp
src/managers/storageManager.cpp

src/systems/UISystem.cpp
src/systems/textSystem.cpp
src/systems/physicsSystem.cpp
src/systems/renderSystem.cpp
src/systems/inputSystem.cpp
src/systems/worldSystem.cpp

src/scenes/level.cpp
src/scenes/chunk.cpp

src/screens/screen.cpp
src/screens/hud.cpp

# Headers
include/game.hpp
include/utils.hpp
include/components.hpp
include/scene.hpp
include/registers.hpp
include/items.hpp

include/components/inventory.hpp
include/components/playerInventory.hpp
include/components/crafting.hpp
include/components/noise.hpp
include/components/furnace.hpp

include/misc/sparse_set.hpp
include/misc/sparse_set_view.hpp

include/opengl/mesh.hpp
include/opengl/shader.hpp
include/opengl/texture.hpp
include/opengl/types.hpp
include/opengl/framebuffer.hpp
include/opengl/ubo.hpp

include/managers/glManager.hpp
include/managers/shaderManager.hpp
include/managers/textureManager.hpp
include/managers/eventManager.hpp
include/managers/localeManager.hpp
include/managers/entityManager.hpp
include/managers/systemManager.hpp
include/managers/componentManager.hpp
include/managers/storageManager.hpp

include/systems/UISystem.hpp
include/systems/textSystem.hpp
include/systems/physicsSystem.hpp
include/systems/renderSystem.hpp
include/systems/inputSystem.hpp
include/systems/worldSystem.hpp

include/scenes/level.hpp
include/scenes/chunk.hpp

include/screens/screen.hpp
include/screens/hud.hpp

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
--preload-file=assets/fonts
--preload-file=assets/shaders
--preload-file=assets/strings
--preload-file=assets/textures
)

if(ANDROID STREQUAL ON) 
	message("-- Building android apk with cmake - experimental")
	message("-- Progress bar doesn't work, please wait till the end")
	message("-- If this doesn't work, go to \"android\" folder and run ./gradlew assembleRelease")

	set(NAMESPACE com.cyao.${PROJECT_NAME})
	set(BUILD_NAME ${PROJECT_NAME})

	add_subdirectory(android)

	return()
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING
		"Choose the type of build, options are:
		 Debug Release RelWithDebInfo"
      	FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	message("-- Debug enabled")
	set(DEBUG TRUE)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

#
# 1. Check platforms
#

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	set(BUILD_NAME main) # Android needs this as libmain
else()
	set(BUILD_NAME ${PROJECT_NAME})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	set(ANDROID TRUE)

	message("-- Building for Android")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(APPLE TRUE)

	message("-- Building for MacOS, to build universal package set \"-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64\"")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS TRUE)

	set(CMAKE_EXECUTABLE_SUFFIX .exe) # Is this needed?

	message("-- Building for Windows, I recommend you to use mingw or unix stuff")
elseif(EMSCRIPTEN)
	set(WEB TRUE)

	set(CMAKE_EXECUTABLE_SUFFIX .js) # Writing javascript from C++ :sunglasses:
	configure_file(cmake/index.html.in ${CMAKE_BINARY_DIR}/index.html)

	message("-- Building for web, did you unset your env vals? \"unset CXXFLAGS CXX CPPFLAGS CC CFLAGS LDFLAGS\"")
else()
	message("-- WARNING Building for unknown platform, this might not work correctly")
endif()

#
# 2. Set compile flags and executable
#

# Clang got some errors with `unordered_map` when using c++23 and libstdc++
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT ANDROID)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fexperimental-library")
endif()

# Platform dependent flags
if (ANDROID)
	# The NDK and SDL will link for a library called libmain.so
	add_library(${BUILD_NAME} SHARED ${SRC})

	target_compile_options(${BUILD_NAME} PRIVATE -DGLES -DANDROID)
elseif(APPLE)
	if(DEBUG)
		add_executable(${BUILD_NAME} ${SRC})
	else()
		# Package into a application bundle
		add_executable(${BUILD_NAME} MACOSX_BUNDLE ${SRC})
		set_target_properties(${BUILD_NAME} PROPERTIES
		    BUNDLE True
		    MACOSX_BUNDLE_ICON_FILE ${CMAKE_SOURCE_DIR}/cmake/${CMAKE_PROJECT_NAME}.ico
		    MACOSX_BUNDLE_GUI_IDENTIFIER org.cyao.${BUILD_NAME}
	   	    MACOSX_BUNDLE_BUNDLE_NAME ${BUILD_NAME}
		    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
		    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
		    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/info.plist.in
		)
	endif()

	target_include_directories(${BUILD_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})

	target_link_libraries(${BUILD_NAME} PRIVATE ${OPENGL_LIBRARIES})
elseif(WINDOWS)
	add_executable(${BUILD_NAME} WIN32 ${SRC})

	if(NOT MSVC)
		target_compile_options(${BUILD_NAME} PRIVATE -D__WINDOWS__)
	endif()
elseif(WEB)
	add_executable(${BUILD_NAME} ${SRC})

	target_compile_options(${BUILD_NAME} PRIVATE -sUSE_SDL=3 -sUSE_FREETYPE=1 -DGLES -fno-rtti)
	target_compile_options(${BUILD_NAME} PRIVATE ${ASSETS_EMBED_WEB})
	target_link_options(${BUILD_NAME} PRIVATE -sUSE_SDL=3 -sUSE_FREETYPE=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES2 -sFULL_ES3 -sALLOW_MEMORY_GROWTH -fno-rtti -sFETCH)
	target_link_options(${BUILD_NAME} PRIVATE ${ASSETS_EMBED_WEB})

	if(DEBUG)
		target_compile_options(${BUILD_NAME} PRIVATE -gsource-map --profiling)
		target_link_options(${BUILD_NAME} PRIVATE -gsource-map --profiling)

		add_custom_target(WebCopySrc ALL COMMAND ${CMAKE_COMMAND} -E create_symlink ../src ${CMAKE_BINARY_DIR}/src)
		add_custom_target(WebCopyInclude ALL COMMAND ${CMAKE_COMMAND} -E create_symlink ../include ${CMAKE_BINARY_DIR}/include)
	else()
		target_compile_options(${BUILD_NAME} PRIVATE -flto)
		target_link_options(${BUILD_NAME} PRIVATE)
	endif()
else()
	add_executable(${BUILD_NAME} ${SRC})
endif()

# General flags
if(MSVC)
	if(DEBUG)
		# TODO: Find out the correct msvc flags
		target_compile_options(${BUILD_NAME} PRIVATE /W4 /WX /MDd /Zi /Ob0 /Od /RTC1 /W3 /GR /EHsc /utf-8)
		target_compile_definitions(${BUILD_NAME} PRIVATE /DWIN32 /D__WINDOWS__ /DDEBUG /D_DEBUG)
	else()
		target_compile_options(${BUILD_NAME} PRIVATE /MD /O2 /Ob2 /utf-8)
		target_compile_definitions(${BUILD_NAME} PRIVATE /D__WINDOWS__ /DWIND32 /DEIGEN_NO_DEBUG /DNDEBUG)
	endif()

	if (HIDPI STREQUAL ON)
		target_compile_definitions(${BUILD_NAME} PRIVATE /DHIDPI)
	endif()
else()
	if(DEBUG)
		target_compile_options(${BUILD_NAME} PRIVATE -Wall -Wextra -Werror -g -O0 -Wundef -Wshadow -Wpointer-arith -Wpointer-arith -Wwrite-strings -Wno-unknown-warning-option -fdiagnostics-color=always)
		target_compile_definitions(${BUILD_NAME} PRIVATE -DDEBUG -D_DEBUG)
	else()
		target_compile_options(${BUILD_NAME} PRIVATE -O3)
		target_compile_definitions(${BUILD_NAME} PRIVATE -DEIGEN_NO_DEBUG -DNDEBUG)
	endif()

	if (HIDPI STREQUAL ON)
		target_compile_definitions(${BUILD_NAME} PRIVATE -DHIDPI)
	endif()

	if (OPTIMIZE STREQUAL ON)
		target_compile_options(${BUILD_NAME} PRIVATE -march=native -Ofast -fopenmp -fno-math-errno)
	endif()

	if (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
		target_compile_options(${BUILD_NAME} PRIVATE -g)
	endif()
endif()

# Enable LTO for non debug
include(CheckIPOSupported)
check_ipo_supported(RESULT LTOSupported OUTPUT error)

# Web takes too long to compile with LTO
if(LTOSupported AND NOT ANDROID AND NOT DEBUG AND NOT WEB)
	message(STATUS "IPO / LTO enabled")

	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
	set_property(TARGET ${BUILD_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
elseif(NOT DEBUG)
	message(STATUS "IPO / LTO not supported: <${error}>")
endif()

#
# 3. Include packages
#

# Project include
target_include_directories(${BUILD_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# SDL
if(NOT ANDROID) # Android projects include themself using subdirectory
	if(WEB)
		find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-static Headers)
	else()
		find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared Headers) # TODO: static build option
	endif()

	target_link_libraries(${BUILD_NAME} PRIVATE ${SDL3_LIBRARIES} SDL3::Headers) # Static library?
else()
	target_link_libraries(${BUILD_NAME} PRIVATE SDL3::SDL3) # Static library?
endif()

# Freetype
if (NOT WEB AND NOT ANDROID)
	find_package(Freetype REQUIRED)
	target_include_directories(${BUILD_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})
	target_link_libraries(${BUILD_NAME} PRIVATE ${FREETYPE_LIBRARIES})
elseif(ANDROID)
	target_link_libraries(${BUILD_NAME} PRIVATE freetype)
endif()

# ImGUI
if(IMGUI STREQUAL ON)
	message("-- Enabling IMGUI")

	if(EXISTS external/imgui/backends)
		message(FATAL_ERROR "Please run git submodule update --remote --init ${CMAKE_PROJECT_DIR}/external/imgui")
	endif()

	target_sources(${BUILD_NAME} PRIVATE ${IMGUI_SRC})
	target_include_directories(${BUILD_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/imgui)

	if(MSVC)
		target_compile_definitions(${BUILD_NAME} PRIVATE /DIMGUI)
	else()
		target_compile_definitions(${BUILD_NAME} PRIVATE -DIMGUI)
	endif()
endif()

#
# 4. Post processing
# 

# Strip debug symbols
if(NOT DEBUG AND NOT ANDROID AND NOT WEB AND NOT CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo)
	message("-- Striping debug symbols")

	if(APPLE)
		add_custom_command(
			TARGET ${BUILD_NAME}
			POST_BUILD
			COMMAND ${CMAKE_STRIP}
			ARGS ${BUILD_NAME}.app/Contents/MacOS/${BUILD_NAME}
		)
	else()
		add_custom_command(
			TARGET ${BUILD_NAME}
			POST_BUILD
			COMMAND ${CMAKE_STRIP}
			ARGS ${BUILD_NAME}${CMAKE_EXECUTABLE_SUFFIX}
		)
	endif()
endif()

# Generate locals
add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/assets/strings/en.json"
	COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/cmake/localize.py" ARGS "${CMAKE_CURRENT_SOURCE_DIR}/assets/strings/strings.csv"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/strings/strings.csv"
)
add_custom_target(localize ALL DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/strings/en.json")

# Copy assets
if(DEBUG)
	add_custom_command(
		OUTPUT ${CMAKE_BINARY_DIR}/assets
		COMMAND ${CMAKE_COMMAND} -E create_symlink "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${CMAKE_CURRENT_SOURCE_DIR}/assets/strings/en.json"
	)
	add_custom_target(copy-assets ALL DEPENDS ${CMAKE_BINARY_DIR}/assets)
else()
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
endif()

# Copy stuff for apple
if(APPLE AND NOT DEBUG)
	# Don't remove, needed by the dylib helper script
	target_link_options(${BUILD_NAME} PRIVATE -headerpad_max_install_names)

	# Add assets
	file(COPY assets DESTINATION ${CMAKE_BINARY_DIR}/${BUILD_NAME}.app/Contents/Resources)
	file(COPY cmake/mac-patch-dylib.sh DESTINATION ${CMAKE_BINARY_DIR}) # Dylib helper script
endif()

#
# 5. Misc
#

# Mold linker
if(MOLD STREQUAL ON) 
	target_link_options(${BUILD_NAME} PRIVATE -fuse-ld=mold)
endif()

# Checks
if(CHECKS STREQUAL ON)
	set(IWYU       ON)
	set(CLANG_TIDY ON)
	set(ADDRESS    ON)
	set(LEAK       ON)
	set(UNDEFINED  ON)
	set(CMAKE_BUILD_TYPE Debug)
endif()

if(IWYU STREQUAL ON)
	message("-- Running IWYU")
	find_program(IWYU_EXE NAMES include-what-you-use iwyu REQUIRED)
	set(IWYU_COMMAND "${IWYU_EXE}" "-Xiwyu" "--mapping_file=${CMAKE_SOURCE_DIR}/.iwyu.imp")
	set_target_properties(${BUILD_NAME} PROPERTIES CXX_INCLUDE_WHAT_YOU_USE "${IWYU_COMMAND}")
endif()
if(CLANG_TIDY STREQUAL ON)
	message("-- Running clang tidy")
	find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}" "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy" "--use-color")
	set_target_properties(${BUILD_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
endif()
if(CPPCHECK STREQUAL ON)
	message("-- Running cppcheck")
	find_program(CPPCHECK_EXE NAMES cppcheck REQUIRED)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
	set(CPPCHECK_COMMAND "${CPPCHECK_EXE}" "--enable=all" "--inconclusive" "--force" "--inline-suppr" "--suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-ignore" "-i${CMAKE_SOURCE_DIR}/external/")
	set_target_properties(${BUILD_NAME} PROPERTIES CXX_CPPCHECK "${CPPCHECK_COMMAND}")
endif()
if(ADDRESS STREQUAL ON)
	message("-- Address sanitizer enabled")
	target_compile_options(${BUILD_NAME} PRIVATE -fsanitize=address -DADDRESS --coverage -g -fno-omit-frame-pointer -O0 -fno-optimize-sibling-calls)
	target_link_options(${BUILD_NAME} PRIVATE -fsanitize=address --coverage -g -O0)
endif()
if(LEAK STREQUAL ON)
	message("-- Leak sanitizer enabled")
	target_compile_options(${BUILD_NAME} PRIVATE -fsanitize=leak --coverage -g -fno-omit-frame-pointer -O0 -fno-optimize-sibling-calls)
	target_link_options(${BUILD_NAME} PRIVATE -fsanitize=leak --coverage -g -O0)
endif()
if(UNDEFINED STREQUAL ON)
	message("-- Undefined sanitizer enabled")
	target_compile_options(${BUILD_NAME} PRIVATE -fsanitize=undefined -coverage -g -fno-omit-frame-pointer -O0 -fno-optimize-sibling-calls)
	target_link_options(${BUILD_NAME} PRIVATE -fsanitize=undefined -coverage -g -O0)
endif()
