// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <memory>
#include <blizzard-archive-library/include/ClientData.hpp>

class DBCFile
{
public:
  explicit DBCFile(const std::string& filename);

  // Open database. It must be openened before it can be used.
  void open(std::shared_ptr<BlizzardArchive::ClientData> clientData);
  void save();

  class NotFound : public std::runtime_error
  {
  public:
    NotFound() : std::runtime_error("Key was not found.")
    { }
  };

  class AlreadyExists : public std::runtime_error
  {
  public:
    AlreadyExists() : std::runtime_error("Key already exists.")
    { }
  };

  class Iterator;
  class Record
  {
  public:
     const float& getFloat(size_t field) const
    {
      assert(field < file.fieldCount);
      return *reinterpret_cast<float*>(offset + field * 4);
    }
    const unsigned int& getUInt(size_t field) const
    {
      assert(field < file.fieldCount);
      return *reinterpret_cast<unsigned int*>(offset + field * 4);
    }
    const int& getInt(size_t field) const
    {
      assert(field < file.fieldCount);
      return *reinterpret_cast<int*>(offset + field * 4);
    }
    const char *getString(size_t field) const
    {
      assert(field < file.fieldCount);
      size_t stringOffset = getUInt(field);
      assert(stringOffset < file.stringSize);
      return file.stringTable.data() + stringOffset;
    }
    const char *getLocalizedString(size_t field, int locale = -1) const
    {
      int loc = locale;
      if (locale == -1)
      {
        assert(field < file.fieldCount - 8);
        for (loc = 0; loc < 15; loc++)
        {
          size_t stringOffset = getUInt(field + loc);
          if (stringOffset != 0)
            break;
        }
      }

      assert(field + loc < file.fieldCount);
      size_t stringOffset = getUInt(field + loc);
      assert(stringOffset < file.stringSize);
      return file.stringTable.data() + stringOffset;
    }

    template<typename T> inline
    void write(size_t field, T val)
    {
      static_assert(sizeof(T) == 4, "This function only writes int/uint/float values.");
      assert(field < file.fieldCount);
      *reinterpret_cast<T*>(offset + field * 4) = val;
    }

    void writeString(size_t field, const std::string& val)
    {
      assert(field < file.fieldCount);

      if (!val.size())
      {
        *reinterpret_cast<unsigned int*>(offset + field * 4) = 0;
        return;
      }

      size_t old_size = file.stringTable.size();
      *reinterpret_cast<unsigned int*>(offset + field * 4) = file.stringTable.size();
      file.stringTable.resize(old_size + val.size() + 1);
      std::copy(val.c_str(), val.c_str() + val.size() + 1, file.stringTable.data() + old_size);
      file.stringSize += val.size() + 1;
    }

    void writeLocalizedString(size_t field, const std::string& val, int locale)
    {
      assert(field < file.fieldCount);

      if (!val.size())
      {
        *reinterpret_cast<unsigned int*>(offset + ((field + locale) * 4)) = 0;
        return;
      }

      size_t old_size = file.stringTable.size();
      *reinterpret_cast<unsigned int*>(offset + ((field + locale) * 4)) = file.stringTable.size();
      file.stringTable.resize(old_size + val.size() + 1);
      std::copy(val.c_str(), val.c_str() + val.size() + 1, file.stringTable.data() + old_size);
      file.stringSize += val.size() + 1;
    }

  private:
    Record(DBCFile &pfile, unsigned char *poffset) : file(pfile), offset(poffset) {}
    DBCFile &file;
    unsigned char *offset;

    friend class DBCFile;
    friend class DBCFile::Iterator;
  };
  /** Iterator that iterates over records
  */
  class Iterator
  {
  public:
    Iterator(DBCFile &file, unsigned char *offset) :
      record(file, offset) {}
    /// Advance (prefix only)
    Iterator & operator++() {
      record.offset += record.file.recordSize;
      return *this;
    }
    /// Return address of current instance
    Record & operator*() { return record; }
    Record* operator->() {
      return &record;
    }
    /// Comparison
    bool operator==( Iterator const &b)  const
    {
      return record.offset == b.record.offset;
    }
  private:
    Record record;
  };

  inline Record getRecord(size_t id)
  {
    return Record(*this, data.data() + id*recordSize);
  }

  inline Iterator begin()
  {
    return Iterator(*this, data.data());
  }
  inline Iterator end()
  {
    return Iterator(*this, data.data() + data.size());
  }

  inline size_t getRecordCount()  { return recordCount; }
  inline size_t getFieldCount()  { return fieldCount; }
  inline Record getByID(unsigned int id, size_t field = 0)
  {
    for (Iterator i = begin(); i != end(); ++i)
    {
      if (i->getUInt(field) == id)
        return (*i);
    }
    throw NotFound();
  }
  inline bool CheckIfIdExists(unsigned int id, size_t field = 0)
  {
      for (Iterator i = begin(); i != end(); ++i)
      {
          if (i->getUInt(field) == id)
              return (true);
      }
      return (false);
  }
  inline int getRecordRowId(unsigned int id, size_t field = 0)
  {
      int row_id = 0;
      for (Iterator i = begin(); i != end(); ++i)
      {
          if (i->getUInt(field) == id)
              return row_id;

          row_id++;
      }
      throw NotFound();
  }

  Record addRecord(size_t id, size_t id_field = 0);
  Record addRecordCopy(size_t id, size_t id_from, size_t id_field = 0);
  void removeRecord(size_t id, size_t id_field = 0);
  int getEmptyRecordID(size_t id_field = 0);

private:
  std::string filename;
  std::uint32_t recordSize;
  std::uint32_t recordCount;
  std::uint32_t fieldCount;
  std::uint32_t stringSize;
  std::vector<unsigned char> data;
  std::vector<char> stringTable;
};
