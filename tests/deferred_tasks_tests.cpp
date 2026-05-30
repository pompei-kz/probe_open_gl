#include <gtest/gtest.h>

import deferred_tasks;

#include <chrono>
#include <string>

namespace {
  using Clock = std::chrono::system_clock;
}

TEST(DeferredTasks, RunsDueTasksOnIdle) {
  int calls = 0;
  const DeferredTasks tasks;
  tasks.add(std::chrono::milliseconds{20}, "task", [&calls] { ++calls; });

  tasks.idle(Clock::now() + std::chrono::milliseconds{40});

  EXPECT_EQ(calls, 1);
  EXPECT_TRUE(tasks.empty());
}

TEST(DeferredTasks, DoesNotRunTasksBeforeDueTime) {
  int calls = 0;
  const DeferredTasks tasks;
  const auto beforeAdd = Clock::now();
  tasks.add(std::chrono::hours{1}, "task", [&calls] { ++calls; });

  tasks.idle(beforeAdd + std::chrono::milliseconds{10});

  EXPECT_EQ(calls, 0);
  EXPECT_EQ(tasks.size(), 1U);
}

TEST(DeferredTasks, RunsTasksOnlyOnce) {
  int calls = 0;
  const DeferredTasks tasks;
  tasks.add(std::chrono::milliseconds{20}, "task", [&calls] { ++calls; });

  tasks.idle(Clock::now() + std::chrono::milliseconds{40});
  tasks.idle(Clock::now() + std::chrono::hours{1});

  EXPECT_EQ(calls, 1);
}

TEST(DeferredTasks, ClearCancelsTasks) {
  int calls = 0;
  const DeferredTasks tasks;
  tasks.add(std::chrono::milliseconds{20}, "task", [&calls] { ++calls; });

  tasks.clear();
  tasks.idle(Clock::now() + std::chrono::hours{1});

  EXPECT_EQ(calls, 0);
  EXPECT_TRUE(tasks.empty());
}

TEST(DeferredTasks, DestructorRunsPendingTasksIgnoringTime) {
  int calls = 0;
  {
    const DeferredTasks tasks;
    tasks.add(std::chrono::hours{1}, "task", [&calls] { ++calls; });
  }

  EXPECT_EQ(calls, 1);
}

TEST(DeferredTasks, RunsMultipleDueTasks) {
  std::string calls;
  const DeferredTasks tasks;
  tasks.add(std::chrono::milliseconds{10}, "task-a", [&calls] { calls += 'a'; });
  tasks.add(std::chrono::milliseconds{20}, "task-b", [&calls] { calls += 'b'; });

  tasks.idle(Clock::now() + std::chrono::seconds{1});

  EXPECT_EQ(calls.size(), 2U);
  EXPECT_NE(calls.find('a'), std::string::npos);
  EXPECT_NE(calls.find('b'), std::string::npos);
}

TEST(DeferredTasks, RunsOnlyTasksWhoseDurationCompleted) {
  std::string calls;
  const DeferredTasks tasks;
  const auto beforeAdd = Clock::now();
  tasks.add(std::chrono::milliseconds{10}, "task-a", [&calls] { calls += 'a'; });
  tasks.add(std::chrono::minutes{1}, "task-b", [&calls] { calls += 'b'; });
  tasks.add(std::chrono::minutes{2}, "task-c", [&calls] { calls += 'c'; });

  tasks.idle(beforeAdd + std::chrono::milliseconds{100});

  EXPECT_EQ(calls, "a");
  EXPECT_EQ(tasks.size(), 2U);

  tasks.idle(beforeAdd + std::chrono::minutes{3});

  EXPECT_EQ(calls.size(), 3U);
  EXPECT_NE(calls.find('a'), std::string::npos);
  EXPECT_NE(calls.find('b'), std::string::npos);
  EXPECT_NE(calls.find('c'), std::string::npos);
  EXPECT_TRUE(tasks.empty());
}

TEST(DeferredTasks, ReplacesTaskWithSameNameWithoutRunningOldTask) {
  std::string calls;
  const DeferredTasks tasks;

  tasks.add(std::chrono::milliseconds{10}, "same-task", [&calls] { calls += 'a'; });
  tasks.add(std::chrono::milliseconds{20}, "same-task", [&calls] { calls += 'b'; });

  tasks.idle(Clock::now() + std::chrono::seconds{1});

  EXPECT_EQ(calls, "b");
  EXPECT_TRUE(tasks.empty());
}
