# Cross-compilation support
CROSS_COMPILE ?=
CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra -g

TARGET = aesdsocket
INIT_SCRIPT = aesdsocket-start-stop

all: $(TARGET)

default: all

$(TARGET): aesdsocket.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install: $(TARGET) $(INIT_SCRIPT)
	install -m 0755 $(TARGET) /usr/bin/
	install -m 0755 $(INIT_SCRIPT) /etc/init.d/
	update-rc.d $(INIT_SCRIPT) defaults

clean:
	rm -f $(TARGET) *.o

uninstall:
	rm -f /usr/bin/$(TARGET)
	rm -f /etc/init.d/$(INIT_SCRIPT)
	update-rc.d $(INIT_SCRIPT) remove

.PHONY: all default clean install uninstall