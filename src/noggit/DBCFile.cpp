// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#include <noggit/DBCFile.h>
#include <noggit/Log.h>
#include <noggit/application/NoggitApplication.hpp>
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
{}

void DBCFile::open()
{
  BlizzardArchive::ClientFile f (filename, NOGGIT_APP->clientData());

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

  if (fieldCount * 4 != recordSize)
  {
    throw std::logic_error ("non four-byte-columns not supported");
  }

  data.resize (recordSize * recordCount);
  f.read (data.data(), data.size());

  stringTable.resize (stringSize);
  f.read (stringTable.data(), stringTable.size());

  f.close();
}

void DBCFile::save()
{
  QSettings app_settings;
  QString str = app_settings.value ("project/path").toString();
  if (!(str.endsWith('\\') || str.endsWith('/')))
  {
    str += "/";
  }

  std::string filename_proj = BlizzardArchive::ClientData::normalizeFilenameInternal(str.toStdString() + filename);
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

DBCFile::Record DBCFile::addRecord(size_t id, size_t id_field)
{
  recordCount++;

  for (Iterator i = begin(); i != end(); ++i)
  {
    if (i->getUInt(id_field) == id)
      throw AlreadyExists();
  }

  size_t old_size = data.size();
  data.resize(old_size + recordSize);
  *reinterpret_cast<unsigned int*>(data.data() + old_size + id_field * sizeof(std::uint32_t)) = id;

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
  *reinterpret_cast<unsigned int*>(data.data() + old_size + id_field * sizeof(std::uint32_t)) = id;

  return Record(*this, data.data() + old_size);
}

void DBCFile::removeRecord(size_t id, size_t id_field)
{
  recordCount--;
  size_t counter = 0;

  for (Iterator i = begin(); i != end(); ++i)
  {
    if (i->getUInt(id_field) == id)
    {
      size_t initial_size = data.size();

      unsigned char* record = data.data() + counter * recordSize;
      std::memmove(record, record + recordSize, recordSize * (recordCount - counter + 1));
      data.resize(initial_size - recordSize);
      return;
    }

    counter++;

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



