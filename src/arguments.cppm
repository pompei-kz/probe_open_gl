module;

#include <filesystem>
#include <memory>
#include <string>

export module arguments;

export class Arguments
{
  struct Impl;
  std::unique_ptr<Impl> impl_;

public:
  Arguments(int argumentCount, char **arguments);

  Arguments(const Arguments &) = delete;

  Arguments &operator=(const Arguments &) = delete;

  Arguments(Arguments &&) noexcept;

  Arguments &operator=(Arguments &&) noexcept;

  ~Arguments();

  [[nodiscard]] const std::string &appName() const;

  [[nodiscard]] const std::filesystem::path &exeFolder() const;

  [[nodiscard]] const std::filesystem::path &configFolder() const;

  [[nodiscard]] const std::filesystem::path &cacheFolder() const;

  void printIntro() const;
};
