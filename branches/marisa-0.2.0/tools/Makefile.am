AM_CFLAGS = -Wall -Wextra
AM_CXXFLAGS = -Wall -Weffc++ -Wextra -I../lib
#AM_CXXFLAGS = -Wall -Weffc++ -Wextra -D_DEBUG -I../lib

bin_PROGRAMS = \
  marisa-build \
  marisa-lookup \
  marisa-reverse-lookup \
  marisa-common-prefix-search \
  marisa-predictive-search \
  marisa-benchmark

noinst_HEADERS = cmdopt.h

marisa_build_SOURCES = marisa-build.cc cmdopt.c
marisa_build_LDADD = ../lib/libmarisa.la

marisa_lookup_SOURCES = marisa-lookup.cc cmdopt.c
marisa_lookup_LDADD = ../lib/libmarisa.la

marisa_reverse_lookup_SOURCES = marisa-reverse-lookup.cc cmdopt.c
marisa_reverse_lookup_LDADD = ../lib/libmarisa.la

marisa_common_prefix_search_SOURCES = marisa-common-prefix-search.cc cmdopt.c
marisa_common_prefix_search_LDADD = ../lib/libmarisa.la

marisa_predictive_search_SOURCES = marisa-predictive-search.cc cmdopt.c
marisa_predictive_search_LDADD = ../lib/libmarisa.la

marisa_benchmark_SOURCES = marisa-benchmark.cc cmdopt.c
marisa_benchmark_LDADD = ../lib/libmarisa.la
