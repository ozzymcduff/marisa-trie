AM_CXXFLAGS = -Wall -Weffc++

lib_LIBRARIES = libmarisa.a

libmarisa_a_SOURCES = \
  marisa/base.cc \
  marisa/progress.cc \
  marisa/reader.cc \
  marisa/writer.cc \
  marisa/mapper.cc \
  marisa/intvector.cc \
  marisa/bitvector.cc \
  marisa/tail.cc \
  marisa/trie.cc \
  marisa/trie-build.cc

include_HEADERS = marisa.h

libmarisa_includedir = ${includedir}/marisa
libmarisa_include_HEADERS = \
  marisa/base.h \
  marisa/rank.h \
  marisa/popcount.h \
  marisa/progress.h \
  marisa/string.h \
  marisa/key.h \
  marisa/range.h \
  marisa/query.h \
  marisa/cell.h \
  marisa/reader.h \
  marisa/writer.h \
  marisa/mapper.h \
  marisa/io.h \
  marisa/vector.h \
  marisa/vector-inline.h \
  marisa/intvector.h \
  marisa/bitvector.h \
  marisa/tail.h \
  marisa/trie.h \
  marisa/trie-inline.h
