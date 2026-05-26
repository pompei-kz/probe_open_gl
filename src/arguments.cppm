module;

#include <filesystem>
#include <cstdlib>
#include <string>

export module arguments;

export class Arguments {
  std::string appName_;
  std::filesystem::path executableDirectory_;
  std::filesystem::path configDirectory_;

public:
  Arguments(int argumentCount, char **arguments) {
    if (argumentCount <= 0 || arguments == nullptr || arguments[0] == nullptr) {
      executableDirectory_ = std::filesystem::current_path();
      appName_ = "probe_open_gl";
    } else {
      const std::filesystem::path programPath(arguments[0]);
      appName_ = programPath.stem().string();
      if (appName_.empty()) {
        appName_ = "probe_open_gl";
      }

      executableDirectory_ = programPath.parent_path();
      if (executableDirectory_.empty()) {
        executableDirectory_ = std::filesystem::current_path();
      }
    }

    if (const char *home = std::getenv("HOME"); home != nullptr && home[0] != '\0') {
      configDirectory_ = std::filesystem::path(home) / appName_;
    } else {
      configDirectory_ = std::filesystem::current_path() / appName_;
    }
  }

  [[nodiscard]] const std::string &appName() const {
    return appName_;
  }

  [[nodiscard]] const std::filesystem::path &executableDirectory() const {
    return executableDirectory_;
  }

  [[nodiscard]] const std::filesystem::path &configDirectory() const {
    return configDirectory_;
  }
};
