#include "events/event.h"

namespace kogayonon
{
  bool Event::isInCategory(EventCategory category)
  {
    return getCategoryFlags() & category;
  }
}
