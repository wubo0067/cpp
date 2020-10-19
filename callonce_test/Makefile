TARGET		= callonce
SRCS		= test.cpp
OBJS     	= test.o
TARGET_OBJS	= $(OBJS)
#
CPPFLAGS    += -I../../
LDFLAGS     += ../../bazel-bin/absl/base/libspinlock_wait.so ../../bazel-bin/absl/base/libbase.so
DN_LIBS		= -Wl,-Bstatic -lspinlock_wait -lbase
DY_LIBS		= -Wl,-Bdynamic -Wl,--eh-frame-hdr -rdynamic -ldl -lm -lc -lz -lrt -lpthread
CFLAGS		+= -std=c++11

.PHONY : all clean depend

all : depend $(TARGET)

-include .depend

$(TARGET) : $(TARGET_OBJS)
	g++ $(LDFLAGS) $(DY_LIBS) -o $@ $^ $(LIBS)

%.o : %.cpp
	g++ $(CFLAGS) $(CPPFLAGS) -c $< -o $@

depend :
	g++ -MM $(CFLAGS) $(CPPFLAGS) $(SRCS) > .depend
	@-(./replace.sh)

clean:
	@-(rm *.o $(TARGET) .depend)