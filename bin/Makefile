TARGET_ARCH = linux64
CC     = g++
OPT    = -O3
DEBUG  = -g
OTHER  = # -Wall -Wno-deprecated
CFLAGS = $(OPT) $(OTHER) # $(DEBUG)

VPATH = ../src:.
MODULE = noxim
SRCS = ./NoximNoC.cpp ./NoximRouter.cpp ./NoximProcessingElement.cpp ./NoximBuffer.cpp \
	./NoximStats.cpp ./NoximGlobalStats.cpp ./NoximGlobalRoutingTable.cpp \
	./NoximLocalRoutingTable.cpp ./NoximGlobalTrafficTable.cpp ./NoximGlobalTaskGraph.cpp ./NoximReservationTable.cpp \
	./NoximPower.cpp ./NoximCmdLineParser.cpp ./NoximMain.cpp
OBJS = $(SRCS:.cpp=.o)

include ./Makefile.defs
