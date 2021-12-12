// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncLoader.h>
#include <noggit/AsyncObject.h>
#include <noggit/ContextObject.hpp>
#include <noggit/Log.h>

#include <ClientData.hpp>
#include <Listfile.hpp>

#include <boost/thread.hpp>

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <QOpenGLContext>

struct pair_hash
{
  std::size_t operator() (const std::pair<int, BlizzardArchive::Listfile::FileKey> &p) const noexcept
  {
    auto h1 = std::hash<int>{}(p.first);
    auto h2 = std::hash<std::string>{}(p.second.filepath());
    auto h3 = std::hash<int>{}(p.second.fileDataID());

    return h1 ^ h2 ^ h3;
  }
};

namespace Noggit
{

  template<typename T>
  struct AsyncObjectMultimap
  {
    AsyncObjectMultimap() = default;
    ~AsyncObjectMultimap()
    {
      /*
      apply ( [&] (std::string const& key, T const&)
              {
                auto pair = std::make_pair(context, key);
                LogDebug << key << ": " << _counts.at(pair) << std::endl;
              }
            );
      */
    }

    template<typename... Args>
      T* emplace (BlizzardArchive::Listfile::FileKey const& file_key, Noggit::NoggitRenderContext context, Args&&... args)
    {
      auto pair = std::make_pair(context, file_key);
      //LogDebug << "Emplacing " << normalized << " into context" << context << std::endl;

      {
        boost::mutex::scoped_lock const lock(_mutex);

        if ([&] { return _counts[pair]++; }())
        {
          return &_elements.at (pair);
        }
      }
        

      T* const obj ( [&]
                     {
                       boost::mutex::scoped_lock const lock(_mutex);
                       return &_elements.emplace ( std::piecewise_construct
                                                 , std::forward_as_tuple (pair)
                                                 , std::forward_as_tuple (file_key.filepath(), context, args...)
                                                 ).first->second;
                     }()
                   );

      AsyncLoader::instance().queue_for_load(static_cast<AsyncObject*>(obj));

      return obj; 
    }
    void erase (BlizzardArchive::Listfile::FileKey const& file_key, Noggit::NoggitRenderContext context)
    {
      auto pair = std::make_pair(context, file_key);
      //LogDebug << "Erasing " << normalized << " from context" << context << std::endl;

      AsyncObject* obj = nullptr;

      {
        boost::mutex::scoped_lock lock(_mutex);

        if (--_counts.at(pair) == 0)
        {
          obj = static_cast<AsyncObject*>(&(_elements.at(pair)));
        }
      }

      if (obj)
      {
        // always make sure an async object can be deleted before deleting it
        if (!obj->finishedLoading())
        {
          AsyncLoader::instance().ensure_deletable(obj);
        }

        {
          boost::mutex::scoped_lock lock(_mutex);
          _elements.erase(pair);
          _counts.erase(pair);
        }
      }
    }
    void apply (std::function<void (BlizzardArchive::Listfile::FileKey const&, T&)> fun)
    {
      boost::mutex::scoped_lock lock(_mutex);

      for (auto& element : _elements)
      {
        fun (element.first.second, element.second);
      }
    }
    void apply (std::function<void (BlizzardArchive::Listfile::FileKey const&, T const&)> fun) const
    {
      boost::mutex::scoped_lock lock(_mutex);
      for (auto const& element : _elements)
      {
        fun (element.first.second, element.second);
      }
    }

    void context_aware_apply(std::function<void (BlizzardArchive::Listfile::FileKey const&, T&)> fun, Noggit::NoggitRenderContext context)
    {
      boost::mutex::scoped_lock lock(_mutex);

      for (auto& element : _elements)
      {
        if (element.first.first != context)
          continue;

        fun (element.first.second, element.second);
      }
    }
    void context_aware_apply(std::function<void (BlizzardArchive::Listfile::FileKey const&, T const&)> fun, Noggit::NoggitRenderContext context) const
    {
      boost::mutex::scoped_lock lock(_mutex);
      for (auto const& element : _elements)
      {
        if (element.first.first != context)
          continue;

        fun (element.first.second, element.second);
      }
    }

  private:
    std::map<std::pair<int, BlizzardArchive::Listfile::FileKey>, T> _elements;
    std::unordered_map<std::pair<int, BlizzardArchive::Listfile::FileKey>, std::size_t, pair_hash> _counts;
    boost::mutex mutable _mutex;
  };

}
