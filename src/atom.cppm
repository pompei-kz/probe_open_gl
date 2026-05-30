module;

export module atom;
#include <string_view>

export namespace atom
{
  enum Atom
  {
    Unknown,
    Oxygen,
    Nitrogen,
    Carbon,
  };


  Atom byName(const std::string &name)
  {
    // TODO convert name to Atom and return it
    return Unknown;
  }
}
