OBJECTSDIR=./obj
CURDIR=.
BINOBJDIR=./bin

SOURCES = $(wildcard *.cpp) 
OBJECTS_debug = $(patsubst %.cpp,$(OBJECTSDIR)/%_d.o,$(SOURCES)) 
OBJECTS_Release = $(patsubst %.cpp,$(OBJECTSDIR)/%_r.o,$(SOURCES))

deps = $(patsubst %.cpp,$(OBJECTSDIR)/%.d,$(SOURCES))

CPPDEBUGFLAGS = -Di386 -g
CPPRELEASEFLAGS = -Di386
LinkFlagsD = -lpthread -pg -g 
LinkFlags = -lpthread -o2

All:$(BINOBJDIR)/libNetWork_debug.a $(BINOBJDIR)/libNetWork_Release.a

$(OBJECTSDIR)/%.d:%.cpp 
	@echo $(join $*,.d Building) \ 
	@set -e; \
	rm -f $@; \
	g++ -M $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\$(OBJECTSDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$  

sinclude	$(deps) 

$(BINOBJDIR)/libNetWork_debug.a:$(OBJECTS_debug)
	@set -e; \
	rm -f $@; \
	ar rcs $@ $(OBJECTS_debug)

$(BINOBJDIR)/libNetWork_Release.a:$(OBJECTS_Release)
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
	rm -f $(OBJECTSDIR)/*.o $(OBJECTSDIR)/*.d $(BINOBJDIR)/libNetWork_debug.a $(BINOBJDIR)/libNetWork_Release.a 
