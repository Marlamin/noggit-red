// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/AsyncLoader.h>
#include <noggit/AsyncObject.h>
#include <noggit/ContextObject.hpp>
#include <noggit/Log.h>
#include <noggit/MPQ.h>

#include <boost/thread.hpp>

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <QOpenGLContext>

struct pair_hash
{
  template <class T1, class T2>
  std::size_t operator() (const std::pair<T1,T2> &p) const
  {
    auto h1 = std::hash<T1>{}(p.first);
    auto h2 = std::hash<T2>{}(p.second);

    return h1 ^ h2; // use hash combine here
  }
};


namespace noggit
{

  template<typename T>
  struct async_object_multimap_with_normalized_key
  {
    async_object_multimap_with_normalized_key (std::function<std::string (std::string)> normalize = &mpq::normalized_filename)
      : _normalize (std::move (normalize))
    {}

    ~async_object_multimap_with_normalized_key()
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
      T* emplace (std::string const& filename, noggit::NoggitRenderContext context, Args&&... args)
    {
      std::string const normalized (_normalize (filename));
      auto pair = std::make_pair(context, normalized);
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
                                                 , std::forward_as_tuple (normalized, context, args...)
                                                 ).first->second;
                     }()
                   );

      AsyncLoader::instance().queue_for_load(static_cast<AsyncObject*>(obj));

      return obj; 
    }
    void erase (std::string const& filename, noggit::NoggitRenderContext context)
    {
      std::string const normalized (_normalize (filename));
      auto pair = std::make_pair(context, normalized);
      //LogDebug << "Erasing " << normalized << " from context" << context << std::endl;

      AsyncObject* obj = nullptr;

      {
        boost::mutex::scoped_lock lock(_mutex);

        if (--_counts.at(pair) == 0)
        {
          obj = static_cast<AsyncObject*>(&_elements.at(pair));
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
    void apply (std::function<void (std::string const&, T&)> fun)
    {
      boost::mutex::scoped_lock lock(_mutex);

      for (auto& element : _elements)
      {
        fun (element.first.second, element.second);
      }
    }
    void apply (std::function<void (std::string const&, T const&)> fun) const
    {
      boost::mutex::scoped_lock lock(_mutex);
      for (auto const& element : _elements)
      {
        fun (element.first.second, element.second);
      }
    }

    void context_aware_apply(std::function<void (std::string const&, T&)> fun, noggit::NoggitRenderContext context)
    {
      boost::mutex::scoped_lock lock(_mutex);

      for (auto& element : _elements)
      {
        if (element.first.first != context)
          continue;

        fun (element.first.second, element.second);
      }
    }
    void context_aware_apply(std::function<void (std::string const&, T const&)> fun, noggit::NoggitRenderContext context) const
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
    std::map<std::pair<int, std::string>, T> _elements;
    std::unordered_map<std::pair<int, std::string>, std::size_t, pair_hash> _counts;
    std::function<std::string (std::string)> _normalize;
    boost::mutex mutable _mutex;
  };

}
