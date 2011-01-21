#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif  // _MSC_VER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include <marisa/io.h>

namespace {

void TestFilename() {
  {
    marisa::Writer writer;
    writer.open("io-test.dat");
    marisa::UInt32 value = 123;
    writer.write(value);
    writer.write(value);
    double values[] = { 345, 456 };
    writer.write(values, 2);
    try {
      writer.write(values, 1U << 30);
      assert(false);
    } catch (const marisa::Exception &ex) {
      assert(ex.status() == MARISA_SIZE_ERROR);
    }
  }

  {
    marisa::Writer writer;
    writer.open("io-test.dat", false, 4, SEEK_SET);
    marisa::UInt32 value = 234;
    writer.write(value);
  }

  {
    marisa::Writer writer;
    writer.open("io-test.dat", false, 0, SEEK_END);
    double value = 567;
    writer.write(value);
  }

  {
    marisa::Reader reader;
    reader.open("io-test.dat");
    marisa::UInt32 value;
    reader.read(&value);
    assert(value == 123);
    reader.read(&value);
    assert(value == 234);
    double values[3];
    reader.read(values, 3);
    assert(values[0] == 345);
    assert(values[1] == 456);
    assert(values[2] == 567);
    char byte;
    try {
      reader.read(&byte);
      assert(false);
    } catch (const marisa::Exception &ex) {
      assert(ex.status() == MARISA_IO_ERROR);
    }
  }

  {
    marisa::Mapper mapper;
    mapper.open("io-test.dat");
    marisa::UInt32 value;
    mapper.map(&value);
    assert(value == 123);
    mapper.map(&value);
    assert(value == 234);
    const double *values;
    mapper.map(&values, 3);
    assert(values[0] == 345);
    assert(values[1] == 456);
    assert(values[2] == 567);
    char byte;
    try {
      mapper.map(&byte);
      assert(false);
    } catch (const marisa::Exception &ex) {
      assert(ex.status() == MARISA_IO_ERROR);
    }
  }

  {
    marisa::Writer writer;
    writer.open("io-test.dat");
  }

  {
    marisa::Reader reader;
    reader.open("io-test.dat");
    char byte;
    try {
      reader.read(&byte);
      assert(false);
    } catch (const marisa::Exception &ex) {
      assert(ex.status() == MARISA_IO_ERROR);
    }
  }
}

void TestFd() {
  {
#ifdef _MSC_VER
    int fd = -1;
    assert(::_sopen_s(&fd, "io-test.dat",
        _O_BINARY | _O_CREAT | _O_WRONLY | _O_TRUNC,
        _SH_DENYRW, _S_IREAD | _S_IWRITE) == 0);
#else  // _MSC_VER
    int fd = ::creat("io-test.dat", 0644);
    assert(fd != -1);
#endif  // _MSC_VER
    marisa::Writer writer(fd);
    marisa::UInt32 value = 345;
    writer.write(value);
    double values[] = { 456, 567, 678 };
    writer.write(values, 3);
#ifdef _MSC_VER
    assert(::_close(fd) == 0);
#else  // _MSC_VER
    assert(::close(fd) == 0);
#endif  // _MSC_VER
  }

  {
#ifdef _MSC_VER
    int fd = -1;
    assert(::_sopen_s(&fd, "io-test.dat", _O_BINARY | _O_RDONLY,
        _SH_DENYRW, _S_IREAD) == 0);
#else  // _MSC_VER
    int fd = ::open("io-test.dat", O_RDONLY);
    assert(fd != -1);
#endif  // _MSC_VER
    marisa::Reader reader(fd);
    marisa::UInt32 value;
    reader.read(&value);
    assert(value == 345);
    double values[3];
    reader.read(values, 3);
    assert(values[0] == 456);
    assert(values[1] == 567);
    assert(values[2] == 678);
    char byte;
    try {
      reader.read(&byte);
      assert(false);
    } catch (const marisa::Exception &ex) {
      assert(ex.status() == MARISA_IO_ERROR);
    }
#ifdef _MSC_VER
    assert(::_close(fd) == 0);
#else  // _MSC_VER
    assert(::close(fd) == 0);
#endif  // _MSC_VER
  }
}

void TestFile() {
  {
#ifdef _MSC_VER
    FILE *file = NULL;
    assert(::fopen_s(&file, "io-test.dat", "wb") == 0);
#else  // _MSC_VER
    FILE *file = std::fopen("io-test.dat", "wb");
    assert(file != NULL);
#endif  // _MSC_VER
    marisa::Writer writer(file);
    marisa::UInt32 value = 345;
    writer.write(value);
    double values[3] = { 456, 567, 678 };
    writer.write(values, 3);
    assert(std::fclose(file) == 0);
  }

  {
#ifdef _MSC_VER
    FILE *file = NULL;
    assert(::fopen_s(&file, "io-test.dat", "rb") == 0);
#else  // _MSC_VER
    FILE *file = std::fopen("io-test.dat", "rb");
    assert(file != NULL);
#endif  // _MSC_VER
    marisa::Reader reader(file);
    marisa::UInt32 value;
    reader.read(&value);
    assert(value == 345);
    double values[3];
    reader.read(values, 3);
    assert(values[0] == 456);
    assert(values[1] == 567);
    assert(values[2] == 678);
    char byte;
    try {
      reader.read(&byte);
      assert(false);
    } catch (const marisa::Exception &ex) {
      assert(ex.status() == MARISA_IO_ERROR);
    }
    assert(std::fclose(file) == 0);
  }
}

void TestStream() {
  {
    std::ofstream file("io-test.dat", std::ios::binary);
    assert(file.is_open());
    marisa::Writer writer(&file);
    marisa::UInt32 value = 345;
    writer.write(value);
    double values[3] = { 456, 567, 678 };
    writer.write(values, 3);
  }

  {
    std::ifstream file("io-test.dat", std::ios::binary);
    assert(file.is_open());
    marisa::Reader reader(&file);
    marisa::UInt32 value;
    reader.read(&value);
    assert(value == 345);
    double values[3];
    reader.read(values, 3);
    assert(values[0] == 456);
    assert(values[1] == 567);
    assert(values[2] == 678);
    char byte;
    try {
      reader.read(&byte);
      assert(false);
    } catch (const marisa::Exception &ex) {
      assert(ex.status() == MARISA_IO_ERROR);
    }
  }
}

}  // namespace

int main() {
  TestFilename();
  TestFd();
  TestFile();
  TestStream();
  return 0;
}
