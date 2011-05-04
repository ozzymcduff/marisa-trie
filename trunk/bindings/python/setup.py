from distutils.core import setup, Extension

marisa_module = Extension("_marisa",
                          sources=["marisa-swig_wrap.cxx", "marisa-swig.cxx"],
                          libraries=["marisa"])

setup(name = "marisa",
      version = "0.2.0-beta4",
      author = "Susumu Yata",
      url = "http://code.google.com/p/marisa-trie/",
      description = """Python binding for libmarisa""",
      ext_modules = [marisa_module],
      py_modules = ["marisa"])
