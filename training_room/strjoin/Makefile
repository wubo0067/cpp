TARGET		= strjoin
SRCS		= test.cpp
OBJS     	= test.o
TARGET_OBJS	= $(OBJS)
#
CPPFLAGS    += -I../../
LDFLAGS     += 
DN_LIBS		+= 
CFLAGS		+= -std=c++11

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