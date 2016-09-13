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

UTILSLOC := ../utils/build/`uname`
UTILSINC := $(UTILSLOC)/include
LIBUTILSLD := -L$(UTILSLOC)/lib  -lLUtils

.PHONY: all clean distclean

all: $(TARGET) BinEdgeToBinCenterPDFFlux.exe
	mkdir -p $(BDIR)
	mv $(TARGET) BinEdgeToBinCenterPDFFlux.exe $(BDIR)/
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built GiBUUTools."
	@echo "*********************************************************************"

../GiBUUInstall/release2016:
	mkdir -p ../GiBUUInstall;
	cd ../GiBUUInstall; svn co -r 6910 http://gibuu.hepforge.org/svn/releases/release2016 ./release2016

../GiBUUInstall/buuinput_2016:
	mkdir -p ../GiBUUInstall;
	cd ../GiBUUInstall; svn co http://gibuu.hepforge.org/svn/releases/buuinput2016 ./buuinput_2016

../GiBUUInstall/release2016/GiBUU2016_ArbFlux_ProdCharg.patched: ../GiBUUInstall/release2016
	cd ../GiBUUInstall; patch -p1 --ignore-whitespace < ../GiBUU2016_ArbFlux_ProdCharg.patch
	touch $@

../GiBUUInstall/release2016/bin/gibuu: ../GiBUUInstall/release2016/GiBUU2016_ArbFlux_ProdCharg.patched ../GiBUUInstall/buuinput_2016
	cd ../GiBUUInstall/release2016; $(MAKE)
	cd ../GiBUUInstall/release2016; mkdir -p bin; cp objects/GiBUU.x bin/gibuu;
	@echo ""
	@echo "*********************************************************************"
	@echo "Success. Built Patched GiBUU 2016!"
	@echo "*********************************************************************"
	@echo "To begin using this please re-source setup.sh"
	@echo "Then test that gibuu is where you expect it by $ which gibuu"

SlaveGiBUU_2016:../GiBUUInstall/release2016/bin/gibuu

$(TARGET): $(TARGETSRC) $(HF) $(OBJS)
	$(CXX) $< -o $@ $(CXXFLAGS) -I$(UTILSINC) $(OBJS) $(LIBUTILSLD) $(LDFLAGS)

BinEdgeToBinCenterPDFFlux.exe: BinEdgeToBinCenterPDFFlux.cxx
	$(CXX) $< -o $@ $(CXXFLAGS) -I$(UTILSINC) $(LIBUTILSLD) $(LDFLAGS)

%.o : %.cxx $(HF)
	$(CXX) $(CXXFLAGS) -I$(UTILSINC) -o $@ -c $<

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
	rm -rf ../GiBUUInstall;
