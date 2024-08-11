# Android README

Hello everyone, I'm going to write here a few difficulties that I encountered when I ported my game engine to Android:

1. `SDL_GetBasePath()` doesn't return anything on android, so you should use and empty string as the base path (""), __NOT__ "./" as in the docs
Example: to access `assets/texture.png`, just pass the string "assets/texture.png" to SDL IO.

2. All the direct filesystem calls have to go: `<filesystem>` doesn't work on Android and produces cryptic false-positive runtime exceptions. 

Stuff like `stbi_load` should be replaced with  a combination of `SDL_LoadFile` which loades the file to memory and `stbi_load_from_memory` which reads from the memory.

For assimp you need to compile it yourself (see `README.md`). You do __NOT__ need `ASSIMP_ANDROID_JNIIOSYSTEM`. It won't work, trust me.

Link it by specifing the directory of your compiled assimp in `CMakeLists.txt`

Dynamic link it, don't worry

For assimp you also need custom IOs, see `src/managers/assimpIO.cpp`

`SDL_GetPathInfo` doesn't work on android, so you must test the file by trying to make a IO stream with it, slow but no other method

3. Android rotation should not be specified in `androidmanifest.xml`, it won't work. Instead set the hint:

```
SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
```

Also to hide the top bar, just set the window to fullscreen

4. The assets shall go to `android/app/src/main/assets`, it get's packed automatically

5. OpenGL `glPolygonMode` isn't present in OpenGL ES, Remove it

6. When looking at SDL examples, look at `opengles2` __the 2 is important__. __Same__ when setting the driver hint.

7. Use glad's `gladLoadGLES2Loader` to load opengl funcs

8. Don't forget to replace the version string when compiling shaders. ImGui too.

9. Android NDK's stdc++ is horribly outdated, stuff like `<ranges>` and `string.contains()` should be used behind feature tests.

Here is a patch to make Eigen work

```
@@ -1306,14 +1306,14 @@ inline int queryTopLevelCacheSize() {
 * This wraps C++20's std::construct_at, using placement new instead if it is not available.
 */

#if EIGEN_COMP_CXXVER >= 20
using std::construct_at;
#else
// #if EIGEN_COMP_CXXVER >= 20
// using std::construct_at;
// #else
template <class T, class... Args>
EIGEN_DEVICE_FUNC T* construct_at(T* p, Args&&... args) {
  return ::new (const_cast<void*>(static_cast<const volatile void*>(p))) T(std::forward<Args>(args)...);
}
#endif
// #endif

/** \internal
 * This wraps C++17's std::destroy_at.  If it's not available it calls the destructor.
```

10: Here is the order for initializing OpenGL:

    1. Set SDL Hints
    
    2. Init SDL

    3. Set GL atrributes

    4. Make window

    5. Make context

    6. Load GL with glad

    7. Make context current

    8. Print gl info

11. Don't forget to change context flags:

```
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
```

12. Android needs java __17__, and can only use 17. While compiling assimp add the API level 24 flag.

13. When using SDL android projects, just delete all the android.mk

14. `Uncaught RuntimeError: indirect call to null` is probably just a undefined opengl function in Emscripten

15. When compiling for web, you need to add flags: `-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES2 -sFULL_ES3 -sALLOW_MEMORY_GROWTH` or your app will break.
