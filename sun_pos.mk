CC=g++
TRACKINGLFLAGS=-lpthread
IDLLFLAGS=-L /usr/local/idl/idl706/bin/bin.linux.x86_64 -lpthread -lidl -lXp -lXpm -lXmu -lXext -lXt -lSM -lICE  -lXinerama -lX11 -ldl -ltermcap -lrt -lm /usr/lib/libXm.a
MAGICKLFLAGS=`Magick++-config --ldflags --libs`
MAGICKCFLAGS=`Magick++-config --cppflags`
CFLAGS=-Wall -fkeep-inline-functions -g -O3
LFLAGS=-lcfitsio
DFLAGS=-D DEBUG=4

all:bin/sun_pos.x
clean: rm bin/sun_pos.x objects/sun_pos.o objects/FitsFile.o objects/Header.o objects/tools.o


bin/sun_pos.x : sun_pos.mk objects/sun_pos.o objects/FitsFile.o objects/Header.o objects/tools.o
	$(CC) $(CFLAGS) $(DFLAGS) objects/sun_pos.o objects/FitsFile.o objects/Header.o objects/tools.o $(LFLAGS) -o bin/sun_pos.x

objects/sun_pos.o : sun_pos.mk ./sun_pos.cpp classes/constants.h classes/FitsFile.h
	$(CC) -c $(CFLAGS) $(DFLAGS) ./sun_pos.cpp -o objects/sun_pos.o

objects/FitsFile.o : sun_pos.mk classes/FitsFile.cpp classes/fitsio.h classes/longnam.h classes/tools.h classes/constants.h classes/Header.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/FitsFile.cpp -o objects/FitsFile.o

objects/Header.o : sun_pos.mk classes/Header.cpp 
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/Header.cpp -o objects/Header.o

objects/tools.o : sun_pos.mk classes/tools.cpp classes/constants.h
	$(CC) -c $(CFLAGS) $(DFLAGS) classes/tools.cpp -o objects/tools.o
