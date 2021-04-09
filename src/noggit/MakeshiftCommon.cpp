#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include "MakeshiftCommon.inl"

using namespace noggit::Recovery;

ChunkHeader::ChunkHeader
(
  char const* magic,
  std::uint32_t size
)
: size{size}
{
  assert(std::strlen(magic) == 4);
  std::copy(magic, magic + 4, reinterpret_cast<char*>(&this->magic));
}

Buffer::Buffer ( )
: _pos{}
{ }

Buffer::Buffer ( std::string_view file )
: _pos{}
{
  if
  (
    std::ifstream f{file.data(), std::ios::binary}
    ;
    f
  )
  {
    f.exceptions(std::ios::badbit);
    f.seekg(0, std::ios::end);
    _data.resize(static_cast<std::size_t>(f.tellg()) + 1);
    f.seekg(0);
    f.read(_data.data(), _data.size());
  }
  else
    std::cerr << "E: Unable to open '" << file << "' for reading.\n";
}

auto Buffer::save ( std::string_view file )
-> void
{
  if
  (
    std::ofstream f(file.data(), std::ios::binary | std::ios::trunc)
    ;
    f
  )
  {
    f.exceptions(std::ios::badbit);
    f.write(_data.data(), _data.size());
  }
  else
    std::cerr << "E: Unable to open '" << file << "' for writing.\n";
}

auto Buffer::read
(
  void* ptr,
  std::size_t amount
)
-> void
{
  assert(_pos + amount < _data.size());
  std::copy(_data.data() + _pos, _data.data() + _pos + amount
  , static_cast<char*>(ptr));
  _pos += amount;
}

auto Buffer::append
(
  void const* mem,
  std::size_t size
)
-> void
{
  std::size_t const pos{_pos};
  _extend(size);
  char const* const begin{static_cast<char const*>(mem)};
  std::copy(begin, begin + size, _data.data() + pos);
}

auto Buffer::extend ( std::size_t amount )
-> void
{ _extend(amount); }

auto Buffer::getPos ( ) const
-> std::size_t
{ return _pos; }

auto Buffer::getData ( ) const
-> std::vector<char> const&
{ return _data; }

auto Buffer::_tether
(
  CommonAnchor* who,
  void const* ptr
)
-> void
{
  assert(_anchors.find(who) == _anchors.cend());
  assert(ptr >= _data.data() && ptr <= _data.data() + _data.size());
  _anchors.emplace(who, static_cast<char const*>(ptr) - _data.data());
}

auto Buffer::_copyTether
(
  CommonAnchor const& from,
  CommonAnchor* to
)
-> void
{
  auto itr{_anchors.find(const_cast<CommonAnchor*>(&from))};
  assert(itr != _anchors.end());
  _anchors.emplace(to, itr->second);
  to->_setPtr(_data.data() + itr->second);
}

auto Buffer::_untether ( CommonAnchor* who )
-> void
{
  assert(_anchors.find(who) != _anchors.cend());
  _anchors.erase(who);
}

auto Buffer::_extend ( std::size_t amount )
-> void
{
  std::size_t const req{_pos + amount};

  if(req > _data.size())
  {
    _data.resize(req);

    for(auto& entry : _anchors)
      entry.first->_setPtr(_data.data() + entry.second);
  }

  _pos = req;
}
