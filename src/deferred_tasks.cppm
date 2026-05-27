module;

#include <chrono>
#include <functional>
#include <string>
#include <string_view>
#include <utility>

export module deferred_tasks;

import sync_map;

export class DeferredTasks {
  struct Task {
    std::function<void()> task;
    std::chrono::system_clock::time_point whenToRun;
  };

  SyncMap<std::string, Task> tasks_;

public:
  DeferredTasks() = default;

  DeferredTasks(const DeferredTasks &) = delete;

  DeferredTasks &operator=(const DeferredTasks &) = delete;

  DeferredTasks(DeferredTasks &&other) = delete;

  DeferredTasks &operator=(DeferredTasks &&other) = delete;

  ~DeferredTasks() {
    runAll();
  }

  void add(const std::chrono::milliseconds runAfterDuration, const std::string_view taskName,
           std::function<void()> task) {
    tasks_.remove(std::string(taskName));
    tasks_.put(std::string(taskName), Task{
                 std::move(task),
                 std::chrono::system_clock::now() + runAfterDuration,
               });
  }

  void clear() {
    tasks_.clear();
  }

  void idle(const std::chrono::system_clock::time_point now) {
    for (
      const auto snapshot = tasks_.snapshot();
      const auto &[taskName, task]: snapshot) {
      if (task.whenToRun <= now && tasks_.remove(taskName)) {
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

private:
  void runAll() {
    while (!tasks_.empty()) {
      for (
        const auto snapshot = tasks_.snapshot();
        const auto &[taskName, task]: snapshot) {
        if (tasks_.remove(taskName)) {
          task.task();
        }
      }
    }
  }
};
