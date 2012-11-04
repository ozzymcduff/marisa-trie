AM_CFLAGS = -Wall -I../lib
AM_CXXFLAGS = -Wall -Weffc++ -I../lib

bin_PROGRAMS = \
  marisa-alpha-build \
  marisa-alpha-lookup \
  marisa-alpha-benchmark \
  marisa-alpha-find \
  marisa-alpha-predict

noinst_HEADERS = cmdopt.h

marisa_alpha_build_SOURCES = marisa-alpha-build.cc cmdopt.c
marisa_alpha_build_LDADD = ../lib/libmarisa_alpha.la

marisa_alpha_lookup_SOURCES = marisa-alpha-lookup.cc cmdopt.c
marisa_alpha_lookup_LDADD = ../lib/libmarisa_alpha.la

marisa_alpha_find_SOURCES = marisa-alpha-find.cc cmdopt.c
marisa_alpha_find_LDADD = ../lib/libmarisa_alpha.la

marisa_alpha_predict_SOURCES = marisa-alpha-predict.cc cmdopt.c
marisa_alpha_predict_LDADD = ../lib/libmarisa_alpha.la

marisa_alpha_benchmark_SOURCES = marisa-alpha-benchmark.cc cmdopt.c
marisa_alpha_benchmark_LDADD = ../lib/libmarisa_alpha.la
