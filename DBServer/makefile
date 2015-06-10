OBJECTSDIR=./obj
CURDIR=.
BINOBJDIR=./bin

SOURCES = $(wildcard *.cpp) 
OBJECTS_debug = $(patsubst %.cpp,$(OBJECTSDIR)/%_d.o,$(SOURCES)) 
OBJECTS_Release = $(patsubst %.cpp,$(OBJECTSDIR)/%_r.o,$(SOURCES))

DEPSHEADER=-I../../mysql/include -I../NetWork -I../include
DEPSLIB=-L../../mysql/lib -lmysqlclient -L../NetWork/bin -L../DBLib/bin

deps = $(patsubst %.cpp,$(OBJECTSDIR)/%.d,$(SOURCES))

CPPDEBUGFLAGS = -Di386 -g $(DEPSHEADER)
CPPRELEASEFLAGS = -Di386 $(DEPSHEADER) 
LinkFlagsD = -lpthread -pg -g $(DEPSLIB) -lNetWork_debug -lDBLib_debug
LinkFlags = -lpthread -o2 $(DEPSLIB) -lNetWork_Release -lDBLib_Release

All:$(BINOBJDIR)/DBServer_debug $(BINOBJDIR)/DBServer_Release

$(OBJECTSDIR)/%.d:%.cpp 
	@echo $(join $*,.d Building) \ 
	@set -e; \
	rm -f $@; \
	g++ -M $< > $@.$$$$ $(DEPSHEADER); \
	sed 's,\($*\)\.o[ :]*,\$(OBJECTSDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$  

sinclude	$(deps) 

$(BINOBJDIR)/DBServer_debug:$(OBJECTS_debug)
	@set -e; \
	rm -f $@; \
	g++ -o $@ $(OBJECTS_debug) $(LinkFlagsD)

$(BINOBJDIR)/DBServer_Release:$(OBJECTS_Release)
	@set -e; \
	rm -f $@; \
	g++ -o $@ $(OBJECTS_Release) $(LinkFlags)

$(OBJECTSDIR)/%_d.o:%.cpp 
	@echo $(join $*,.cpp Building)
	@g++ -o $@ -c $< $(CPPDEBUGFLAGS) 
	
$(OBJECTSDIR)/%_r.o:%.cpp 
	@echo $(join $*,.cpp Building)
	@g++ -o $@ -c $< $(CPPRELEASEFLAGS) 
		
clean:
	rm -f $(OBJECTSDIR)/*.o $(OBJECTSDIR)/*.d $(BINOBJDIR)/DBServer_debug $(BINOBJDIR)/DBServer_Release 
