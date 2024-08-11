# Android README

Hello everyone, I'm going to write here a few difficulties that I encountered when I ported my game engine to Android:

1. `SDL_GetBasePath()` doesn't return anything on android, so you should use and empty string as the base path (""), __NOT__ "./" as in the docs
Example: to access `assets/texture.png`, just pass the string "assets/texture.png" to SDL IO.

2. All the direct filesystem calls have to go: `<filesystem>` doesn't work on Android and produces cryptic false-positive runtime exceptions. 

Stuff like `stbi_load` should be replaced with  a combination of `SDL_LoadFile` which loades the file to memory and `stbi_load_from_memory` which reads from the memory.

For assimp you need to compile it yourself (see `README.md`). You do __NOT__ need `ASSIMP_ANDROID_JNIIOSYSTEM`. It won't work, trust me.

Link it by specifing the directory of your compiled assimp in `CMakeLists.txt`
