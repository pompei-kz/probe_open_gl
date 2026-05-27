#include <gtest/gtest.h>

import deferred_tasks;

#include <string>

TEST(DeferredTasks, RunsTasksInReverseOrderOnDestruction) {
  std::string calls;
  {
    DeferredTasks tasks;
    tasks.add([&calls] { calls += '1'; });
    tasks.add([&calls] { calls += '2'; });
    tasks.add([&calls] { calls += '3'; });
  }

  EXPECT_EQ(calls, "321");
}

TEST(DeferredTasks, RunExecutesTasksOnce) {
  int calls = 0;
  DeferredTasks tasks;
  tasks.add([&calls] { ++calls; });

  tasks.run();
  tasks.run();

  EXPECT_EQ(calls, 1);
  EXPECT_TRUE(tasks.empty());
}

TEST(DeferredTasks, ClearCancelsTasks) {
  int calls = 0;
  {
    DeferredTasks tasks;
    tasks.add([&calls] { ++calls; });
    tasks.clear();
  }

  EXPECT_EQ(calls, 0);
}

TEST(DeferredTasks, MoveTransfersOwnership) {
  int calls = 0;
  {
    DeferredTasks source;
    source.add([&calls] { ++calls; });
    DeferredTasks target = std::move(source);

    EXPECT_TRUE(source.empty());
    EXPECT_EQ(target.size(), 1U);
  }

  EXPECT_EQ(calls, 1);
}
