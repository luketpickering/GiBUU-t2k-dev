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
	mkdir -p $(BDIR) $(LDIR)
	mv $(TARGET) $(BDIR)/
	mv GiBUUToStdHep_Utils.so GiRooTracker.so $(LDIR)/
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built GiBUUToStdHep."
	@echo "*********************************************************************"


$(TARGET): $(TARGETSRC) GiBUUToStdHep_Utils.so GiRooTracker.so
	$(CXX) -o $@ $(CXXFLAGS) $(LDFLAGS) $^

GiRooTracker.so: GiRooTracker.hxx GiRooTracker.cxx
	$(CXX) $(CXXFLAGS) -shared -o $@ GiRooTracker.cxx

GiBUUToStdHep_Utils.so: GiBUUToStdHep_Utils.hxx GiBUUToStdHep_Utils.cxx
	$(CXX) $(CXXFLAGS) -shared -o $@ GiBUUToStdHep_Utils.cxx

clean:
	rm -rf $(TARGET) GiBUUToStdHep_Utils.so GiRooTracker.so $(LDIR) $(BDIR)
