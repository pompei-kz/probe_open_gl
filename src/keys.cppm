module;

#include <SDL2/SDL.h>
#include <vector>

export module keys;

export namespace keys
{

  /*
   * Key check function names:
   *
   *   [KeyDown|KeyUp|MouseDown|MouseUp]_[Free|Ctrl|Shift|Alt|CtrlShift|CtrlAlt|ShiftAlt|CtrlShiftAlt]<KEY_NAME>
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
    // clang-format off
    return true

            && event.type == SDL_MOUSEBUTTONDOWN
            /* TODO add here check for Primary mouse button  */
            && (event.key.keysym.mod & KMOD_CTRL  ) == 0
            && (event.key.keysym.mod & KMOD_SHIFT ) == 0
            && (event.key.keysym.mod & KMOD_ALT   ) == 0
    ;
    // clang-format on
  }
} // namespace keys
