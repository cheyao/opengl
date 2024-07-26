#!/bin/bash
install_name_tool -change @rpath/libSDL3.0.dylib @executable_path/libSDL3.0.dylib *.app/Contents/MacOS/OpenGL
install_name_tool -change @rpath/libassimp.5.dylib @executable_path/libassimp.5.dylib *.app/Contents/MacOS/OpenGL
# install_name_tool -change @rpath/libSDL3_image.0.dylib @executable_path/libSDL3_image.0.dylib *.app/Contents/MacOS/OpenGL
cp /usr/local/lib/libSDL3.0.dylib *.app/Contents/MacOS
cp /usr/local/lib/libassimp.5.dylib *.app/Contents/MacOS
# cp /usr/local/lib/libSDL3_image.0.dylib *.app/Contents/MacOS
# See https://stackoverflow.com/questions/1596945/building-osx-app-bundle
# Maybe need to make this: https://stackoverflow.com/questions/4677044/how-to-use-dylib-in-mac-os-x-c
