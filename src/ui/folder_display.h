#pragma once
#include "display.h"

namespace kogayonon
{
class FolderDisplay : public Display
{
public:
  FolderDisplay(std::string name, std::string path) : Display(std::move(name)), m_path(std::move(path)) {}

  void draw() override;

  inline std::string getPath() const
  {
    return m_path;
  }

private:
  std::string m_path;
};
} // namespace kogayonon