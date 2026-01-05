# Kogayonon Game Engine
:heavy_exclamation_mark: This is a work in progress :heavy_exclamation_mark:</br>

Hi there, this is my most ambitious project ever in my favourite language, C++.

It is a lightweight game engine that at some point in the not so far future, will be able to create games.

Written using C++ 20.

## Youtube clips
* Rigid bodies workflow
[![Rigid bodies with Nvidia PhysX](https://img.youtube.com/vi/tkhdwBTy-jc/0.jpg)](https://www.youtube.com/watch?v=tkhdwBTy-jc "Rigid bodies with Nvidia PhysX")

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
vcpkg install glm entt sdl2 soil2 spdlog physx rapidjson tinyfiledialogs
```

Or if you use Visual Studio IDE hover over the find package error in CmakeLists and install from there

```
find_package(SDL2 CONFIG REQUIRED)
find_package(soil2 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(rapidjson CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)
```
## Thirdparty :v:
This project would not be possible without those open source projects

[EnTT](https://github.com/skypjack/entt) - entity component system</br>
[SDL2](https://github.com/libsdl-org/SDL)</br>
[ImGui](https://github.com/ocornut/imgui) - c++ immediate mode gui</br>
[GLM](https://github.com/g-truc/glm) - math</br>
[PhysX](https://github.com/NVIDIA-Omniverse/PhysX) - physics</br>
[tinyfiledialogs](https://github.com/native-toolkit/libtinyfiledialogs) </br>
[spdlog](https://github.com/gabime/spdlog) - logging </br>
[rapidjson](https://github.com/Tencent/rapidjson)</br>

## Authors
Only me :grin: