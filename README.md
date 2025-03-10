# kogayonon game engine

This is a game engine im currently working on.
It is written in c++ having c++17 as standard.

Features implemented and soon to be implemented
- [x] Model loading with cgltf[^2]
    - [x] Async model loading from multiple threads
    - [x] Separate file streams for parallel processing of models
    - [ ] Model animations
- [x] ImGui[^1]
- [x] Layer based rendering
- [x] Event system(keyboard, mouse ...)
    - [x] Event listener and dispatcher
    - [ ] Ingame events
    - [ ] Unsubscribe mechanism (eg. if inventory is open close it with escape, not open main menu)
- [ ] Multiple shaders
- [ ] Phisics
- [ ] Lightning
- [ ] Map
  - [ ] Entities (eg. place them somewhere in the map)
  - [ ] Level editor

[^1]: [ImGui repo](https://github.com/ocornut/imgui)
[^2]: [CGLTF repo](https://github.com/jkuhlmann/cgltf)