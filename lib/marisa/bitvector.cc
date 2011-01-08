#include <algorithm>

#include "./bitvector.h"
#include "./popcount.h"

namespace marisa {
namespace {

const UInt8 SelectTable[8][256] = {
  {
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
  },
  {
    7, 7, 7, 1, 7, 2, 2, 1, 7, 3, 3, 1, 3, 2, 2, 1,
    7, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1,
    5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 6, 6, 1, 6, 2, 2, 1, 6, 3, 3, 1, 3, 2, 2, 1,
    6, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    6, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1,
    5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 7, 7, 1, 7, 2, 2, 1, 7, 3, 3, 1, 3, 2, 2, 1,
    7, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1,
    5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    7, 6, 6, 1, 6, 2, 2, 1, 6, 3, 3, 1, 3, 2, 2, 1,
    6, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1,
    6, 5, 5, 1, 5, 2, 2, 1, 5, 3, 3, 1, 3, 2, 2, 1,
    5, 4, 4, 1, 4, 2, 2, 1, 4, 3, 3, 1, 3, 2, 2, 1
  },
  {
    7, 7, 7, 7, 7, 7, 7, 2, 7, 7, 7, 3, 7, 3, 3, 2,
    7, 7, 7, 4, 7, 4, 4, 2, 7, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 5, 7, 5, 5, 2, 7, 5, 5, 3, 5, 3, 3, 2,
    7, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 6, 7, 6, 6, 2, 7, 6, 6, 3, 6, 3, 3, 2,
    7, 6, 6, 4, 6, 4, 4, 2, 6, 4, 4, 3, 4, 3, 3, 2,
    7, 6, 6, 5, 6, 5, 5, 2, 6, 5, 5, 3, 5, 3, 3, 2,
    6, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 7, 7, 7, 7, 2, 7, 7, 7, 3, 7, 3, 3, 2,
    7, 7, 7, 4, 7, 4, 4, 2, 7, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 5, 7, 5, 5, 2, 7, 5, 5, 3, 5, 3, 3, 2,
    7, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2,
    7, 7, 7, 6, 7, 6, 6, 2, 7, 6, 6, 3, 6, 3, 3, 2,
    7, 6, 6, 4, 6, 4, 4, 2, 6, 4, 4, 3, 4, 3, 3, 2,
    7, 6, 6, 5, 6, 5, 5, 2, 6, 5, 5, 3, 5, 3, 3, 2,
    6, 5, 5, 4, 5, 4, 4, 2, 5, 4, 4, 3, 4, 3, 3, 2
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 3,
    7, 7, 7, 7, 7, 7, 7, 4, 7, 7, 7, 4, 7, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 5, 7, 7, 7, 5, 7, 5, 5, 3,
    7, 7, 7, 5, 7, 5, 5, 4, 7, 5, 5, 4, 5, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 3,
    7, 7, 7, 6, 7, 6, 6, 4, 7, 6, 6, 4, 6, 4, 4, 3,
    7, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 3,
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 3,
    7, 7, 7, 7, 7, 7, 7, 4, 7, 7, 7, 4, 7, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 5, 7, 7, 7, 5, 7, 5, 5, 3,
    7, 7, 7, 5, 7, 5, 5, 4, 7, 5, 5, 4, 5, 4, 4, 3,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 3,
    7, 7, 7, 6, 7, 6, 6, 4, 7, 6, 6, 4, 6, 4, 4, 3,
    7, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 3,
    7, 6, 6, 5, 6, 5, 5, 4, 6, 5, 5, 4, 5, 4, 4, 3
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 5, 7, 7, 7, 5, 7, 5, 5, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 4,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 5,
    7, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 5, 7, 7, 7, 5, 7, 5, 5, 4,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 4,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 5,
    7, 7, 7, 6, 7, 6, 6, 5, 7, 6, 6, 5, 6, 5, 5, 4
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 5,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 5,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 6, 7, 6, 6, 5
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6
  },
  {
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
  }
};

}  // namespace

BitVector::BitVector()
    : blocks_(), num_bits_(0), ranks_(), select0s_(), select1s_() {}

void BitVector::build() {
  Vector<Rank> ranks((num_bits_ + 511) / 512);
  Vector<UInt32> select0s;
  Vector<UInt32> select1s;
  UInt32 num_0s = 0;
  UInt32 num_1s = 0;
  for (UInt32 i = 0; i < num_bits_; ++i) {
    if ((i % 64) == 0) {
      UInt32 rank_id = i / 512;
      switch ((i / 64) % 8) {
        case 0: {
          ranks[rank_id].set_abs(num_1s);
          break;
        }
        case 1: {
          ranks[rank_id].set_rel1(num_1s - ranks[rank_id].abs());
          break;
        }
        case 2: {
          ranks[rank_id].set_rel2(num_1s - ranks[rank_id].abs());
          break;
        }
        case 3: {
          ranks[rank_id].set_rel3(num_1s - ranks[rank_id].abs());
          break;
        }
        case 4: {
          ranks[rank_id].set_rel4(num_1s - ranks[rank_id].abs());
          break;
        }
        case 5: {
          ranks[rank_id].set_rel5(num_1s - ranks[rank_id].abs());
          break;
        }
        case 6: {
          ranks[rank_id].set_rel6(num_1s - ranks[rank_id].abs());
          break;
        }
        case 7: {
          ranks[rank_id].set_rel7(num_1s - ranks[rank_id].abs());
          break;
        }
      }
    }
    if ((*this)[i]) {
      if ((num_1s % 512) == 0) {
        select1s.push_back(i);
      }
      ++num_1s;
    } else {
      if ((num_0s % 512) == 0) {
        select0s.push_back(i);
      }
      ++num_0s;
    }
  }
  if ((num_bits_ % 512) != 0) {
    UInt32 rank_id = (num_bits_ - 1) / 512;
    switch (((num_bits_ - 1) / 64) % 8) {
      case 0: {
        ranks[rank_id].set_rel1(num_1s - ranks[rank_id].abs());
      }
      case 1: {
        ranks[rank_id].set_rel2(num_1s - ranks[rank_id].abs());
      }
      case 2: {
        ranks[rank_id].set_rel3(num_1s - ranks[rank_id].abs());
      }
      case 3: {
        ranks[rank_id].set_rel4(num_1s - ranks[rank_id].abs());
      }
      case 4: {
        ranks[rank_id].set_rel5(num_1s - ranks[rank_id].abs());
      }
      case 5: {
        ranks[rank_id].set_rel6(num_1s - ranks[rank_id].abs());
      }
      case 6: {
        ranks[rank_id].set_rel7(num_1s - ranks[rank_id].abs());
        break;
      }
    }
  }
  select0s.push_back(num_bits_);
  select1s.push_back(num_bits_);
  select0s.shrink();
  select1s.shrink();

  blocks_.shrink();
  ranks_.swap(&ranks);
  select0s_.swap(&select0s);
  select1s_.swap(&select1s);
}

bool BitVector::mmap(Mapper *mapper, const char *filename,
    long offset, int whence) {
  Mapper temp_mapper;
  if (!temp_mapper.open(filename, offset, whence) || !map(&temp_mapper)) {
    return false;
  }
  temp_mapper.swap(mapper);
  return true;
}

bool BitVector::map(const void *ptr) {
  Mapper mapper(ptr);
  return map(&mapper);
}

bool BitVector::map(const void *ptr, std::size_t size) {
  Mapper mapper(ptr, size);
  return map(&mapper);
}

bool BitVector::map(Mapper *mapper) {
  BitVector temp;
  if (!temp.blocks_.map(mapper) ||
      !mapper->map(&temp.num_bits_) ||
      !temp.ranks_.map(mapper) ||
      !temp.select0s_.map(mapper) ||
      !temp.select1s_.map(mapper)) {
    return false;
  }
  temp.swap(this);
  return true;
}

bool BitVector::load(const char *filename, long offset, int whence) {
  Reader reader;
  if (!reader.open(filename, offset, whence)) {
    return false;
  }
  return read(&reader);
}

bool BitVector::read(int fd) {
  Reader reader(fd);
  return read(&reader);
}

bool BitVector::read(::FILE *file) {
  Reader reader(file);
  return read(&reader);
}

bool BitVector::read(std::istream *stream) {
  Reader reader(stream);
  return read(&reader);
}

bool BitVector::read(Reader *reader) {
  BitVector temp;
  if (!temp.blocks_.read(reader) ||
      !reader->read(&temp.num_bits_) ||
      !temp.ranks_.read(reader) ||
      !temp.select0s_.read(reader) ||
      !temp.select1s_.read(reader)) {
    return false;
  }
  temp.swap(this);
  return true;
}

bool BitVector::save(const char *filename, bool trunc_flag,
    long offset, int whence) const {
  Writer writer;
  if (!writer.open(filename, trunc_flag, offset, whence)) {
    return false;
  }
  return write(&writer);
}

bool BitVector::write(int fd) const {
  Writer writer(fd);
  return write(&writer);
}

bool BitVector::write(::FILE *file) const {
  Writer writer(file);
  return write(&writer);
}

bool BitVector::write(std::ostream *stream) const {
  Writer writer(stream);
  return write(&writer);
}

bool BitVector::write(Writer *writer) const {
  return blocks_.write(writer) &&
      writer->write(num_bits_) &&
      ranks_.write(writer) &&
      select0s_.write(writer) &&
      select1s_.write(writer);
}

UInt32 BitVector::rank1(UInt32 i) const {
  const Rank &rank = ranks_[i / 512];
  UInt32 offset = rank.abs();
  switch ((i / 64) % 8) {
    case 1: {
      offset += rank.rel1();
      break;
    }
    case 2: {
      offset += rank.rel2();
      break;
    }
    case 3: {
      offset += rank.rel3();
      break;
    }
    case 4: {
      offset += rank.rel4();
      break;
    }
    case 5: {
      offset += rank.rel5();
      break;
    }
    case 6: {
      offset += rank.rel6();
      break;
    }
    case 7: {
      offset += rank.rel7();
      break;
    }
  }
  switch ((i / 32) % 2) {
    case 1: {
      offset += PopCount(blocks_[(i / 32) - 1]);
    }
    case 0: {
      offset += PopCount(blocks_[i / 32]
          & ((static_cast<UInt32>(1) << (i % 32)) - 1));
      break;
    }
  }
  return offset;
}

UInt32 BitVector::select0(UInt32 i) const {
  UInt32 select_id = i / 512;
  if ((i % 512) == 0) {
    return select0s_[select_id];
  }
  UInt32 begin = select0s_[select_id] / 512;
  UInt32 end = (select0s_[select_id + 1] + 511) / 512;
  while (begin + 1 < end) {
    UInt32 middle = (begin + end) / 2;
    if (i < (middle * 512) - ranks_[middle].abs()) {
      end = middle;
    } else {
      begin = middle;
    }
  }
  UInt32 rank_id = begin;
  i -= (rank_id * 512) - ranks_[rank_id].abs();

  const Rank &rank = ranks_[rank_id];
  UInt32 block_id = rank_id * 16;
  if (i < (256U - rank.rel4())) {
    if (i < (128U - rank.rel2())) {
      if (i >= (64U - rank.rel1())) {
        block_id += 2;
        i -= 64 - rank.rel1();
      }
    } else if (i < (192U - rank.rel3())) {
      block_id += 4;
      i -= 128 - rank.rel2();
    } else {
      block_id += 6;
      i -= 192 - rank.rel3();
    }
  } else if (i < (384U - rank.rel6())) {
    if (i < (320U - rank.rel5())) {
      block_id += 8;
      i -= 256 - rank.rel4();
    } else {
      block_id += 10;
      i -= 320 - rank.rel5();
    }
  } else if (i < (448U - rank.rel7())) {
    block_id += 12;
    i -= 384 - rank.rel6();
  } else {
    block_id += 14;
    i -= 448 - rank.rel7();
  }

  UInt32 block = ~blocks_[block_id];
  PopCount count(block);
  if (i >= count.lo32()) {
    ++block_id;
    i -= count.lo32();
    block = ~blocks_[block_id];
    count = PopCount(block);
  }

  UInt32 bit_id = block_id * 32;
  if (i < count.lo16()) {
    if (i >= count.lo8()) {
      bit_id += 8;
      block >>= 8;
      i -= count.lo8();
    }
  } else if (i < count.lo24()) {
    bit_id += 16;
    block >>= 16;
    i -= count.lo16();
  } else {
    bit_id += 24;
    block >>= 24;
    i -= count.lo24();
  }
  return bit_id + SelectTable[i][block & 0xFF];
}

UInt32 BitVector::select1(UInt32 i) const {
  UInt32 select_id = i / 512;
  if ((i % 512) == 0) {
    return select1s_[select_id];
  }
  UInt32 begin = select1s_[select_id] / 512;
  UInt32 end = (select1s_[select_id + 1] + 511) / 512;
  while (begin + 1 < end) {
    UInt32 middle = (begin + end) / 2;
    if (i < ranks_[middle].abs()) {
      end = middle;
    } else {
      begin = middle;
    }
  }
  UInt32 rank_id = begin;
  i -= ranks_[rank_id].abs();

  const Rank &rank = ranks_[rank_id];
  UInt32 block_id = rank_id * 16;
  if (i < rank.rel4()) {
    if (i < rank.rel2()) {
      if (i >= rank.rel1()) {
        block_id += 2;
        i -= rank.rel1();
      }
    } else if (i < rank.rel3()) {
      block_id += 4;
      i -= rank.rel2();
    } else {
      block_id += 6;
      i -= rank.rel3();
    }
  } else if (i < rank.rel6()) {
    if (i < rank.rel5()) {
      block_id += 8;
      i -= rank.rel4();
    } else {
      block_id += 10;
      i -= rank.rel5();
    }
  } else if (i < rank.rel7()) {
    block_id += 12;
    i -= rank.rel6();
  } else {
    block_id += 14;
    i -= rank.rel7();
  }

  UInt32 block = blocks_[block_id];
  PopCount count(block);
  if (i >= count.lo32()) {
    ++block_id;
    i -= count.lo32();
    block = blocks_[block_id];
    count = PopCount(block);
  }

  UInt32 bit_id = block_id * 32;
  if (i < count.lo16()) {
    if (i >= count.lo8()) {
      bit_id += 8;
      block >>= 8;
      i -= count.lo8();
    }
  } else if (i < count.lo24()) {
    bit_id += 16;
    block >>= 16;
    i -= count.lo16();
  } else {
    bit_id += 24;
    block >>= 24;
    i -= count.lo24();
  }
  return bit_id + SelectTable[i][block & 0xFF];
}

void BitVector::clear() {
  BitVector().swap(this);
}

void BitVector::swap(BitVector *rhs) {
  blocks_.swap(&rhs->blocks_);
  std::swap(num_bits_, rhs->num_bits_);
  ranks_.swap(&rhs->ranks_);
  select0s_.swap(&rhs->select0s_);
  select1s_.swap(&rhs->select1s_);
}

}  // namespace marisa
