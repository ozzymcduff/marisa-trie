AM_CXXFLAGS = -Wall -Weffc++ -DDEBUG -I../lib

TESTS = \
  unit-test \
  io-test \
  vector-test \
  intvector-test \
  bitvector-test \
  tail-test \
  trie-test

check_PROGRAMS = $(TESTS)

unit_test_SOURCES = unit-test.cc
unit_test_LDADD = ../lib/libmarisa.a

io_test_SOURCES = io-test.cc
io_test_LDADD = ../lib/libmarisa.a

vector_test_SOURCES = vector-test.cc
vector_test_LDADD = ../lib/libmarisa.a

intvector_test_SOURCES = intvector-test.cc
intvector_test_LDADD = ../lib/libmarisa.a

bitvector_test_SOURCES = bitvector-test.cc
bitvector_test_LDADD = ../lib/libmarisa.a

tail_test_SOURCES = tail-test.cc
tail_test_LDADD = ../lib/libmarisa.a

trie_test_SOURCES = trie-test.cc
trie_test_LDADD = ../lib/libmarisa.a
