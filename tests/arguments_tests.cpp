#include <gtest/gtest.h>

import arguments;

#include <cstdlib>
#include <filesystem>
#include <string>

namespace {
  char *mutableArg(std::string &value) {
    return value.data();
  }

  std::filesystem::path expectedConfigDirectory(const std::string &appName) {
    if (const char *home = std::getenv("HOME"); home != nullptr && home[0] != '\0') {
      return std::filesystem::path(home) / appName;
    }
    return std::filesystem::current_path() / appName;
  }

  std::filesystem::path expectedCacheDirectory(const std::string &appName) {
    if (const char *home = std::getenv("HOME"); home != nullptr && home[0] != '\0') {
      return std::filesystem::path(home) / ".cache" / appName;
    }
    return std::filesystem::current_path() / ".cache" / appName;
  }
}

TEST(Arguments, ExtractsExecutableDirectoryAndAppNameFromProgramPath) {
  std::string program = "/tmp/probe_open_gl_test_runner";
  char *argv[] = {mutableArg(program)};

  const Arguments arguments(1, argv);

  EXPECT_EQ(arguments.appName(), "probe_open_gl_test_runner");
  EXPECT_EQ(arguments.executableDirectory(), std::filesystem::path("/tmp"));
  EXPECT_EQ(arguments.configDirectory(), expectedConfigDirectory("probe_open_gl_test_runner"));
  EXPECT_EQ(arguments.cacheDirectory(), expectedCacheDirectory("probe_open_gl_test_runner"));
}

TEST(Arguments, UsesCurrentDirectoryWhenProgramHasNoParentPath) {
  std::string program = "probe_open_gl";
  char *argv[] = {mutableArg(program)};

  const Arguments arguments(1, argv);

  EXPECT_EQ(arguments.appName(), "probe_open_gl");
  EXPECT_EQ(arguments.executableDirectory(), std::filesystem::current_path());
  EXPECT_EQ(arguments.configDirectory(), expectedConfigDirectory("probe_open_gl"));
  EXPECT_EQ(arguments.cacheDirectory(), expectedCacheDirectory("probe_open_gl"));
}

TEST(Arguments, FallsBackWhenArgumentVectorIsMissing) {
  const Arguments arguments(0, nullptr);

  EXPECT_EQ(arguments.appName(), "probe_open_gl");
  EXPECT_EQ(arguments.executableDirectory(), std::filesystem::current_path());
  EXPECT_EQ(arguments.configDirectory(), expectedConfigDirectory("probe_open_gl"));
  EXPECT_EQ(arguments.cacheDirectory(), expectedCacheDirectory("probe_open_gl"));
}

TEST(Arguments, FallsBackWhenProgramNameIsEmpty) {
  std::string program;
  char *argv[] = {mutableArg(program)};

  const Arguments arguments(1, argv);

  EXPECT_EQ(arguments.appName(), "probe_open_gl");
  EXPECT_EQ(arguments.executableDirectory(), std::filesystem::current_path());
  EXPECT_EQ(arguments.configDirectory(), expectedConfigDirectory("probe_open_gl"));
  EXPECT_EQ(arguments.cacheDirectory(), expectedCacheDirectory("probe_open_gl"));
}
