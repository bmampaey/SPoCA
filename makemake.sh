#!/usr/bin/env bash

if [ -z "$CFITSIO_HOME" ]; then
	CFITSIO_HOME=/usr/local
fi

CPPFLAGS="-I$CFITSIO_HOME/include `Magick++-config --cppflags | tr -d '\n'` -DMAGICK"
CXXFLAGS="-pipe -fPIC -fkeep-inline-functions -g -O3 ${CPPFLAGS}"
LDFLAGS="-L$CFITSIO_HOME/lib `Magick++-config --ldflags --libs | tr -d '\n'` -lcfitsio -lpthread -Llib"

BINARIES1=`echo programs/*.cpp | sed "s/programs\/\([^ ]*\)\.cpp/bin1\/\1.x/g"`
BINARIES2=`echo programs/*.cpp | sed "s/programs\/\([^ ]*\)\.cpp/bin2\/\1.x/g"`

OBJECTS1=`echo classes/*.cpp | sed "s/classes\/\([^ ]*\)\.cpp/classes\/objects1\/\1.o/g"`
OBJECTS2=`echo classes/*.cpp | sed "s/classes\/\([^ ]*\)\.cpp/classes\/objects2\/\1.o/g"`

echo "all: onechannel twochannels"
echo "onechannel: lib/libSPoCA1.so ${BINARIES1}"
echo "twochannels: lib/libSPoCA2.so ${BINARIES2}"

echo "# This blank rule prevents make from deleting intermediary object files"
echo ".SECONDARY:"
echo "	"

echo "strip: all"
echo "	strip lib/*.so bin{1,2}/*.x"
echo "clean:"
echo "	rm lib/*.so classes/objects{1,2}/*.o bin{1,2}/* programs/objects{1,2}/*.o"

echo "bin2/%.x: programs/objects2/%.o lib/libSPoCA2.so"
echo "	g++ -o \$@ ${LDFLAGS} -lSPoCA2 programs/objects2/\$*.o"
echo "bin1/%.x: programs/objects1/%.o lib/libSPoCA1.so"
echo "	g++ -o \$@ ${LDFLAGS} -lSPoCA1 programs/objects1/\$*.o"

echo "lib/libSPoCA2.so: ${OBJECTS2}"
echo "	g++ -o \$@ -shared ${LDFLAGS} \$?"
echo "lib/libSPoCA1.so: ${OBJECTS1}"
echo "	g++ -o \$@ -shared ${LDFLAGS} \$?"

for numberchannels in 1 2; do
	for directory in classes programs; do
		for module in `echo $directory/*.cpp | sed "s/$directory\/\([^ ]*\)\.cpp/\1/g"`; do
			cpp -MM -DNUMBERCHANNELS=$numberchannels -MT $directory/objects$numberchannels/$module.o $CPPFLAGS $directory/$module.cpp
			echo "	g++ -o \$@ -c $CXXFLAGS -DNUMBERCHANNELS=$numberchannels $directory/$module.cpp"
		done
	done
done
