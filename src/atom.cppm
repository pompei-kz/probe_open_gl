module;

#include <stdexcept>
#include <string>
#include <string_view>

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

    throw std::runtime_error("atom::byName(): unknown atom name: " + name);
  }
} // namespace atom
