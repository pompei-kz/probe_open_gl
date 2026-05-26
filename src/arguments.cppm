module;

#include <filesystem>

export module arguments;

export class Arguments {
  std::filesystem::path appName_ = "probe_open_gl"; // TODO add getter to it
  std::filesystem::path executableDirectory_;
  std::filesystem::path configDirectory_;// TODO make it to `~/{appName_}/` and add getter to it

public:
  Arguments(int argumentCount, char **arguments) {
    if (argumentCount <= 0 || arguments == nullptr || arguments[0] == nullptr) {
      executableDirectory_ = std::filesystem::current_path();
    } else {
      executableDirectory_ = std::filesystem::path(arguments[0]).parent_path();
      if (executableDirectory_.empty()) {
        executableDirectory_ = std::filesystem::current_path();
      }
    }
  }

  [[nodiscard]] const std::filesystem::path &executableDirectory() const {
    return executableDirectory_;
  }
};
