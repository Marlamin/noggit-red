// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/async_priority.hpp>
#include <Listfile.hpp>

#include <atomic>
#include <condition_variable>
#include <mutex>

class AsyncObject
{
private: 
  bool _loading_failed = false;

protected:
  std::atomic<bool> finished = {false};
  std::mutex _mutex;
  std::condition_variable _state_changed;

  BlizzardArchive::Listfile::FileKey _file_key;

  AsyncObject(BlizzardArchive::Listfile::FileKey file_key);

public:

  [[nodiscard]]
  BlizzardArchive::Listfile::FileKey const& file_key() const;

  AsyncObject() = delete;
  virtual ~AsyncObject() = default;

  [[nodiscard]]
  virtual bool finishedLoading() const;

  [[nodiscard]]
  bool loading_failed() const;

  void wait_until_loaded();

  void error_on_loading();

  [[nodiscard]]
  virtual bool is_required_when_saving() const;

  [[nodiscard]]
  virtual async_priority loading_priority() const;

  virtual void finishLoading() = 0;
  virtual void waitForChildrenLoaded() = 0;
};
