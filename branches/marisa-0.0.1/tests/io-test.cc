#ifdef _MSC_VER
#include <io.h>
#endif  // _MSC_VER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cassert>
#include <cstdio>
#include <fstream>

#include <marisa/io.h>

namespace {

void TestFilename() {
  {
    marisa::Writer writer;
    assert(writer.open("io-test.dat"));
    marisa::UInt32 value = 123;
    assert(writer.write(value));
    std::vector<double> vec;
    vec.push_back(234);
    vec.push_back(345);
    vec.push_back(456);
    assert(writer.write(vec));
  }

  {
    marisa::Writer writer;
    assert(writer.open("io-test.dat", false, 4, SEEK_SET));
    marisa::UInt32 value = 4;
    assert(writer.write(value));
  }

  {
    marisa::Writer writer;
    assert(writer.open("io-test.dat", false, 0, SEEK_END));
    double value = 567;
    assert(writer.write(value));
  }

  {
    marisa::Reader reader;
    assert(reader.open("io-test.dat"));
    marisa::UInt32 value;
    assert(reader.read(&value));
    assert(value == 123);
    std::vector<double> vec;
    assert(reader.read(&vec));
    assert(vec.size() == 4);
    assert(vec[0] == 234);
    assert(vec[1] == 345);
    assert(vec[2] == 456);
    assert(vec[3] == 567);
    char byte;
    assert(!reader.read(&byte));
  }

  {
    marisa::Mapper mapper;
    assert(mapper.open("io-test.dat"));
    marisa::UInt32 value;
    assert(mapper.map(&value));
    assert(value == 123);
    const double *objs;
    marisa::UInt32 num_objs;
    assert(mapper.map(&objs, &num_objs));
    assert(num_objs == 4);
    assert(objs[0] == 234);
    assert(objs[1] == 345);
    assert(objs[2] == 456);
    assert(objs[3] == 567);
    char byte;
    assert(!mapper.map(&byte));
  }

  {
    marisa::Writer writer;
    assert(writer.open("io-test.dat"));
  }

  {
    marisa::Reader reader;
    assert(reader.open("io-test.dat"));
    char byte;
    assert(!reader.read(&byte));
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
    assert(writer.write(value));
    std::vector<double> vec;
    vec.push_back(456);
    vec.push_back(567);
    vec.push_back(678);
    assert(writer.write(vec));
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
    assert(reader.read(&value));
    assert(value == 345);
    std::vector<double> vec;
    assert(reader.read(&vec));
    assert(vec.size() == 3);
    assert(vec[0] == 456);
    assert(vec[1] == 567);
    assert(vec[2] == 678);
    char byte;
    assert(!reader.read(&byte));
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
    assert(writer.write(value));
    std::vector<double> vec;
    vec.push_back(456);
    vec.push_back(567);
    vec.push_back(678);
    assert(writer.write(vec));
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
    assert(reader.read(&value));
    assert(value == 345);
    std::vector<double> vec;
    assert(reader.read(&vec));
    assert(vec.size() == 3);
    assert(vec[0] == 456);
    assert(vec[1] == 567);
    assert(vec[2] == 678);
    char byte;
    assert(!reader.read(&byte));
    assert(std::fclose(file) == 0);
  }
}

void TestStream() {
  {
    std::ofstream stream("io-test.dat", std::ios::binary);
    marisa::Writer writer(&stream);
    marisa::UInt32 value = 456;
    assert(writer.write(value));
    std::vector<double> vec;
    vec.push_back(567);
    vec.push_back(678);
    vec.push_back(789);
    assert(writer.write(vec));
  }

  {
    std::ifstream stream("io-test.dat", std::ios::binary);
    marisa::Reader reader(&stream);
    marisa::UInt32 value;
    assert(reader.read(&value));
    assert(value == 456);
    std::vector<double> vec;
    assert(reader.read(&vec));
    assert(vec.size() == 3);
    assert(vec[0] == 567);
    assert(vec[1] == 678);
    assert(vec[2] == 789);
    char byte;
    assert(!reader.read(&byte));
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
