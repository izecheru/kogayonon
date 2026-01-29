#pragma once
#include <memory>

namespace kogayonon_game
{
struct GameState
{
};

class Game
{
public:
  Game();
  ~Game() = default;

  void init();
  void run();
  void close();
  void save();
  void pause();
  void resume();

private:
  void prepareAssets();

private:
  GameState m_state;
  // std::unique_ptr<kogayonon_core::Registry> m_registry;
};
} // namespace kogayonon_game