module;

#include <SDL2/SDL.h>

export module keys;

export namespace keys
{

  /*
   * Key check function names:
   *
   *   [KeyDown|KeyUp|MouseDown|MouseUp|MouseWheel|MouseMotion]_[Free|Ctrl|Shift|Alt|CtrlShift|CtrlAlt|ShiftAlt|CtrlShiftAlt]<KEY_NAME>
   *
   *  where:
   *
   *  [a|b|c] - one of A, or B, or C.
   *
   *  <KEY_NAME> - one of:
   *
   *  1) One of ABCD...XYZ - key on keyboard.
   *  2) Primary           - primary key on mouse
   *  3) Secondary         - secondary key on mouse
   *  4) Middle            - middle key on mouse
   */

  bool KeyDown_CtrlQ(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_q
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeSpace(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_SPACE
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool MouseDown_FreePrimary(const SDL_Event &event)
  {
    const SDL_Keymod mod = SDL_GetModState();

    // clang-format off
    return true

            && event.type == SDL_MOUSEBUTTONDOWN
            && event.button.button == SDL_BUTTON_LEFT
            && (mod & KMOD_CTRL  ) == 0
            && (mod & KMOD_SHIFT ) == 0
            && (mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeW(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_w
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeS(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_s
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeA(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_a
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeD(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_d
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeQ(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_q
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeE(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_e
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeRight(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_RIGHT
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeLeft(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_LEFT
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeUp(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_UP
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_FreeDown(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_DOWN
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftW(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_w
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftS(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_s
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftA(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_a
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftD(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_d
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftQ(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_q
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftE(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_e
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlW(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_w
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlS(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_s
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlA(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_a
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlD(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_d
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlE(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_e
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_AltW(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_w
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_AltS(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_s
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_AltA(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_a
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_AltD(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_d
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_AltQ(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_q
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_AltE(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_e
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftW(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_w
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftS(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_s
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftA(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_a
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftD(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_d
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftQ(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_q
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftE(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_e
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlAltW(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_w
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlAltS(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_s
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlAltA(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_a
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlAltD(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_d
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlAltQ(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_q
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlAltE(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_e
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftAltW(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_w
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftAltS(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_s
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftAltA(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_a
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftAltD(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_d
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftAltQ(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_q
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_ShiftAltE(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_e
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftAltW(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_w
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftAltS(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_s
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftAltA(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_a
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftAltD(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_d
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftAltQ(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_q
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyDown_CtrlShiftAltE(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYDOWN
            && event.key.repeat == 0
            && event.key.keysym.sym == SDLK_e
            && (event.key.keysym.mod & KMOD_CTRL  ) != 0
            && (event.key.keysym.mod & KMOD_SHIFT ) != 0
            && (event.key.keysym.mod & KMOD_ALT   ) != 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeW(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_w
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeS(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_s
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeA(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_a
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeD(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_d
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeQ(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_q
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeE(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_e
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeRight(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_RIGHT
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeLeft(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_LEFT
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeUp(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_UP
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool KeyUp_FreeDown(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_KEYUP
            && event.key.keysym.sym == SDLK_DOWN
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool MouseWheel_Free(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_MOUSEWHEEL
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }

  bool MouseMotion_Free(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_MOUSEMOTION
    ;
    // clang-format on
  }

  bool Window_SizeChanged(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_WINDOWEVENT
            && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED
    ;
    // clang-format on
  }

  bool Quit(const SDL_Event &event)
  {
    // clang-format off
    return true

            && event.type == SDL_QUIT
    ;
    // clang-format on
  }
} // namespace keys
