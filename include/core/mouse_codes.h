#pragma once
using MouseCode = uint16_t;

enum : MouseCode
{
  BUTTON_1 = 0,
  BUTTON_2 = 1,
  BUTTON_3 = 2,
  BUTTON_4 = 3,
  BUTTON_5 = 4,
  BUTTON_6 = 5,
  BUTTON_7 = 6,
  BUTTON_8 = 7,
  BUTTON_LAST = BUTTON_8,
  BUTTON_LEFT = BUTTON_1,
  BUTTON_RIGHT = BUTTON_2,
  BUTTON_MIDDLE = BUTTON_3,
};

using MouseAction = uint16_t;
enum :MouseAction
{
  Release = 0,
  Press = 1,
};

using MouseModifier = uint16_t;
enum :MouseModifier
{
  Shift = 1,
  Control = 2,
  Alt = 4,
  Super = 8,
  CapsLock = 10,
  NumLock = 20,
};

