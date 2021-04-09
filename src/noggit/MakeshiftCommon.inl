#ifndef NOGGIT_SRC_NOGGIT_MAKESHIFTCOMMON_INL
#define NOGGIT_SRC_NOGGIT_MAKESHIFTCOMMON_INL

#include <algorithm>
#include <cassert>
#include "MakeshiftCommon.hpp"

namespace noggit::Recovery
{
  template < typename This >
  template < std::convertible_to<This> Other >
  Buffer::Anchor<This>::Anchor ( Anchor<Other> const& other )
  : _buf{other._buf}
  { _buf->_copyTether(other, this); }

  template < typename This >
  Buffer::Anchor<This>::~Anchor ( )
  { _buf->_untether(this); }

  template < typename This >
  template < std::convertible_to<This> Other >
  auto Buffer::Anchor<This>::operator = ( Anchor<Other> const& other )
  -> Anchor<This>&
  {
    _buf->_untether(this);
    _buf = other._buf;
    _buf->_copyTether(other, this);
  }

  template < typename This >
  auto Buffer::Anchor<This>::operator * ( )
  -> This&
  { return *_ptr; }

  template < typename This >
  auto Buffer::Anchor<This>::operator * ( ) const
  -> This const&
  { return *_ptr; }

  template < typename This >
  auto Buffer::Anchor<This>::operator [] ( std::size_t idx )
  -> This&
  { return _ptr[idx]; }

  template < typename This >
  auto Buffer::Anchor<This>::operator [] ( std::size_t idx ) const
  -> This const&
  { return _ptr[idx]; }

  template < typename This >
  auto Buffer::Anchor<This>::operator -> ( )
  -> This*
  { return _ptr; }

  template < typename This >
  auto Buffer::Anchor<This>::operator -> ( ) const
  -> This const*
  { return _ptr; }

  template < typename This >
  Buffer::Anchor<This>::operator This* ( )
  { return _ptr; }

  template < typename This >
  Buffer::Anchor<This>::operator This const* ( ) const
  { return _ptr; }

  template < typename This >
  constexpr
  Buffer::Anchor<This>::Anchor
  (
    This* ptr,
    Buffer* buf
  )
  : _ptr{ptr}, _buf{buf}
  { _buf->_tether(this, _ptr); }

  template < typename This >
  auto Buffer::Anchor<This>::_setPtr ( void* ptr )
  -> void
  {
    _ptr = reinterpret_cast<This*>(ptr);
  }

  template < Buffer::SeekMode mode >
  auto Buffer::seek ( std::size_t val )
  -> std::size_t
  {
    if constexpr(mode == SeekMode::Absolute)
      _pos = val;
    else
      _pos += val;

    return _pos;
  }

  template < typename Ty >
  auto Buffer::read ( Ty* ptr )
  -> void
  {
    assert(_pos + sizeof(Ty) < _data.size());
    std::copy(_data.data() + _pos, _data.data() + _pos + sizeof(Ty)
    , reinterpret_cast<char*>(ptr));
    _pos += sizeof(Ty);
  }

  template < typename Ty >
  auto Buffer::append ( Ty const& val )
  -> void
  { append(&val, sizeof(Ty)); }

  template < typename Ty >
  constexpr
  Buffer::operator Anchor<Ty> ( )
  { return Anchor<Ty>{reinterpret_cast<Ty*>(_data.data() + _pos), this}; }
}

#endif //NOGGIT_SRC_NOGGIT_MAKESHIFTCOMMON_INL
