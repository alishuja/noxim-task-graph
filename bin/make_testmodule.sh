#/bin/sh
g++ -I. -I.. -I../src -L. -L.. -L..src -Wall -fPIC -c ../src/testmodule.cpp 
g++ -shared -Wl,-soname,testmodule.so -o testmodule.so testmodule.o
