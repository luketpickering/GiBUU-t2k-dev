CXX := g++
RCINT := rootcint
RC := root-config

BDIR :=bin
LDIR :=lib

TARGET := GiBUUToStdHep.exe
TARGETSRC := $(TARGET:.exe=.cxx)

TOBJS := GiRooTracker.cxx GiBUUToStdHep_Utils.cxx
TOBJH := $(TOBJS:.cxx=.hxx)

ROOTCFLAGS := `$(RC) --cflags`
ROOTLDFLAGS := `$(RC) --libs --glibs`

CXXFLAGS := -fPIC $(ROOTCFLAGS) -g -std=c++11 -Wall
LDFLAGS := $(ROOTLDFLAGS) -Wl,-rpath=.

.PHONY: all clean

all: $(TARGET)
	mkdir -p $(BDIR)
	mv $(TARGET) $(BDIR)/
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built GiBUUToStdHep."
	@echo "*********************************************************************"


$(TARGET): $(TARGETSRC) GiBUUToStdHep_Utils.o GiRooTracker.o
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^

GiRooTracker.o: GiRooTracker.hxx GiRooTracker.cxx
	$(CXX) $(CXXFLAGS) -o $@ -c GiRooTracker.cxx

GiBUUToStdHep_Utils.o: GiBUUToStdHep_Utils.hxx GiBUUToStdHep_Utils.cxx
	$(CXX) $(CXXFLAGS) -o $@ -c GiBUUToStdHep_Utils.cxx

docs: $(TARGETSRC) $(TOBJS) $(TOBJH) dox/GiBUUToStdHep.dox.cfg
	cd dox; doxygen GiBUUToStdHep.dox.cfg;

latex_docs: docs
	cd dox/latex; $(MAKE);
	cp dox/latex/refman.pdf ./GiBUUToStdHep_dox.pdf

clean:
	rm -rf $(TARGET) GiBUUToStdHep_Utils.o GiRooTracker.o $(BDIR)
	rm -rf dox/html dox/latex GiBUUToStdHep_dox.pdf
