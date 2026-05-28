module;

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

module deferred_tasks;

import sync_map;

struct DeferredTasks::Impl
{
  struct Task
  {
    std::function<void()>                 task;
    std::chrono::system_clock::time_point whenToRun;
  };

  SyncMap<std::string, Task> tasks;

  void runAll()
  {
    while (!tasks.empty())
    {
      for (const auto snapshot = tasks.snapshot(); const auto &[taskName, task] : snapshot)
      {
        if (tasks.remove(taskName))
        {
          task.task();
        }
      }
    }
  }
};

DeferredTasks::DeferredTasks()
    : impl_(std::make_unique<Impl>())
{
}

DeferredTasks::~DeferredTasks()
{
  impl_->runAll();
}

void DeferredTasks::add(const std::chrono::milliseconds runAfterDuration, const std::string_view taskName, std::function<void()> task) const
{
  impl_->tasks.remove(std::string(taskName));
  impl_->tasks.put(std::string(taskName),
                   Impl::Task{
                       std::move(task),
                       std::chrono::system_clock::now() + runAfterDuration,
                   });
}

void DeferredTasks::clear() const
{
  impl_->tasks.clear();
}

void DeferredTasks::idle(const std::chrono::system_clock::time_point now) const
{
  for (const auto snapshot = impl_->tasks.snapshot(); const auto &[taskName, task] : snapshot)
  {
    if (task.whenToRun <= now && impl_->tasks.remove(taskName))
    {
      task.task();
    }
  }
}

bool DeferredTasks::empty() const
{
  return impl_->tasks.empty();
}

std::size_t DeferredTasks::size() const
{
  return impl_->tasks.size();
}
