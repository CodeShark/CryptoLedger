INCLUDE_PATH += -Isrc

include mk/os.mk mk/cxx_flags.mk

LIBS = \
    -lleveldb

OBJS = \
    obj/LevelDBModel.o

TESTS = \
    build/leveldbmodel$(EXE_EXT)

all: $(TESTS)

obj/LevelDBModel.o: src/LevelDBModel.cpp src/LevelDBModel.h src/DBModel.h
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) -c $< -o $@

build/leveldbmodel$(EXE_EXT): src/TestLevelDBModel.cpp obj/LevelDBModel.o
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) $< $(OBJS) -o $@ $(LIBS)

clean:
	-rm -f $(TESTS) $(OBJS)
