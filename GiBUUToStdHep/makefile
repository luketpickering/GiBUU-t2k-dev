CXX = g++
RCINT = rootcint
RC = root-config

BDIR = bin

LHPCROOT =../LesHouchesParserClasses_CPP/LHPC
LHPCINC :=$(LHPCROOT)/include
LHPCLIB :=$(LHPCROOT)/lib/libLHPC.a

TARGET = GiBUUToStdHep.exe
TARGETSRC := $(TARGET:.exe=.cxx)

TOBJS = GiRooTracker.cxx GiBUUToStdHep_Utils.cxx
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


$(TARGET): $(TARGETSRC) GiBUUToStdHep_Utils.o GiRooTracker.o $(LHPCLIB)
	$(CXX) -o $@ $(CXXFLAGS) $(LHPCLIB) -I$(LHPCINC) $(LDFLAGS) $^

GiRooTracker.o: GiRooTracker.hxx GiRooTracker.cxx
	$(CXX) $(CXXFLAGS) -o $@ -c GiRooTracker.cxx

GiBUUToStdHep_Utils.o: GiBUUToStdHep_Utils.hxx GiBUUToStdHep_Utils.cxx
	$(CXX) $(CXXFLAGS) -o $@ -c GiBUUToStdHep_Utils.cxx

$(LHPCLIB):
	cd $(LHPCROOT); $(MAKE)

docs: $(TARGETSRC) $(TOBJS) $(TOBJH) dox/GiBUUToStdHep.dox.cfg
	cd dox; doxygen GiBUUToStdHep.dox.cfg;

latex_docs: docs
	cd dox/latex; $(MAKE);
	cp dox/latex/refman.pdf ./GiBUUToStdHep_dox.pdf

clean:
	rm -rf $(TARGET) GiBUUToStdHep_Utils.o GiRooTracker.o $(BDIR)
	rm -rf dox/html dox/latex GiBUUToStdHep_dox.pdf
	cd $(LHPCROOT); $(MAKE) clean
