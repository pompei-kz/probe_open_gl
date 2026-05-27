module;

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <utility>

export module deferred_tasks;

import sync_map;

export class DeferredTasks {

  std::atomic<int64_t> nextTaskId_{1};

  struct Task {
    std::function<void()> task;
    std::chrono::system_clock::time_point whenToRun;
  };

  SyncMap<int64_t, Task> tasks_;

public:
  DeferredTasks() = default;

  DeferredTasks(const DeferredTasks &) = delete;

  DeferredTasks &operator=(const DeferredTasks &) = delete;

  DeferredTasks(DeferredTasks &&other) = delete;

  DeferredTasks &operator=(DeferredTasks &&other) = delete;

  ~DeferredTasks() = default;

  void add(std::function<void()> task, std::chrono::milliseconds runAfterDuration) {
    const int64_t taskId = nextTaskId_++;
    tasks_.put(taskId, Task{
      std::move(task),
      std::chrono::system_clock::now() + runAfterDuration,
    });
  }

  void clear() {
    tasks_.clear();
  }

  void idle(std::chrono::system_clock::time_point now) {
    const auto snapshot = tasks_.snapshot();
    for (const auto &[taskId, task]: snapshot) {
      if (task.whenToRun <= now && tasks_.remove(taskId)) {
        task.task();
      }
    }
  }

  [[nodiscard]] bool empty() const {
    return tasks_.empty();
  }

  [[nodiscard]] std::size_t size() const {
    return tasks_.size();
  }
};
