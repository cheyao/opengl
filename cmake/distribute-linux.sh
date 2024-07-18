#!/bin/bash
echo "You need to have patchelf installed"

OUTPUT_NAME=OpenGL

mkdir output

if [ ! -f /usr/lib/libSDL3.so.0 ]; then
	export SDL_DIR=/usr/local/lib/
else
	export SDL_DIR=/usr/lib/
fi

mkdir build
cd build
cmake ..
cmake --build . -- -j4
mv ${OUTPUT_NAME} ../output/${OUTPUT_NAME}
mv assets ../output/
cd ../output/
mkdir libs
patchelf --set-rpath '$ORIGIN/libs' ${OUTPUT_NAME}

cp ${SDL_DIR}/libSDL3.so.0 libs
cp ${SDL_DIR}/libSDL3_image.so.0 libs
# cp /usr/local/libassimp.so.5 libs

cd ..

unset LDFLAGS
unset CFLAGS
unset CXXFLAGS

rm -rf build

echo "Output: output"

