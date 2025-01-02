// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBCFile.h>
#include <noggit/Log.h>
#include <noggit/application/NoggitApplication.hpp>
#include <noggit/project/CurrentProject.hpp>
#include <ClientFile.hpp>

#include <string>
#include <QSettings>
#include <QDir>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <cstring>


template<typename T> inline
auto write(std::ostream& stream, T const& val) -> void
{
  stream.write(reinterpret_cast<char const*>(&val), sizeof(T));
}

DBCFile::DBCFile(const std::string& _filename)
  : filename(_filename)
{
}

void DBCFile::open(std::shared_ptr<BlizzardArchive::ClientData> clientData)
{
  BlizzardArchive::ClientFile f(filename, clientData.get());

  if (f.isEof())
  {
    LogError << "The DBC file \"" << filename << "\" could not be opened. This application may crash soon as the file is most likely needed." << std::endl;
    return;
  }
  LogDebug << "Opening DBC \"" << filename << "\"" << std::endl;

  char header[4];

  f.read(header, 4); // Number of records
  assert(header[0] == 'W' && header[1] == 'D' && header[2] == 'B' && header[3] == 'C');
  f.read(&recordCount, 4);
  f.read(&fieldCount, 4);
  f.read(&recordSize, 4);
  f.read(&stringSize, 4);

  if (!fieldCount || !recordSize)
  {
    throw std::logic_error("DBC error, field count or record size is 0 : " + filename);
  }

  if (fieldCount * 4 != recordSize)
  {
    throw std::logic_error("non four-byte-columns not supported : " + filename);
  }

  data.resize(recordSize * recordCount);
  f.read(data.data(), data.size());

  stringTable.resize(stringSize);
  f.read(stringTable.data(), stringTable.size());

  f.close();
}

void DBCFile::save()
{
  QString str = QString(Noggit::Project::CurrentProject::get()->ProjectPath.c_str());
  if (!(str.endsWith('\\') || str.endsWith('/')))
  {
    str += "/";
  }

  std::string filename_proj = BlizzardArchive::ClientData::normalizeFilenameUnix(str.toStdString() + filename);
  QDir dir(str + "/DBFilesClient/");
  if (!dir.exists())
    dir.mkpath(".");

  std::ofstream stream(filename_proj, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

  stream << 'W' << 'D' << 'B' << 'C';


  write(stream, recordCount);
  write(stream, fieldCount);
  write(stream, recordSize);
  write(stream, stringSize);

  stream.write(reinterpret_cast<char*>(data.data()), data.size());
  stream.write(stringTable.data(), stringSize);
  stream.close();
}

void DBCFile::overwriteWith(DBCFile const& file)
{
  filename = file.filename;
  recordSize = file.recordSize;
  recordCount = file.recordCount;
  fieldCount = file.fieldCount;
  stringSize = file.stringSize;
  data = file.data;
  stringTable = file.stringTable;
}

DBCFile DBCFile::createNew(std::string filename, std::uint32_t fieldCount, std::uint32_t recordSize)
{
  DBCFile file{};
  file.filename = std::move(filename);
  file.recordSize = recordSize;
  file.fieldCount = fieldCount;
  return file;
}

DBCFile::Record DBCFile::addRecord(size_t id, size_t id_field)
{
  assert(recordSize > 0);
  assert(id_field < fieldCount);

  for (Iterator i = begin(); i != end(); ++i)
  {
    if (i->getUInt(id_field) == id)
      throw AlreadyExists();
  }

  size_t old_size = data.size();
  data.resize(old_size + recordSize);
  *reinterpret_cast<unsigned int*>(data.data() + old_size + id_field * sizeof(std::uint32_t)) = static_cast<unsigned int>(id);

  recordCount++;

  return Record(*this, data.data() + old_size);
}

DBCFile::Record DBCFile::addRecordCopy(size_t id, size_t id_from, size_t id_field)
{
  recordCount++;

  bool from_found = false;
  size_t from_idx = 0;

  for (Iterator i = begin(); i != end(); ++i)
  {
    if (i->getUInt(id_field) == id)
      throw AlreadyExists();

    if (i->getUInt(id_field) == id_from)
    {
      from_found = true;
    }

    if (!from_found)
    {
      from_idx++;
    }
  }

  if (!from_found)
  {
    throw NotFound();
  }

  size_t old_size = data.size();
  data.resize(old_size + recordSize);

  Record record_from = getRecord(from_idx);
  std::copy(data.data() + from_idx * recordSize, data.data() + from_idx * recordSize + recordSize, data.data() + old_size);
  *reinterpret_cast<unsigned int*>(data.data() + old_size + id_field * sizeof(std::uint32_t)) = static_cast<unsigned int>(id);

  return Record(*this, data.data() + old_size);
}

void DBCFile::removeRecord(size_t id, size_t id_field)
{
  if (recordCount == 0)
  {
    throw NotFound();
  }

  size_t row_counter = 0;

  for (Iterator i = begin(); i != end(); ++i)
  {
    if (i->getUInt(id_field) == id)
    {
      size_t initial_size = data.size();

      size_t row_position = row_counter * recordSize; // position of the record to remove

      size_t datasizeafterRow = recordSize * (recordCount - row_counter); // size of the data after the row that needs to be moved at the old row's position

      // assert(initial_size >= (datasizeafterRow + row_position));
      if ((row_position + datasizeafterRow) > initial_size)
      {
        throw std::out_of_range("Attempting to remove more data than available");
      }

      // size_t numRecordsToMove = recordCount - row_counter; // Number of records to move down

      unsigned char* record = data.data() + row_position; // data to remove at position

      // Move all data after the row to the row's position
      // only do it if it wasn't the last row
      if (row_position + recordSize < initial_size)
      {
        assert(row_counter < recordCount);
        std::memmove(record, record + recordSize, datasizeafterRow);
      }
      data.resize(initial_size - recordSize);

      recordCount--;
      return;
    }

    row_counter++;

  }

  throw NotFound();

}

int DBCFile::getEmptyRecordID(size_t id_field)
{

  unsigned int id = 0;

  for (Iterator i = begin(); i != end(); ++i)
  {
    id = std::max(i->getUInt(id_field), id);
  }

  return static_cast<int>(++id);
}



