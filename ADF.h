#pragma once

#include <vector>
#include <string>

class ADF {
public:

  static constexpr int SECTOR_SIZE = 512;
  static constexpr int BLOCK_SIZE = 512;
  static constexpr int SECTORS_PER_TRACK = 11;
  static constexpr int TRACKS_PER_CYLINDER = 2;
  static constexpr int NUM_CYLINDERS = 80;
  static constexpr int DISK_SIZE = SECTOR_SIZE * SECTORS_PER_TRACK * TRACKS_PER_CYLINDER * NUM_CYLINDERS;
  static constexpr int HASHTABLE_SIZE = 72;
  static constexpr int NUM_SECTORS = SECTORS_PER_TRACK * TRACKS_PER_CYLINDER * NUM_CYLINDERS;
  static constexpr int ROOT_BLOCK = 880;
  static constexpr DWORD T_HEADER = 2;
  static constexpr DWORD T_LIST = 16;
  static constexpr DWORD ST_FILE = -3;
  static constexpr DWORD ST_ROOT = 1;
  static constexpr DWORD ST_USERDIR = 2;
  static constexpr DWORD ST_LINKFILE = -4;
  static constexpr DWORD ST_LINKDIR = 4;

  static WORD Little(WORD w) {
    return ((w & 0xFF00) >> 8)
         | ((w & 0x00FF) << 8);
  }

  static DWORD Little(DWORD dw) {
    return ((dw & 0xFF000000) >> 24)
         | ((dw & 0x00FF0000) >>  8)
         | ((dw & 0x0000FF00) <<  8)
         | ((dw & 0x000000FF) << 24);
  }

  static FILETIME FileTimeFromAmigaTime(DWORD days, DWORD mins, DWORD ticks) {
    // Amiga: Since 1 jan 1978. ticks = 1/50s
    // FILETIME: 100-nanosecond intervals since 1 jan 1601
    const uint64_t amiga_epoch_offset = 11896934400ull*10000000ull; // or 11,896,070,400, or 11,896,977,600 depending on source
    ULARGE_INTEGER filetime;
    filetime.QuadPart = ticks * 10000000ull/50ull + mins * 60*10000000ull + days * 24*60*60*10000000ull + amiga_epoch_offset;
    FILETIME ft;
    ft.dwLowDateTime = filetime.LowPart;
    ft.dwHighDateTime = filetime.HighPart;
    return ft;
  }

  struct RootBlock {
    const char * buffer;

    RootBlock(const char * buffer) : buffer(buffer) {}

    DWORD type()        { return Little(*(DWORD *) &buffer[ 0]); }
    DWORD header_key()  { return Little(*(DWORD *) &buffer[ 4]); }
    DWORD high_seq()    { return Little(*(DWORD *) &buffer[ 8]); }
    DWORD ht_size()     { return Little(*(DWORD *) &buffer[12]); }
    DWORD first_data()  { return Little(*(DWORD *) &buffer[16]); }
    DWORD chksum()      { return Little(*(DWORD *) &buffer[20]); }
    DWORD * ht()        { return (DWORD *) &buffer[24]; }

    DWORD * ht_begin()  { return (DWORD *) &buffer[24]; }
    DWORD * ht_end()    { return (DWORD *) &buffer[BLOCK_SIZE-200]; }

    DWORD bm_flag()     { return Little(*(DWORD *) &buffer[BLOCK_SIZE-200]); }
    DWORD * bm_pages()  { return (DWORD *) &buffer[BLOCK_SIZE-196]; }
    DWORD bm_ext()      { return Little(*(DWORD *) &buffer[BLOCK_SIZE-96]); }
    DWORD r_days()      { return Little(*(DWORD *) &buffer[BLOCK_SIZE-92]); }
    DWORD r_mins()      { return Little(*(DWORD *) &buffer[BLOCK_SIZE-88]); }
    DWORD r_ticks()     { return Little(*(DWORD *) &buffer[BLOCK_SIZE-84]); }
    int name_len()      { return (int) buffer[BLOCK_SIZE-80]; }
    std::string GetVolumeName() {
        return std::string(&buffer[BLOCK_SIZE-79], &buffer[BLOCK_SIZE-79] + name_len());
    }
    DWORD v_days()      { return Little(*(DWORD *) &buffer[BLOCK_SIZE-40]); }
    DWORD v_mins()      { return Little(*(DWORD *) &buffer[BLOCK_SIZE-36]); }
    DWORD v_ticks()     { return Little(*(DWORD *) &buffer[BLOCK_SIZE-32]); }
    DWORD c_days()      { return Little(*(DWORD *) &buffer[BLOCK_SIZE-28]); }
    DWORD c_mins()      { return Little(*(DWORD *) &buffer[BLOCK_SIZE-24]); }
    DWORD c_ticks()     { return Little(*(DWORD *) &buffer[BLOCK_SIZE-20]); }

    DWORD next_hash()   { return Little(*(DWORD *) &buffer[BLOCK_SIZE-16]); }
    DWORD parent_dir()  { return Little(*(DWORD *) &buffer[BLOCK_SIZE-12]); }
    DWORD extension()   { return Little(*(DWORD *) &buffer[BLOCK_SIZE- 8]); }
    DWORD sec_type()    { return Little(*(DWORD *) &buffer[BLOCK_SIZE- 4]); }
  };

  struct ExtensionBlock {
    const char * buffer;
    ExtensionBlock(const char * buffer) : buffer(buffer) {}

    DWORD type()        { return Little(*(DWORD *) &buffer[ 0]); }
    DWORD header_key()  { return Little(*(DWORD *) &buffer[ 4]); }
    DWORD high_seq()    { return Little(*(DWORD *) &buffer[ 8]); }
    DWORD chksum()      { return Little(*(DWORD *) &buffer[20]); }
    DWORD data_blocks_begin() { return Little(*(DWORD *) &buffer[24]); }
    DWORD data_blocks_end()   { return Little(*(DWORD *) &buffer[BLOCK_SIZE-200]); }
    DWORD parent()      { return Little(*(DWORD *) &buffer[BLOCK_SIZE-12]); }
    DWORD extension()   { return Little(*(DWORD *) &buffer[BLOCK_SIZE- 8]); }
    DWORD sec_type()    { return Little(*(DWORD *) &buffer[BLOCK_SIZE- 4]); }
  };

  struct DataBlock {
    const char * buffer;
    DataBlock(const char * buffer) : buffer(buffer) {}
    DWORD type()        { return Little(*(DWORD *) &buffer[ 0]); }
    DWORD header_key()  { return Little(*(DWORD *) &buffer[ 4]); }
    DWORD seq_num()     { return Little(*(DWORD *) &buffer[ 8]); }
    DWORD data_size()   { return Little(*(DWORD *) &buffer[12]); }
    DWORD next_data()   { return Little(*(DWORD *) &buffer[16]); }
    DWORD chksum()      { return Little(*(DWORD *) &buffer[20]); }
    const char * data_begin() { return &buffer[24]; }
    const char * data_end() {
      int size = data_size();
      if (size > BLOCK_SIZE-24) {
          size = BLOCK_SIZE-24;
      }
      return data_begin() + size;
    }
  };

  struct HeaderBlock {
    const char * buffer;

    HeaderBlock(const char * buffer) : buffer(buffer) {}

    DWORD type()        { return Little(*(DWORD *) &buffer[ 0]); }
    DWORD high_seq()    { return Little(*(DWORD *) &buffer[ 8]); }
    DWORD ht_size()     { return *(DWORD *) &buffer[12]; }
    DWORD * ht_begin()  { return (DWORD *) &buffer[24]; }
    DWORD * ht_end()    { return (DWORD *) &buffer[BLOCK_SIZE-200]; }

    DWORD protect()     { return Little(*(DWORD *) &buffer[BLOCK_SIZE-192]); }
    DWORD byte_size()   { return Little(*(DWORD *) &buffer[BLOCK_SIZE-188]); }
    int comment_len()   { return (int) buffer[BLOCK_SIZE-184]; }
    std::string GetComment() {
        return std::string(&buffer[BLOCK_SIZE-183], &buffer[BLOCK_SIZE-183] + comment_len());
    }

    DWORD days()        { return Little(*(DWORD *) &buffer[BLOCK_SIZE-92]); }
    DWORD mins()        { return Little(*(DWORD *) &buffer[BLOCK_SIZE-88]); }
    DWORD ticks()       { return Little(*(DWORD *) &buffer[BLOCK_SIZE-84]); }
    FILETIME GetFileTime() {
        return FileTimeFromAmigaTime(days(), mins(), ticks());
    }

    int name_len()      { return (int) buffer[BLOCK_SIZE-80]; }
    std::string GetName() {
        return std::string(&buffer[BLOCK_SIZE-79], &buffer[BLOCK_SIZE-79] + name_len());
    }

    DWORD hash_chain()  { return Little(*(DWORD *) &buffer[BLOCK_SIZE-16]); }
    DWORD extension()   { return Little(*(DWORD *) &buffer[BLOCK_SIZE- 8]); }
    DWORD sec_type()    { return Little(*(DWORD *) &buffer[BLOCK_SIZE- 4]); }
  };

  struct Disk {
    std::vector<char> buffer;

    Disk() = default;
    Disk(const Disk &) = delete;
    const Disk & operator=(const Disk &) = delete;

    size_t size() { return buffer.size(); }

    const char * sector(int i) const { return &buffer[SECTOR_SIZE * i]; }

    bool IsAmigaDOS() const {
      return buffer[0] == 'D' &&
             buffer[1] == 'O' &&
             buffer[2] == 'S';
    }
    bool IsFFS() const            { return (buffer[3] & 1) == 1; }
    bool IsInternational() const  { return (buffer[3] & 6) != 0; }
    bool IsDirectoryCache() const { return (buffer[3] & 4) == 4; }
  };

  struct DiskEntry {
    int type;
    int sector;
    int parent;
    std::string name;
    std::string comment;
    FILETIME filetime;
    int filesize;
    DWORD attrib;
  };

  static void process(Disk & disk, DWORD * begin, DWORD * end, const std::string & parentName, std::vector<DiskEntry> & entries) {
    int visited_sectors = 0;
    for (DWORD * h = begin; h != end; ++h) {
      for (int sector = Little(*h); sector != 0; ) {
        HeaderBlock header(disk.sector(sector));
        if (header.type() != T_HEADER) {
          return;
        }

        std::string name = parentName + header.GetName();

        if (header.sec_type() == ST_FILE) {
          DiskEntry entry;
          entry.type = header.sec_type();
          entry.sector = sector;
          entry.name = name;
          entry.comment = header.GetComment();
          entry.filetime = header.GetFileTime();
          entry.filesize = header.byte_size();
          entry.attrib = header.protect();
          entries.push_back(entry);
        } else if (header.sec_type() == ST_USERDIR) {
          process(disk, header.ht_begin(), header.ht_end(), name + "/", entries);
        }
        sector = header.hash_chain();

        // sanity check: abort if we have visited more sectors than fits on a disk.
        // we're probably stuck in an infinite loop.
        ++visited_sectors;
        if (visited_sectors > NUM_SECTORS) {
          return;
        }
      }
    }
  }

  static bool extract(Disk & disk, const DiskEntry & entry, std::vector<char> & output) {
    HeaderBlock header(disk.sector(entry.sector));
    if (header.type() != T_HEADER) {
      return false;
    }
    bool first = true;
    for (;;) {
      if (header.sec_type() != ST_FILE) {
        return false;
      }
      int num = header.high_seq();
      for (int i = 0; i < num; ++i) {
        const int block = Little(*(header.ht_end() - i - 1));
        const int currsize = (int) output.size();

        const char * data;
        int chunksize;
        if (disk.IsFFS()) {
          const int remains = entry.filesize - currsize;
          if (remains < BLOCK_SIZE) {
            chunksize = remains;
          } else {
            chunksize = BLOCK_SIZE;
          }
          if (chunksize < 0) {
            return false;
          }
          data = disk.sector(block);
        } else {
          DataBlock db(disk.sector(block));
          data = db.data_begin();
          chunksize = (int) (db.data_end() - db.data_begin());
          if (output.size() + chunksize > entry.filesize) {
            return false;
          }
        }

        // allow first block to have zero size, but treat as error otherwise
        if (chunksize == 0 && !first) {
          return false;
        }

        output.resize(output.size() + chunksize);
        memcpy(&output[currsize], data, chunksize);

        first = false;
      }
      const int extension = header.extension();
      if (extension == 0) {
        break;
      }
      header = HeaderBlock(disk.sector(extension));
      if (header.type() != T_LIST) {
        return false;
      }
    }

    return true;
  }

};
