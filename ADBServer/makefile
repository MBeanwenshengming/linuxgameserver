OBJECTSDIR=./obj
CURDIR=.
BINOBJDIR=./bin

SOURCES = $(wildcard *.cpp) 
OBJECTS_debug = $(patsubst %.cpp,$(OBJECTSDIR)/%_d.o,$(SOURCES)) 
OBJECTS_Release = $(patsubst %.cpp,$(OBJECTSDIR)/%_r.o,$(SOURCES))

DEPSHEADER=-I../NetWork -I../include `mysql_config --cflags`
DEPSLIB=-L../NetWork/bin -L../DBLib/bin -L../DBLib/DBLib_Debug  -L../NetWork/NetWork_Debug -L../NetWork/NetWork_Release -L/usr/lib/mysql -lmysqlclient -lpthread -lm -lrt -ldl

deps = $(patsubst %.cpp,$(OBJECTSDIR)/%.d,$(SOURCES))

CPPDEBUGFLAGS = -Di386 -g $(DEPSHEADER)
CPPRELEASEFLAGS = -Di386 $(DEPSHEADER) 
LinkFlagsD = -lpthread -pg -g $(DEPSLIB) -lNetWork_Debug -lDBLib_Debug
LinkFlags = -lpthread -o2 $(DEPSLIB) -lNetWork_Release -lDBLib_Release

All:$(BINOBJDIR)/ADBServer_debug $(BINOBJDIR)/ADBServer_Release

$(OBJECTSDIR)/%.d:%.cpp 
	@echo $(join $*,.d Building) \ 
	@set -e; \
	rm -f $@; \
	g++ -M $< > $@.$$$$ $(DEPSHEADER); \
	sed 's,\($*\)\.o[ :]*,\$(OBJECTSDIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$  

sinclude	$(deps) 

$(BINOBJDIR)/ADBServer_debug:$(OBJECTS_debug)
	@set -e; \
	rm -f $@; \
	g++ -o $@ $(OBJECTS_debug) $(LinkFlagsD)

$(BINOBJDIR)/ADBServer_Release:$(OBJECTS_Release)
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
	rm -f $(OBJECTSDIR)/*.o $(OBJECTSDIR)/*.d $(BINOBJDIR)/ADBServer_debug $(BINOBJDIR)/ADBServer_Release 
