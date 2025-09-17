#pragma once
#include "rendering/batcher.h"

namespace kogayonon_resources
{
class Model;
}

namespace kogayonon_rendering
{
class ModelBatchRenderer : public Batcher<kogayonon_resources::Model>
{
public:
  ModelBatchRenderer();
  ~ModelBatchRenderer();

private:
};
} // namespace kogayonon_rendering