# kogayonon game engine

This is a game engine im currently working on.
It is written in c++ having c++17 as standard.
- [x] Model loading with assimp[^2]
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

## Changelog I guess 
This is going to start from today (23/02/2025) because i want to keep track of my changes and improvements

 - 23/02/2025
   - async loading of models

[^1]: [ImGui repo](https://github.com/ocornut/imgui)
[^2]: [Assimp repo](https://github.com/assimp/assimp)
