// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/AsyncObject.h>
#include <noggit/Log.h>

 AsyncObject::AsyncObject(BlizzardArchive::Listfile::FileKey file_key) : _file_key(std::move(file_key)) {}

[[nodiscard]]
 BlizzardArchive::Listfile::FileKey const& AsyncObject::file_key() const
{
  return _file_key;
}

[[nodiscard]]
 bool AsyncObject::finishedLoading() const
{
  return finished.load();
}

[[nodiscard]]
 bool AsyncObject::loading_failed() const
{
  return _loading_failed;
}

 void AsyncObject::wait_until_loaded()
{
  if (finished.load())
  {
    return;
  }

  std::unique_lock<std::mutex> lock(_mutex);

  _state_changed.wait
  (lock
    , [&]
    {
      return finished.load();
    }
  );
}

 void AsyncObject::error_on_loading()
{
  LogError << "File " << (_file_key.hasFilepath() ? _file_key.filepath() : std::to_string(_file_key.fileDataID()))
    << " could not be loaded" << std::endl;

  _loading_failed = true;
  finished = true;
  _state_changed.notify_all();
}

[[nodiscard]]
 bool AsyncObject::is_required_when_saving() const
{
  return false;
}

[[nodiscard]]
 async_priority AsyncObject::loading_priority() const
{
  return async_priority::medium;
}
