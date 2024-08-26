# Tiny Game Engine

[See the Project on hackclub.com](https://hackclub.com/arcade/showcase/project/receEPjMZdAepNusN/)

[Web version](https://cheyao.github.io/game)

Hello and welcome to the repo of my tiny game engine.

The resources that I used are:
- [Learn OpenGL](https://learnopengl.com)
- [Game Programming in C++](https://github.com/gameprogcpp/code) <- Do me a favour and don't read this book, it's a bad one
- [OpenGL RedBook](https://www.opengl-redbook.com)

The UI part is compleatly thought up and created by me.

Hope everything works out!

Dependencies:

- Assimp 
- SDL3 
- Eigen (Included in the repo)

## Building

### MacOS and Linux

Install `assimp` with your package manager

Compile SDL3 (Yeah, it's experimental so no binaries atm):
```
$ git clone --depth 1 https://github.com/libsdl-org/SDL.git
$ cd SDL 
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
$ sudo cmake --install .
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

Best luck, my CMakeLists.txt theoretically can support msvc, but if it doesn't please submit a pr, if it doesn't work just use WSL (or go use liux).

### Android

MacOS Dependencies:
```
$ brew install --cask android-platform-tools
$ brew install --cask android-ndk
$ brew install --cask android-platform-tools
$ brew install --cask temurin@17
$ brew install ninja cmake
```

Add these to your `.zshrc`:
```
export ANDROID_NDK_HOME="/usr/local/share/android-ndk"
export JAVA_HOME=`/usr/libexec/java_home -v 17`
export ANDROID_HOME="/usr/local/share/android-commandlinetools/"
```

For android you need to compile assimp by yourself:

Change the `CMAKE_TOOLCHAIN_FILE` if you are not on MacOS:
```
$ git clone --depth 1 https://github.com/assimp/assimp 
$ cd assimp 
$ mkdir build && cd build
$ cmake -G Ninja\ 
        -DASSIMP_ANDROID_JNIIOSYSTEM=ON \
        -DCMAKE_TOOLCHAIN_FILE=/usr/local/share/android-ndk/build/cmake/android.toolchain.cmake \
        -DANDROID_NDK=${ANDROID_NDK_HOME} \
        -DCMAKE_BUILD_TYPE=Release \
        -DANDROID_ABI=arm64-v8a \
        -DBUILD_SHARED_LIBS=1 \
        -DANDROID_NATIVE_API_LEVEL=24 \
        ..
```

Now build the project:
```
$ git clone --depth 1 https://github.com/cheyao/opengl.git 
$ cd opengl
$ git submodule --init --recrusive
$ mkdir build && cd build 
$ cmake -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DANDROID=ON \
        ..
$ ninja
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
- [x] Android build
    - [x] Android controls
- [x] UI
    - [x] Divs
    - [x] Embed text in buttons
- [x] Pause menu etc
    - [x] Text
    - [ ] Settings
- [x] Localization
    - [x] Fallback fonts.
    - [ ] Maybe [ICU?](https://icu.unicode.org/)
- [x] Emscripten build
    - [ ] Web detect android
    - [ ] Progress bar
    - [ ] White text
- [x] TTF
    - [ ] Button auto scale + text
    - [ ] Generate texturemap
    - [ ] Signed distance field fonts https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
- [x] Debug menu 
    - [x] Collision area editor
    - [ ] Signal list
- [ ] Entity name saved
- [ ] Windows build
- [ ] Multithreading
- [ ] Shadows
- [ ] Save state
- [ ] Preload
- [ ] Add functional hot reload
- [ ] Grid map
- [ ] Combine localemanager and textmanager
- [ ] Move scenes to `src/scenes`

- [ ] Read books (By priority)
    - [ ] [Effective Modern C++]()
    - [ ] [Game maths](https://gamemath.com/book/intro.html)
    - [ ] [Game engine architecture](https://www.amazon.com/dp/1568814135)
    - [ ] [Game programming gems](https://www.satori.org/game-programming-gems/)
    - [ ] [Real time rendering](https://www.amazon.com/dp/1568814240)
    - [ ] [Real time collision detection](https://www.amazon.com/dp/1558607323)

## Notes

Language tags are first looking up main tag, if matching, use it, otherwise find sumtag, and if still not present, use `en`.

Locales: First ISO-639 language specifier (such as "en" for English, "de" for German, etc). Then country strings in the format YY, where "YY" is an ISO-3166 country code

Just use `git diff` to get the changes

From discord:

It's generally recommended to const-ref objects of which size extends 2 * sizeof(std::uintmax_t) aka 16 bytes

Thx to Thimas Kole for his earth model: "Planet Earth (Alt-Drag to change Lighting)" (https://skfb.ly/Ft8J).

https://harfbuzz.github.io/ ?

https://github.com/fribidi/fribidi/ ?

https://www.unicode.org/reports/tr9/ ?

https://faultlore.com/blah/text-hates-you/

https://stackoverflow.com/questions/2071621/how-to-do-opengl-live-text-rendering-for-a-gui

# :3
