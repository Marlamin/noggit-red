// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once
#include <noggit/ModelHeaders.h>
#include <math/interpolation.hpp>
#include <cassert>
#include <map>
#include <vector>
#include <memory>
#include <type_traits>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <ClientFile.hpp>

namespace Animation
{
  namespace Interpolation
  {
    //! \todo C++0x: Change namespace to "enum class Type : int16_t", remove typedef.
    namespace Type
    {
      typedef int16_t Type_t;
      enum
      {
        NONE,
        LINEAR,
        HERMITE
      };
    }
  }

  template<class FROM, class TO>
  struct Conversion
  {
    inline TO operator()(const FROM& value)
    {
      return TO(value);
    }
  };

  template<>
  inline glm::quat Conversion<packed_quaternion, glm::quat>::operator()(const packed_quaternion& value)
  {
    //! \todo Check if this is really correct.
    return glm::quat(
      static_cast<float>((value.w > 0 ? value.w - 32767 : value.w + 32767) / 32767.0f),
      static_cast<float>((value.x > 0 ? value.x - 32767 : value.x + 32767) / 32767.0f),
      static_cast<float>((value.y > 0 ? value.y - 32767 : value.y + 32767) / 32767.0f),
      static_cast<float>((value.z > 0 ? value.z - 32767 : value.z + 32767) / 32767.0f));
  }

  template<>
  inline float Conversion<int16_t, float>::operator()(const int16_t& value)
  {
    return value / 32767.0f;
  }

  //! \note AnimatedType is the type of data getting animated.
  //! \note DataType is the type of data stored.
  //! \note The conversion from DataType to AnimatedType is done via Animation::Conversion.
  template<class AnimatedType, class DataType = AnimatedType>
  class M2Value
  {
  private:
    typedef uint32_t TimestampType;
    typedef uint32_t AnimationIdType;

    typedef std::vector<AnimatedType> AnimatedTypeVectorType;
    typedef std::vector<TimestampType> TimestampTypeVectorType;

    Animation::Conversion<DataType, AnimatedType> _conversion;

    static const int32_t NO_GLOBAL_SEQUENCE = -1;
    int32_t _globalSequenceID;
    int32_t* _globalSequences;

    Animation::Interpolation::Type::Type_t _interpolationType;

    std::map<AnimationIdType, TimestampTypeVectorType> times;
    std::map<AnimationIdType, AnimatedTypeVectorType> data;

    // for nonlinear interpolations:
    std::map<AnimationIdType, AnimatedTypeVectorType> in;
    std::map<AnimationIdType, AnimatedTypeVectorType> out;

  public:
    bool uses(AnimationIdType anim)
    {
      if (_globalSequenceID != NO_GLOBAL_SEQUENCE)
      {
        anim = AnimationIdType();
      }

      return !data[anim].empty();
    }

    AnimatedType getValue (AnimationIdType anim, TimestampType time, int animtime)
    {
      if (_globalSequenceID != NO_GLOBAL_SEQUENCE)
      {
        if (_globalSequences[_globalSequenceID])
        {
          time = animtime % _globalSequences[_globalSequenceID];
        }
        else
        {
          time = TimestampType();
        }
        anim = AnimationIdType();
      }

      TimestampTypeVectorType& timestampVector = times[anim];
      AnimatedTypeVectorType& dataVector = data[anim];
      AnimatedTypeVectorType& inVector = in[anim];
      AnimatedTypeVectorType& outVector = out[anim];

      if (dataVector.empty())
      {
        return AnimatedType();
      }

      AnimatedType result = dataVector[0];

      if (!timestampVector.empty())
      {
        TimestampType max_time = timestampVector.back();
        if (max_time > 0)
        {
          time %= max_time;
        }
        else
        {
          time = TimestampType();
        }

        size_t pos = 0;
        for (size_t i = 0; i < timestampVector.size() - 1; ++i)
        {
          if (time >= timestampVector[i] && time < timestampVector[i + 1])
          {
            pos = i;
            break;
          }
        }

        if (pos == timestampVector.size() - 1 || _interpolationType == Animation::Interpolation::Type::NONE)
        {
          result = dataVector[pos];
        }
        else
        {

          TimestampType t1 = timestampVector[pos];
          TimestampType t2 = timestampVector[pos + 1];
          const float percentage = (time - t1) / static_cast<float>(t2 - t1);

          switch (_interpolationType)
          {
          case Animation::Interpolation::Type::LINEAR:
          {
            //result = math::interpolation::linear (percentage, dataVector[pos], dataVector[pos + 1]);
            if constexpr (std::is_same_v<AnimatedType, glm::quat>)
            {
              result = glm::slerp(dataVector[pos], dataVector[pos + 1], percentage);
            }
            else
            {
              result = glm::mix(dataVector[pos], dataVector[pos + 1], percentage);
            }
           
          }
            break;

          case Animation::Interpolation::Type::HERMITE:
          {
            result = math::interpolation::hermite(percentage, dataVector[pos], dataVector[pos + 1], inVector[pos], outVector[pos]);
          }
            break;
          }
        }
      }

      return result;
    }

    //! \todo Use a vector of BlizzardArchive::ClientFile& for the anim files instead for safety.
    M2Value (const AnimationBlock& animationBlock
             , const BlizzardArchive::ClientFile& file
             , int32_t* globalSequences
             , const std::vector<std::unique_ptr<BlizzardArchive::ClientFile>>& animation_files
             = std::vector<std::unique_ptr<BlizzardArchive::ClientFile>>())
    {
      assert(animationBlock.nTimes == animationBlock.nKeys);

      _interpolationType = animationBlock.type;

      _globalSequences = globalSequences;
      _globalSequenceID = animationBlock.seq;
      if (_globalSequenceID != NO_GLOBAL_SEQUENCE)
      {
        assert(_globalSequences && "Animation said to have global sequence, but pointer to global sequence data is nullptr");
      }

      const AnimationBlockHeader* timestampHeaders = file.get<AnimationBlockHeader>(animationBlock.ofsTimes);
      const AnimationBlockHeader* keyHeaders = file.get<AnimationBlockHeader>(animationBlock.ofsKeys);

      for (std::uint32_t j = 0; j < animationBlock.nTimes; ++j)
      {
        const TimestampType* timestamps = j < animation_files.size() && animation_files[j] ?
          animation_files[j]->get<TimestampType>(timestampHeaders[j].ofsEntries) :
          file.get<TimestampType>(timestampHeaders[j].ofsEntries);

        for (std::uint32_t i = 0; i < timestampHeaders[j].nEntries; ++i)
        {
          times[j].push_back(timestamps[i]);
        }
      }

      for (std::uint32_t j = 0; j < animationBlock.nKeys; ++j)
      {
        const DataType* keys = j < animation_files.size() && animation_files[j] ?
          animation_files[j]->get<DataType>(keyHeaders[j].ofsEntries) :
          file.get<DataType>(keyHeaders[j].ofsEntries);

        switch (_interpolationType)
        {
        case Animation::Interpolation::Type::NONE:
        case Animation::Interpolation::Type::LINEAR:
          for (std::uint32_t i = 0; i < keyHeaders[j].nEntries; ++i)
          {
            data[j].push_back(_conversion(keys[i]));
          }
          break;

        case Animation::Interpolation::Type::HERMITE:
          for (std::uint32_t i = 0; i < keyHeaders[j].nEntries; ++i)
          {
            data[j].push_back(_conversion(keys[i * 3]));
            in[j].push_back(_conversion(keys[i * 3 + 1]));
            out[j].push_back(_conversion(keys[i * 3 + 2]));
          }
          break;
        }
      }
    }

    void apply(AnimatedType function(const AnimatedType))
    {
      switch (_interpolationType)
      {
      case Animation::Interpolation::Type::NONE:
      case Animation::Interpolation::Type::LINEAR:
        for (std::uint32_t i = 0; i < data.size(); ++i)
        {
          for (std::uint32_t j = 0; j < data[i].size(); ++j)
          {
            data[i][j] = function(data[i][j]);
          }
        }
        break;

      case Animation::Interpolation::Type::HERMITE:
        for (std::uint32_t i = 0; i < data.size(); ++i)
        {
          for (std::uint32_t j = 0; j < data[i].size(); ++j)
          {
            data[i][j] = function(data[i][j]);
            in[i][j] = function(in[i][j]);
            out[i][j] = function(out[i][j]);
          }
        }
        break;
      }
    }
  };
};
