// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <Listfile.hpp>
#include <noggit/Log.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

enum class async_priority : int
{
  high,
  medium,
  low,
  count
};

class AsyncObject
{
private: 
  bool _loading_failed = false;

protected:
  std::atomic<bool> finished = {false};
  std::mutex _mutex;
  std::condition_variable _state_changed;

  BlizzardArchive::Listfile::FileKey _file_key;

  AsyncObject(BlizzardArchive::Listfile::FileKey file_key) : _file_key(std::move(file_key)) {}

public:

  [[nodiscard]]
  BlizzardArchive::Listfile::FileKey const& file_key() const { return _file_key; };

  AsyncObject() = delete;
  virtual ~AsyncObject() = default;

  [[nodiscard]]
  virtual bool finishedLoading() const
  {
    return finished.load();
  }

  [[nodiscard]]
  bool loading_failed() const
  {
    return _loading_failed;
  }

  void wait_until_loaded()
  {
    if (finished.load())
    {
      return;
    }

    std::unique_lock<std::mutex> lock (_mutex);

    _state_changed.wait 
    ( lock
    , [&]
      {
        return finished.load();
      }
    );
  }

  void error_on_loading()
  {
    LogError << "File " <<  (_file_key.hasFilepath() ? _file_key.filepath() : std::to_string(_file_key.fileDataID()))
      << " could not be loaded" << std::endl;

    _loading_failed = true;
    finished = true;
    _state_changed.notify_all();
  }

  [[nodiscard]]
  virtual bool is_required_when_saving() const
  {
    return false;
  }

  [[nodiscard]]
  virtual async_priority loading_priority() const
  {
    return async_priority::medium;
  }

  virtual void finishLoading() = 0;
  virtual void waitForChildrenLoaded() = 0;
};
