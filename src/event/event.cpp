#include "event/event.h"

namespace kogayonon
{
  bool Event::isInCategory(EventCategory category)
  {
    return getCategoryFlags() & category;
  }
} // namespace kogayonon
