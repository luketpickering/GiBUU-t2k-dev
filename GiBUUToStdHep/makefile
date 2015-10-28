CXX = g++
RCINT = rootcint
RC = root-config

BDIR = bin

TARGET = GiBUUToStdHep.exe
TARGETSRC := $(TARGET:.exe=.cxx)

SRCF = GiRooTracker.cxx GiBUUToStdHep_Utils.cxx
HF := $(SRCF:.cxx=.hxx)
OBJS := $(SRCF:.cxx=.o)

ROOTCFLAGS := `$(RC) --cflags`
ROOTLDFLAGS := `$(RC) --libs --glibs`

CXXFLAGS := -fPIC $(ROOTCFLAGS) -g -std=c++11 -Wall
LDFLAGS := $(ROOTLDFLAGS) -Wl,-rpath=.

LHPCROOT =../LesHouchesParserClasses_CPP/LHPC
LHPCINC :=$(LHPCROOT)/include
LHPCLIB :=$(LHPCROOT)/lib/libLHPC.a
LHPCLD := -L$(LHPCROOT)/lib -lLHPC

UTILSLOC := ../utils
LIBUTILS := $(UTILSLOC)/lib/libPureGenUtils.a
LIBUTILSLD := -L$(UTILSLOC)/lib  -lPureGenUtils

.PHONY: all clean distclean

all: $(TARGET)
	mkdir -p $(BDIR)
	mv $(TARGET) $(BDIR)/
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built GiBUUToStdHep."
	@echo "*********************************************************************"

../GiBUUInstall/release1.6:
	mkdir -p ../GiBUUInstall;
	cd ../GiBUUInstall; svn co http://gibuu.hepforge.org/svn/releases/release1.6 ./release1.6

../GiBUUInstall/buuinput:
	mkdir -p ../GiBUUInstall;
	cd ../GiBUUInstall; svn co http://gibuu.hepforge.org/svn/releases/buuinput1.6 ./buuinput

../GiBUUInstall/release1.6/InitialStateInfo.patched: ../GiBUUInstall/release1.6
	cd ../GiBUUInstall; patch -p0 < ../AddInitStateInfo_fixed.patch
	touch $@


../GiBUUInstall/release1.6/bin/gibuu: ../GiBUUInstall/release1.6/InitialStateInfo.patched ../GiBUUInstall/buuinput
	cd ../GiBUUInstall/release1.6; $(MAKE)
	cd ../GiBUUInstall/release1.6; mkdir -p bin; cp objects/GiBUU.x bin/gibuu;
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built Patched GiBUU!"
	@echo "*********************************************************************"

SlaveGiBUU:../GiBUUInstall/release1.6/bin/gibuu

$(TARGET): $(TARGETSRC) $(HF) $(OBJS) $(LHPCLIB) $(LIBUTILS)
	$(CXX) $< -o $@ $(CXXFLAGS) -I$(UTILSLOC) -I$(LHPCINC) $(OBJS) $(LHPCLD) $(LIBUTILSLD) $(LDFLAGS)

%.o : %.cxx $(HF)
	$(CXX) $(CXXFLAGS) -I$(UTILSLOC) -o $@ -c $<

$(LIBUTILS):
	cd $(UTILSLOC); $(MAKE)

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

distclean: clean
	cd $(LHPCROOT); $(MAKE) clean
	cd $(UTILSLOC); $(MAKE) clean
	rm -rf ../GiBUUInstall;
