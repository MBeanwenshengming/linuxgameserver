OBJECTSDIR=./obj
CURDIR=.
BINOBJDIR=./bin

SOURCES = $(wildcard *.cpp) 
OBJECTS_debug = $(patsubst %.cpp,$(OBJECTSDIR)/%_d.o,$(SOURCES)) 
OBJECTS_Release = $(patsubst %.cpp,$(OBJECTSDIR)/%_r.o,$(SOURCES))

DEPSHEADER=-I../../mysql/include -I../NetWork
DEPSLIB=-L../../mysql/lib -lmysqlclient

deps = $(patsubst %.cpp,$(OBJECTSDIR)/%.d,$(SOURCES))

CPPDEBUGFLAGS = -Di386 -g $(DEPSHEADER)
CPPRELEASEFLAGS = -Di386 $(DEPSHEADER) 
LinkFlagsD = -lpthread -pg -g $(DEPSLIB)
LinkFlags = -lpthread -o2 $(DEPSLIB) 

All:$(BINOBJDIR)/libDBLib_debug.a $(BINOBJDIR)/libDBLib_Release.a

$(OBJECTSDIR)/%.d:%.cpp 
	@echo $(join $*,.d Building) \ 
	@set -e; \
	rm -f $@; \
	g++ -M $< > $@.$$$$ $(DEPSHEADER); \
	sed 's,\($*\)\.o[ :]*,\$(OBJECTSDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$  

sinclude	$(deps) 

$(BINOBJDIR)/libDBLib_debug.a:$(OBJECTS_debug)
	@set -e; \
	rm -f $@; \
	ar rcs $@ $(OBJECTS_debug)

$(BINOBJDIR)/libDBLib_Release.a:$(OBJECTS_Release)
	@set -e; \
	rm -f $@; \
	ar rcs $@ $(OBJECTS_Release)

$(OBJECTSDIR)/%_d.o:%.cpp 
	@echo $(join $*,.cpp Building)
	@g++ -o $@ -c $< $(CPPDEBUGFLAGS) 
	
$(OBJECTSDIR)/%_r.o:%.cpp 
	@echo $(join $*,.cpp Building)
	@g++ -o $@ -c $< $(CPPRELEASEFLAGS) 
		
clean:
	rm -f $(OBJECTSDIR)/*.o $(OBJECTSDIR)/*.d $(BINOBJDIR)/libDBLib_debug.a $(BINOBJDIR)/libDBLib_Release.a 
