TOPDIR := $(shell dirname `pwd`)

PYDIR   = $(TOPDIR)/python
UTILDIR = $(TOPDIR)/util

LIBDIR  = $(PYDIR)/lib
SRCDIR  = $(PYDIR)/src

ifneq (,$(findstring Darwin,$(shell uname -a)))
	LIBSO_FLAGS= -dynamiclib -undefined dynamic_lookup
else
	LIBSO_FLAGS= -shared
endif

#PYINCDIR    = /Users/eml/.pyenv/versions/riak_2.6.9/include/python2.6/
#NUMPYINCDIR = /usr/local/lib/python2.7/site-packages/numpy/core/include/numpy

ifndef PYINCDIR
  error = true
endif

ifndef NUMPYINCDIR
  error = true
endif

ifdef error
all:
	@echo "You must define PYINCDIR (path to Python.h) and NUMPYINCDIR (path to arrayobject.h) to compile this code"
else
all: dirs compile libs
endif

mytest:
	echo $(TESTDIR)

dirs:
	if [ ! -d $(LIBDIR) ]; then mkdir $(LIBDIR); fi
	if [ ! -d $(SRCDIR) ]; then mkdir $(SRCDIR); fi

compile: copy_src compile_python

copy_src:
	cp $(UTILDIR)/* $(PYDIR)/src
	cp $(PYDIR)/c_src/* $(PYDIR)/src

compile_python:
	cd $(PYDIR)/src; g++ -I $(PYINCDIR) -I $(NUMPYINCDIR) -c *.cc *.cpp
	cd $(BASEDIR)

libs:
	g++ $(LIBSO_FLAGS) -o $(LIBDIR)/profilerPyTest.so $(PYDIR)/src/*.o

clean:
	\rm -rf $(LIBDIR)
	\rm -rf $(SRCDIR)





