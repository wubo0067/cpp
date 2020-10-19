TARGET		= uniptr
SRCS		= uniptr.cpp
OBJS     	= uniptr.o
TARGET_OBJS	= $(OBJS)
#

CFLAGS		+= -std=c++17

.PHONY : all clean depend

all : depend $(TARGET)

-include .depend

$(TARGET) : $(TARGET_OBJS)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o : %.cpp
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

depend :
	$(CXX) -MM $(CFLAGS) $(CPPFLAGS) $(SRCS) > .depend
	@-(./replace.sh)

clean:
	@-(rm *.o $(TARGET) .depend)