SOURCES = $(wildcard *.cpp) 
OBJECTS = $(patsubst %.cpp,%.o,$(SOURCES)) 

CPPDEBUGFLAGS = -Di386 -g -I../NetWork
LinkFlagsD = -lpthread -pg -g -lNetWork -L../NetWork


All:Connector


%.d:%.cpp 
	@set -e; \
	rm -f $@; \
	g++ -M $(CPPDEBUGFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$  

sinclude $(SOURCES:.cpp=.d)


Connector:$(OBJECTS)
	@g++ -o Connector $(OBJECTS) $(LinkFlagsD)

%.o:%.cpp 
	@echo $(join $*,.cpp Building) 
	@g++ -o $@ -c $< $(CPPDEBUGFLAGS) 
	
clean:
	rm -f *.o *.d Connector



