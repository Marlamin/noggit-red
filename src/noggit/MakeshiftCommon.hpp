#ifndef NOGGIT_SRC_NOGGIT_MAKESHIFTCOMMON_HPP
#define NOGGIT_SRC_NOGGIT_MAKESHIFTCOMMON_HPP

#include <concepts>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace noggit::Recovery
{
  struct ChunkHeader
  {
    ChunkHeader
    (
      char const* magic,
      std::uint32_t size = 0
    );

    std::uint32_t magic;
    std::uint32_t size;
  };

  class Buffer
  {
    public:
      enum struct SeekMode : bool
      {
        Absolute,
        Relative
      };

      class CommonAnchor
      {
        protected:
          friend Buffer;

          virtual
          auto _setPtr ( void* ptr )
          -> void = 0;
      };

      template < typename This >
      class Anchor : protected CommonAnchor
      {
        public:
          template < std::convertible_to<This> Other >
          Anchor ( Anchor<Other> const& other );
          Anchor ( Anchor<This>&& other )
          = delete;
          ~Anchor ( );
          template < std::convertible_to<This> Other >
          auto operator = ( Anchor<Other> const& other )
          -> Anchor<This>&;
          auto operator = ( Anchor<This>&& other )
          -> Anchor& = delete;
          auto operator * ( )
          -> This&;
          auto operator * ( ) const
          -> This const&;
          auto operator [] ( std::size_t idx )
          -> This&;
          auto operator [] ( std::size_t idx ) const
          -> This const&;
          auto operator -> ( )
          -> This*;
          auto operator -> ( ) const
          -> This const*;
          operator This* ( );
          operator This const* ( ) const;
        protected:
          friend class Buffer;

          explicit constexpr
          Anchor
          (
            This* ptr,
            Buffer* buf
          );
          auto _setPtr ( void* ptr )
          -> void override;

          This* _ptr;
          Buffer* _buf;
      };

      Buffer ( );
      explicit
      Buffer ( std::string_view file );
      auto save ( std::string_view file )
      -> void;
      template < SeekMode mode = SeekMode::Absolute >
      auto seek ( std::size_t val )
      -> std::size_t;
      template < typename Ty >
      auto read ( Ty* ptr )
      -> void;
      auto read
      (
        void* ptr,
        std::size_t amount
      )
      -> void;
      template < typename Ty >
      auto append ( Ty const& val )
      -> void;
      auto append
      (
        void const* mem,
        std::size_t size
      )
      -> void;
      auto extend ( std::size_t amount )
      -> void;
      auto getPos ( ) const
      -> std::size_t;
      auto getData ( ) const
      -> std::vector<char> const&;
      template < typename Ty >
      constexpr
      operator Anchor<Ty> ( );
    protected:
      template < typename Ty >
      friend class Anchor;

      auto _tether
      (
        CommonAnchor* who,
        void const* ptr
      )
      -> void;
      auto _copyTether
      (
        CommonAnchor const& from,
        CommonAnchor* to
      )
      -> void;
      auto _untether ( CommonAnchor* who )
      -> void;
      auto _extend ( std::size_t amount )
      -> void;
      std::size_t _pos;
      std::vector<char> _data;
      std::unordered_map<CommonAnchor*, std::size_t> _anchors;
  };
}

#endif //NOGGIT_SRC_NOGGIT_MAKESHIFTCOMMON_HPP
