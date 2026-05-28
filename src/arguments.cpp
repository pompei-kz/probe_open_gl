module;

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

module arguments;

struct Arguments::Impl
{
  std::string           appName;
  std::filesystem::path exeFolder;
  std::filesystem::path configFolder;
  std::filesystem::path cacheFolder;
};

Arguments::Arguments(const int argumentCount, char **arguments)
    : impl_(std::make_unique<Impl>())
{
  if (argumentCount <= 0 || arguments == nullptr || arguments[0] == nullptr)
  {
    impl_->exeFolder = std::filesystem::current_path();
    impl_->appName   = "probe_open_gl";
  }
  else
  {
    const std::filesystem::path programPath(arguments[0]);
    impl_->appName = programPath.stem().string();
    if (impl_->appName.empty())
    {
      impl_->appName = "probe_open_gl";
    }

    impl_->exeFolder = programPath.parent_path();
    if (impl_->exeFolder.empty())
    {
      impl_->exeFolder = std::filesystem::current_path();
    }
  }

  if (const char *home = std::getenv("HOME"); home != nullptr && home[0] != '\0')
  {
    impl_->configFolder = std::filesystem::path(home) / impl_->appName;
    impl_->cacheFolder  = std::filesystem::path(home) / ".cache" / impl_->appName;
  }
  else
  {
    impl_->configFolder = std::filesystem::current_path() / impl_->appName;
    impl_->cacheFolder  = std::filesystem::current_path() / ".cache" / impl_->appName;
  }
}

Arguments::Arguments(Arguments &&) noexcept = default;

Arguments &Arguments::operator=(Arguments &&) noexcept = default;

Arguments::~Arguments() = default;

const std::string &Arguments::appName() const
{
  return impl_->appName;
}

const std::filesystem::path &Arguments::exeFolder() const
{
  return impl_->exeFolder;
}

const std::filesystem::path &Arguments::configFolder() const
{
  return impl_->configFolder;
}

const std::filesystem::path &Arguments::cacheFolder() const
{
  return impl_->cacheFolder;
}

void Arguments::printIntro() const
{
  const std::filesystem::path intoPath = exeFolder() / "intro.txt";
  if (std::ifstream introFile(intoPath); introFile)
  {
    std::cout << introFile.rdbuf() << std::endl;
  }
  else
  {
    std::cerr << "VpO9pfn9wt :: Failed to open " << intoPath << std::endl;
  }
}
