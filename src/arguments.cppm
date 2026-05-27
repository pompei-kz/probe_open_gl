module;

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

export module arguments;

export class Arguments
{
  std::string           appName_;
  std::filesystem::path exeFolder_;
  std::filesystem::path configFolder_;
  std::filesystem::path cacheFolder_;

public:
  Arguments(int argumentCount, char **arguments)
  {
    if (argumentCount <= 0 || arguments == nullptr || arguments[0] == nullptr)
    {
      exeFolder_ = std::filesystem::current_path();
      appName_   = "probe_open_gl";
    }
    else
    {
      const std::filesystem::path programPath(arguments[0]);
      appName_ = programPath.stem().string();
      if (appName_.empty())
      {
        appName_ = "probe_open_gl";
      }

      exeFolder_ = programPath.parent_path();
      if (exeFolder_.empty())
      {
        exeFolder_ = std::filesystem::current_path();
      }
    }

    if (const char *home = std::getenv("HOME"); home != nullptr && home[0] != '\0')
    {
      configFolder_ = std::filesystem::path(home) / appName_;
      cacheFolder_  = std::filesystem::path(home) / ".cache" / appName_;
    }
    else
    {
      configFolder_ = std::filesystem::current_path() / appName_;
      cacheFolder_  = std::filesystem::current_path() / ".cache" / appName_;
    }
  }

  [[nodiscard]] const std::string &appName() const { return appName_; }

  [[nodiscard]] const std::filesystem::path &exeFolder() const { return exeFolder_; }

  [[nodiscard]] const std::filesystem::path &configFolder() const { return configFolder_; }

  [[nodiscard]] const std::filesystem::path &cacheFolder() const { return cacheFolder_; }

  void printIntro() const
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
};
