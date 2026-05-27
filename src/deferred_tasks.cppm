module;

#include <atomic>
#include <chrono>
#include <functional>
#include <utility>
#include <vector>
import sync_map;

export module deferred_tasks;

export class DeferredTasks {

  std::atomic<int64_t> nextTaskId_;

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

  DeferredTasks &operator=(DeferredTasks &&other) noexcept {
    if (this != &other) {
      tasks_ = std::move(other.tasks_);
      other.tasks_.clear();
    }
    return *this;
  }

  ~DeferredTasks() {
  }

  void add(std::function<void()> task, std::chrono::milliseconds runAfterDuration) {

  }

  void clear() {
    tasks_.clear();
  }
};
