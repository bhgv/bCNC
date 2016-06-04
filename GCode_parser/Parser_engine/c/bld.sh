#!/bin/sh

#echo `python-config --cflags`
#echo `python-config --ldflags`

#export DBG="-g"
export DBG=""

export C_FLAGS="-c -fPIC \
-I/usr/include/python2.7 -I/usr/include/arm-linux-gnueabihf/python2.7 \
-fno-strict-aliasing -D_FORTIFY_SOURCE=2 \
-Wformat -Werror=format-security -DNDEBUG -fwrapv -O3 \
"
#-march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -marm \
#-fstack-protector-strong \

export LD_FLAGS="-fPIC -shared \
-L/usr/lib/python2.7/config-arm-linux-gnueabihf -L/usr/lib -lpython2.7 \
-lpthread -ldl -lutil -lm -Xlinker -export-dynamic \
-Wl,-O3 -Wl,-Bsymbolic-functions \
"

tools/coco_cpp/Coco -frames $PWD/tools/coco_cpp GCode.atg

g++ $C_FLAGS $DBG \
-o GCodemodule.o GCodemodule.cpp 

g++ $C_FLAGS $DBG \
-o Parser.o Parser.cpp

g++ $C_FLAGS $DBG \
-o Scanner.o Scanner.cpp

g++ $C_FLAGS $DBG \
-o GCode_foos.o GCode_foos.cpp

g++ $LD_FLAGS $DBG \
-o GCodemodule.so GCodemodule.o Parser.o Scanner.o GCode_foos.o 

#-march=armv7-a -mtune=cortex-a7 -mfpu=neon-vfpv4 -marm \
