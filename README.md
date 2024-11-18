<h1 align="center" style="text-shadow: 2px 2px 5px red;">Cyao Game Engine</h1>
<h2 align="center">Features</h2>

<ul>
    <li><b>Cross-Platorm</b> 💻📱: <b>MacOS</b>, <b>Linux</b>, <b>Windows</b>, <b>Web</b>, <b>Android</b> and <b>IOS</b> all supported!</li>
    <li><b>Performant</b> 🏃: Uses C++, the game runs smoothly even on browsers. (<b>60</b>+ FPS on a <b>10 year old</b> laptop with <b>5000</b>+ entities!)</li>
    <li><b>Multilingual</b> 🌐: <b>Unicode</b> and Locale support build in. (No more �!)</li>
    <li><b>Modern</b> ✨: Uses <b>Modern</b> C++20 features and follows best practices. SDL3 used!</li>
    <li><b>Extendable</b> 🧩: The Game Engine uses the <b class="flip">Bleeding-edge</b> ECS arcitecture.</li>
    <li><b>2D and 3D</b> ❤️: Make <b>2D</b> and <b>3D</b> Together!</li>
    <li><b>Debuggable</b> 🔍: Custom asserts and <b>Debug menu</b> helps you identify any bugs 🐛!</li>
    <li><b>Custom Saves</b> 📝: Easily save <b>any</b> data on any platform you want in a json file, the game engine will manage it for you!</li>
</ul>

<h3 align="center"><a href="https://cheyao.github.io/game">Try out web version now!</a></h3>

<h3 align="center">Debug mode</h3>
<div align="center">
    <img src="https://media.githubusercontent.com/media/cheyao/opengl/main/misc/debug.png" width="auto" height="auto" alt="Debug menu">
</div>

<h2 align="center">Building</h2>

Dependencies:

- Assimp
- SDL3
- Freetype 2

FIXME:
1. chunk 0 offset
2. break no contain?

### MacOS and Linux

Install `assimp` and `freetype` with your package manager

Compile SDL3:
```bash
$ git clone --depth 1 https://github.com/libsdl-org/SDL.git
$ cd SDL 
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
$ sudo cmake --install .
```

Install dependencies:

```bash
# Install freetype 2
# MacOS
$ brew install freetype
# Linux
$ sudo pacman -S freetype2
```

Compile project:

```bash
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
```bash
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
```bash
$ brew install --cask android-platform-tools android-ndk temurin@17
$ brew install cmake
```

Add these to your `.zshrc` (Change it to the appropriate folders on linux):
```bash
$ export ANDROID_NDK_HOME="/usr/local/share/android-ndk"
$ export JAVA_HOME=`/usr/libexec/java_home -v 17` # Yes, you __must__ use java 17, blame android
$ export ANDROID_HOME="/usr/local/share/android-commandlinetools/"
```

For android you need to compile assimp by yourself:

Change the `CMAKE_TOOLCHAIN_FILE` if you are not on MacOS:
```bash
$ git clone --depth 1 https://github.com/assimp/assimp.git
$ cd assimp
$ mkdir build && cd build
$ cmake -DASSIMP_ANDROID_JNIIOSYSTEM=OFF \
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

Configure your gradlew's credentials (I'm not gonna let you use mine duh)
```
File: ~/.gradle/gradle.properties

RELEASE_STORE_FILE=[KEYSTORE PATH RELATIVE FROM ./android]
RELEASE_STORE_PASSWORD=[KEYSTORE PASSWORD]
RELEASE_KEY_ALIAS=[KEY NAME]
RELEASE_KEY_PASSWORD=[KEY PASSWORD]
```

Now build the project:
```bash
$ git clone --depth 1 https://github.com/cheyao/opengl.git
$ cd opengl
$ git submodule update --init --recursive
$ mkdir build && cd build 
$ cmake -DCMAKE_BUILD_TYPE=Release \
        -DANDROID=ON \
        ..
# Note: You might need to edit CMakeLists.txt:320 to point to your assimp lib dir
$ cmake --build .
```
Now there is the apk in the folder

## Web 
Install [emsdk](https://emscripten.org/) to `~/emsdk`

On arch just run `sudo pacman -S emscripten`

Build SDL3:

```bash
$ source ~/emsdk/emsdk_env.sh # Only on MacOS
$ git clone --depth 1 https://github.com/libsdl-org/SDL.git
$ cd SDL 
$ mkdir build && cd build
$ emcmake cmake ..
$ cmake --build .
$ cmake --install .
```

Build assimp:
```bash
$ git clone --depth 1 https://github.com/assimp/assimp && cd assimp
$ mkdir build && cd build
& cmake -DASSIMP_BUILD_ZLIB=ON -DBUILD_SHARED_LIBS=OFF ..
$ cmake --build .
```

Build the project:
```bash
$ git clone --depth 1 https://github.com/cheyao/opengl.git
$ cd opengl
$ mkdir build && cd build 
$ emcmake cmake -DCMAKE_BUILD_TYPE=Release ..
$ emcmake cmake --build .
$ python3 -m http.server
```

If you get the error `emcc: error: unable to create cache directory "{cachdir}"` run the build command with sudo

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

## Notes

The linux save directory is `~/.local/share/cyao/opengl/`

## TODO

- [x] In-game metrics & ms count
    - [x] ImGUI Menu
- [ ] Live edit shaders in-game
- [ ] Fix MacOS bundle process https://github.com/libsdl-org/SDL/blob/main/docs/README-macos.md
- [x] Game pause + resume things
- [x] Debug menu with ImGUI and item names
- [x] Better component and actors
- [x] UI Scaling
- [x] Multitouch
- [x] Collision resolver
- [x] Scaling
- [x] Save state
- [x] Android build
    - [x] Android controls
- [x] Debug menu 
    - [x] Collision area editor
    - [x] Signal list
    - [x] Velocity arrows
    - [x] Component and entity list
        - [ ] Real time component and entity addition
- [ ] UI
    - [ ] Embed text in buttons
    - [ ] MacOS: Link new freetype version
- [ ] Pause menu etc
    - [x] Text
    - [ ] Settings
- [x] Localization
    - [x] Fallback fonts.
- [x] Emscripten build
    - [ ] Progress bar
    - [ ] Text with contrast
- [x] TTF
    - [ ] Button auto scale + text
    - [ ] Generate texturemap
    - [ ] Signed distance field fonts https://steamcdn-a.akamaihd.net/apps/valve/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf
- [ ] Windows build
- [ ] Level map
    - [ ] Level generator
    - [ ] Thing
    - [ ] Binary json (MessagePack)
    - [x] Move scenes to `src/scenes`
- [ ] Grid map
- [ ] Add functional hot reload
- [ ] Entity name saved
- [x] Combine localemanager and textmanager
- [ ] Shadows
- [ ] Multithreading
    - [ ] Preload

- [ ] Read books (By priority)
    - [ ] [Effective Modern C++]()
    - [ ] [Game maths](https://gamemath.com/book/intro.html)
    - [ ] [Game engine architecture](https://www.amazon.com/dp/1568814135)
    - [ ] [Game programming gems](https://www.satori.org/game-programming-gems/)
    - [ ] [Real time rendering](https://www.amazon.com/dp/1568814240)
    - [ ] [Real time collision detection](https://www.amazon.com/dp/1558607323)

# :3
