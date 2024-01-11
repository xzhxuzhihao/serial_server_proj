TARGET = helloworld
OBJECTS = ./libmt7628/serial_config_api.o ./libmt7628/serial_transfer_api.o ./libmt7628/main.o
VPATH = ./libmt7628
#CC = gcc
$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm *.o ./libmt7628/*.o helloworld
