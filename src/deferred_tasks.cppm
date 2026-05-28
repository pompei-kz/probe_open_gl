module;

#include <chrono>
#include <functional>
#include <memory>
#include <string_view>

export module deferred_tasks;

export class DeferredTasks
{
public:
  DeferredTasks();

  DeferredTasks(const DeferredTasks &) = delete;

  DeferredTasks &operator=(const DeferredTasks &) = delete;

  DeferredTasks(DeferredTasks &&other) = delete;

  DeferredTasks &operator=(DeferredTasks &&other) = delete;

  ~DeferredTasks();

  void add(std::chrono::milliseconds runAfterDuration, std::string_view taskName, std::function<void()> task) const;

  void clear() const;

  void idle(std::chrono::system_clock::time_point now) const;

  [[nodiscard]] bool empty() const;

  [[nodiscard]] std::size_t size() const;

private:
  struct Impl;

  std::unique_ptr<Impl> impl_;
};
