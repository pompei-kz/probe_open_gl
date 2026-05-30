module;

#include <string>

export module atom;

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
    if (name == "Oxygen")
    {
      return Oxygen;
    }
    if (name == "Nitrogen")
    {
      return Nitrogen;
    }
    if (name == "Carbon")
    {
      return Carbon;
    }

    return Unknown;
  }
} // namespace atom
