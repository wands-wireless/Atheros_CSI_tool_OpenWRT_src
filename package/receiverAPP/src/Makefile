#INCLUDE_DIR=$(TOOLCHAIN_DIR)/usr/include

PROFILE = -O2 -s
#CC=mips-openwrt-linux-gcc

CFLAGS = $(PROFILE)

OBJS = csi_func.o client_main.o

receiverAPP: $(OBJS)
	$(CC) $(OBJS) -o receiverAPP

csi_func.o: csi_func.c csi_func.h
	$(CC) -c $(CFLAGS) -I -o $@ $<

client_main.o: client_main.c csi_func.h
	$(CC) -c $(CFLAGS) -I -o $@ $<

clean: 
	rm -f *.o client_main
#all: client_main
#client_main: client_main.o
#client_main.o: client_main.c
