# Tiny Game Engine

Hello and welcome to the repo of my tiny game engine.

Im currently following [Learn OpenGL](https://learnopengl.com), but instead of their flat, multifile style of coding, I am integrating everything into a component-based game system.

Hope everything works out!

# Building

## MacOS and Linux

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
$ git clone https://github.com/cheyao/opengl.git
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

## Windows

Best luck, my CMakeLists.txt theoretically can support msvc, but if it doesn't please submit a pr, if it doesn't work just use WSL

# Improving performance
Compile with the flag `-DOPTIMIZE`, but you won't be able to send the binary to others

# Distribuing
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

`./cmake/distribute-linux.sh` For patching the executable then copying the dlibs into `libs` folder, should be enough to patch for linux release

`./cmake/mac-patch-dylib.sh` Same but for mac, only works for bundled macos app

Others are to-fix stuff (For android and web)

TODO: Improve performance by revamping hot reload

"Planet Earth (Alt-Drag to change Lighting)" (https://skfb.ly/Ft8J) by Thomas Kole is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
