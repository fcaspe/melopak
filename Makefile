CPPFLAGS = -g -Wall -O2

LIBS = -lasound -lpthread

MGEN_SOURCE_DIR = ./src/melogen/
MGEN_OBJS = melogen.o sequencer.o midi_output.o
MGEN_BIN = melogen

MPAK_SOURCE_DIR = ./src/melopak/
MPAK_OBJS = melopak.o packer.o
MPAK_BIN = melopak
BIN_DIR = ./bin/

INCLUDE_DIR = ./src/libs/

all: melogen melopak
	mkdir -p $(BIN_DIR)
	mv melopak melogen $(BIN_DIR)
	rm -f $(MGEN_OBJS) $(MPAK_OBJS)

melogen: $(MGEN_OBJS)
	g++ -o $(MGEN_BIN) $(MGEN_OBJS) $(LIBS)

melogen.o: $(MGEN_SOURCE_DIR)/melogen.cpp
	g++ $(CPPFLAGS) -I $(INCLUDE_DIR) -c $(MGEN_SOURCE_DIR)/melogen.cpp

sequencer.o: $(MGEN_SOURCE_DIR)/sequencer.cpp
	g++ $(CPPFLAGS) -I $(INCLUDE_DIR) -c $(MGEN_SOURCE_DIR)/sequencer.cpp

midi_output.o: $(MGEN_SOURCE_DIR)/midi_output.cpp
	g++ $(CPPFLAGS) -I $(INCLUDE_DIR) -c $(MGEN_SOURCE_DIR)/midi_output.cpp


melopak: $(MPAK_OBJS)
	g++ -o $(MPAK_BIN) $(MPAK_OBJS) $(LIBS)

melopak.o: $(MPAK_SOURCE_DIR)/melopak.cpp
	g++ $(CPPFLAGS) -I $(INCLUDE_DIR) -c $(MPAK_SOURCE_DIR)/melopak.cpp

packer.o: $(MPAK_SOURCE_DIR)/packer.cpp
	g++ $(CPPFLAGS) -I $(INCLUDE_DIR) -c $(MPAK_SOURCE_DIR)/packer.cpp

clean:
	rm -f $(BIN_DIR)$(MGEN_BIN) $(MGEN_OBJS)
	rm -f $(BIN_DIR)$(MPAK_BIN) $(MPAK_OBJS)

