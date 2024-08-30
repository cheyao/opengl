<div align="center">
    <img src="https://raw.githubusercontent.com/cheyao/opengl/main/misc/styles.svg" width="400" height="400" alt="css-in-readme">
</div>

<h3><a href="https://cheyao.github.io/game">Try out web version now!</a></h3>

<h2 align="center">Building</h2>

<div class="note">

Some *emphasis* and <strong>strong</strong>!

</div>

Dependencies:

- Assimp
- SDL3
- Freetype 2

### MacOS and Linux

Install `assimp` and `freetype` with your package manager

Compile SDL3:
```
$ git clone --depth 1 https://github.com/libsdl-org/SDL.git
$ cd SDL 
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
$ sudo cmake --install .
```

Install dependencies:

```
# Install freetype 2
# MacOS
$ brew install freetype
# Linux
$ sudo pacman -S freetype2
```

Compile project:

```
$ git clone --depth 1 https://github.com/cheyao/opengl.git
$ cd opengl 
$ mkdir build && cd build 
$ cmake ..
$ cmake --build .

For Linux:
$ ./OpenGL

For MacOS:
$ mv OpenGL.app /Applications/
$ open /Applications/OpenGL.app # Or just openg the app
```

If you are getting this error:
```
Undefined symbols for architecture x86_64:
  "Assimp::IOSystem::CurrentDirectory[abi:cxx11]() const", referenced from:
      vtable for GameIOSystem in IOManager.cpp.o
ld: symbol(s) not found for architecture x86_64
collect2: error: ld returned 1 exit status
```
This means that you didn't use the same compiler to compile assimp and this engine (clang and gcc are somehow not compatable)

### Windows

Same, use cmake to build project

### Android

MacOS Dependencies:
```
$ brew install --cask android-platform-tools
$ brew install --cask android-ndk
$ brew install --cask android-platform-tools
$ brew install --cask temurin@17
$ brew install ninja cmake
```

Add these to your `.zshrc` (Change it to the appropriate folders on linux):
```
export ANDROID_NDK_HOME="/usr/local/share/android-ndk"
export JAVA_HOME=`/usr/libexec/java_home -v 17` # Yes, you __must__ use java 17, blame android
export ANDROID_HOME="/usr/local/share/android-commandlinetools/"
```

For android you need to compile assimp by yourself:

Change the `CMAKE_TOOLCHAIN_FILE` if you are not on MacOS:
```
$ git clone --depth 1 https://github.com/assimp/assimp.git
$ cd assimp
$ mkdir build && cd build
$ cmake -DASSIMP_ANDROID_JNIIOSYSTEM=ON \
        -DCMAKE_TOOLCHAIN_FILE=/usr/local/share/android-ndk/build/cmake/android.toolchain.cmake \
        -DANDROID_NDK=${ANDROID_NDK_HOME} \
        -DCMAKE_BUILD_TYPE=Release \
        -DANDROID_ABI=arm64-v8a \
        -DBUILD_SHARED_LIBS=1 \
        -DANDROID_NATIVE_API_LEVEL=24 \
        ..
$ cmake --build .
$ sudo cmake --install .
```

Now build the project:
```
$ git clone --depth 1 https://github.com/cheyao/opengl.git
$ cd opengl
$ git submodule --init --recrusive
$ mkdir build && cd build 
$ cmake -DCMAKE_BUILD_TYPE=Release \
        -DANDROID=ON \
        ..
# Note: You might edit CMakeLists.txt
$ cmake --build .
# Now there is the apk in the folder
```

## Improving performance
Compile with the flag `-DOPTIMIZE=ON`, but you won't be able to send the binary to others (Enables also unsafe floating points)

## Distribuing
There are scripts in the cmake folder:

```
cmake
├── distribute-linux.sh
├── getdll32.sh
├── getdll64.sh
├── index.html.in
├── info.plist.in
├── mac-patch-dylib.sh
└── sign-apk.sh
```

- `./cmake/distribute-linux.sh` For patching the executable then copying the dlibs into `libs` folder, should be enough to patch for linux release
- `./cmake/mac-patch-dylib.sh` Same but for mac, only works for bundled macos app
- `./cmake/index.html.in` Is the Emscripten idex
- `./cmake/sign-apk.sh` Is to sign the android apk

## Assets
The assets should be bundled automatically with android and macos, on other platforms you must distribute with the assets (that are copied into the project folder)

```
assets
├── fonts
├── models
├── shaders
└── textures
    └── ui
```

- `./assets/fonts/` Fonts
- `./assets/models/` 3D models
- `./assets/shaders/` shaders
- `./assets/textures/` Textures, the models will look here for the corresponding textures
- `./assets/textures/ui` Textures for the UI
- `./assets/strings.csv` Source loc csv file (See [here](https://en.wikipedia.org/wiki/IETF_language_tag) for language codes)
- `./assets/strings.json` Output loc file (to be generated) 

## Explanation

This game engine uses the Entity-Component-System architecture, implemented using `Scenes` and `sparse_sets`.

Entities are just UUIDs

Components are just plain old structs

The systems manage all the logic and changes the components

## TODO

- [x] Game pause + resume things
- [x] Debug menu with ImGUI and item names
- [x] Better component and actors
- [x] UI Scaling
- [x] Multitouch
- [x] Collision resolver
- [ ] Scaling
- [x] Save state
- [x] Android build
    - [x] Android controls
- [x] Debug menu 
    - [x] Collision area editor
    - [x] Signal list
    - [x] Velocity arrows
    - [x] Component and entity list
        - [ ] Real time component and entity addition
- [x] UI
    - [x] Divs
    - [x] Embed text in buttons
    - [ ] MacOS: Link new freetype version
- [x] Pause menu etc
    - [x] Text
    - [ ] Settings
- [x] Localization
    - [x] Fallback fonts.
    - [ ] Maybe [ICU](https://icu.unicode.org/)?
- [x] Emscripten build
    - [ ] Progress bar
    - [ ] White text
- [x] TTF
    - [ ] Button auto scale + text
    - [ ] Generate texturemap
    - [ ] Signed distance field fonts https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
- [ ] Entity name saved
- [ ] Multithreading
    - [ ] Preload
- [ ] Shadows
- [ ] Add functional hot reload
- [ ] Grid map
- [ ] Level map
    - [ ] Level generator
    - [ ] Thing
    - [ ] Binary json (MessagePack)
    - [ ] Move scenes to `src/scenes`
- [ ] Combine localemanager and textmanager
- [ ] Windows build

- [ ] Read books (By priority)
    - [ ] [Effective Modern C++]()
    - [ ] [Game maths](https://gamemath.com/book/intro.html)
    - [ ] [Game engine architecture](https://www.amazon.com/dp/1568814135)
    - [ ] [Game programming gems](https://www.satori.org/game-programming-gems/)
    - [ ] [Real time rendering](https://www.amazon.com/dp/1568814240)
    - [ ] [Real time collision detection](https://www.amazon.com/dp/1558607323)

# :3
