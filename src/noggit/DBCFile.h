// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <blizzard-archive-library/include/ClientData.hpp>

class DBCFile
{
public:
  explicit DBCFile(const std::string& filename);

  // Open database. It must be openened before it can be used.
  void open(std::shared_ptr<BlizzardArchive::ClientData> clientData);
  void save();

  void overwriteWith(DBCFile const& file);

  static DBCFile createNew(std::string filename, std::uint32_t fieldCount, std::uint32_t recordSize);

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
     const float& getFloat(size_t field) const;

    const unsigned int& getUInt(size_t field) const;

    const int& getInt(size_t field) const;

    const char *getString(size_t field) const;

    const char *getLocalizedString(size_t field, int locale = -1) const;

    template<typename T> inline
    void write(size_t field, T val)
    {
      static_assert(sizeof(T) == 4, "This function only writes int/uint/float values.");
      assert(field < file.fieldCount);
      *reinterpret_cast<T*>(offset + field * 4) = val;
    }

    void writeString(size_t field, const std::string& val);

    void writeLocalizedString(size_t field, const std::string& val, unsigned int locale);

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

  Record getRecord(size_t id);

  Iterator begin();
  Iterator end();

  size_t getRecordCount() const;
  size_t getFieldCount() const;
  size_t getRecordSize() const;

  Record getByID(unsigned int id, size_t field = 0);
  bool CheckIfIdExists(unsigned int id, size_t field = 0);
  int getRecordRowId(unsigned int id, size_t field = 0);

  Record addRecord(size_t id, size_t id_field = 0);
  Record addRecordCopy(size_t id, size_t id_from, size_t id_field = 0);
  void removeRecord(size_t id, size_t id_field = 0);
  int getEmptyRecordID(size_t id_field = 0);

private:
  DBCFile() = default;

  std::string filename;
  std::uint32_t recordSize = 0;
  std::uint32_t recordCount = 0;
  std::uint32_t fieldCount = 0;
  std::uint32_t stringSize = 0;
  std::vector<unsigned char> data;
  std::vector<char> stringTable;
};
