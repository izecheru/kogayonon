# Kogayonon Game Engine
:heavy_exclamation_mark: This is a work in progress :heavy_exclamation_mark:</br>

Hi there, this is my most ambitious project ever in my favourite language, C++.

It is a lightweight game engine that at some point in the not so far future, will be able to create games.

Written using C++ 20.

## Screenshots

Soon... :framed_picture:

## Features :gear:
- gltf model loading
    - autoloads textures that it finds (configured only for some basic materials
- texture loading (.png and .jpg)
- drag and drop assets into the viewport to load them and create a specific type of entity
- can select and deselect an entity and modify it's properties
    - does not support viewport entity selection based on a ray tracer (TODO)
- listens for file events from resources folder (can hot reload if needed, currently I do not do this for any type of asset)
- *instanced rendering*, instead of rendering a type of model for 100 times with 100 calls, we just have an instance matrix for each instance and draw them all with a single call to ``` glDrawElementsInstanced ```
    - this is also a work in progress since the code is very ugly at the moment

## Build :building_construction:
### Requires vcpkg
```
git clone https://github.com/microsoft/vcpkg
./vcpkg/bootstrap-vcpkg.bat -disableMetrics
```
Set those two variables
```
VCPKG_ROOT=[path_to_vcpkg]
VCPKG_DEFAULT_TRIPLET=x64-windows
```
Open a terminal and type :computer:
```
vcpkg integrate install
vcpkg integrate powershell
```

Finally you can install the *current* dependencies using a terminal
```
vcpkg install glm entt sdl2 soil2 spdlog
```
Or if you use Visual Studio IDE hover over the find package error in CmakeLists and install from there
```
find_package(SDL2 CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
```
## Thirdparty :v:
This project would not be possible without those open source projects

[EnTT](https://github.com/skypjack/entt) - entity component system</br>
[SDL2](https://github.com/libsdl-org/SDL)</br>
[ImGui](https://github.com/ocornut/imgui) - c++ immediate mode gui</br>
[GLM](https://github.com/g-truc/glm) - math</br>

## Authors
Only me :grin: