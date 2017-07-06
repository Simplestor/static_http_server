CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)gcc

TARGET=httpserver


LDFLAGS += -lpthread

CFLAGS +=
CFLAGS += -I./common -I./socket

OBJS += main.o http_server.o\
        common/log.o common/lock.o common/wait.o \
        socket/server_socket.o

all:$(TARGET)

$(TARGET):$(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(TARGET)
clean:
	rm -rf $(OBJS) $(TARGET) *.d* \
    rm -rf socket/*.d* socket/*.o* \
    rm -rf common/*.d* common/*.o*

%.o:%.c
	@echo "CC $<"
	@$(CC) -o $@ -c $< $(CFLAGS)
%.d:%.c
	@set -e; rm -f $@; $(CC) -MM $< $(CFLAGS) > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

-include $(OBJS:.o=.d)

