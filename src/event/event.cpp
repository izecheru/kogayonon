#include "events/event.h"

bool Event::isInCategory(EventCategory category) {
  return getCategoryFlags() & category;
}