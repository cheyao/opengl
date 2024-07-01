#!/bin/bash
echo "You need to have patchelf installed"

OUTPUT_NAME=Golf

mkdir output

if [ ! -f /usr/lib/libSDL3.so.0 ]; then
	export SDL_DIR=/usr/local/lib/
else
	export SDL_DIR=/usr/lib/
fi

mkdir build64
cd build64
cmake ..
cmake --build . -- -j4
mv ${OUTPUT_NAME} ../output/${OUTPUT_NAME}64
mv assets ../output/
cd ../output/
mkdir lib64
patchelf --set-rpath '$ORIGIN/lib64' ${OUTPUT_NAME}64

cp ${SDL_DIR}/libSDL3.so.0 lib64
cp ${SDL_DIR}/libSDL3_image.so.0 lib64

cd ..

echo "Building 32 bit"
export LDFLAGS=-m32
export CFLAGS=-m32
export CXXFLAGS=-m32

if [ ! -f /usr/lib32/libSDL3.so.0 ]; then
	export SDL_DIR=/usr/local/lib32/
else
	export SDL_DIR=/usr/lib32/
fi

mkdir build32
cd build32
cmake ..
cmake --build . -- -j 4
mv ${OUTPUT_NAME} ../output/${OUTPUT_NAME}32
cd ../output/
mkdir lib32
patchelf --set-rpath '$ORIGIN/lib32' ${OUTPUT_NAME}32

cp ${SDL_DIR}/libSDL3.so.0 lib32
cp ${SDL_DIR}/libSDL3_image.so.0 lib32

cd ..

unset LDFLAGS
unset CFLAGS
unset CXXFLAGS

cp cmake/linux-launcher output/${OUTPUT_NAME}
sed -i "s/PROJECT_NAME/${OUTPUT_NAME}/g" output/${OUTPUT_NAME}

rm -rf build32
rm -rf build64

echo "Output: output"

