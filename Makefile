INCLUDE_PATH += -Isrc

include mk/os.mk mk/cxx_flags.mk

LIBS = \
    -lleveldb \
    -lcrypto

OBJS = \
    obj/LevelDBModel.o

TESTS = \
    build/leveldbmodel$(EXE_EXT) \
    build/hashtrie$(EXE_EXT)

all: $(TESTS)

obj/LevelDBModel.o: src/LevelDBModel.cpp src/LevelDBModel.h src/DBModel.h
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) -c $< -o $@

build/leveldbmodel$(EXE_EXT): src/TestLevelDBModel.cpp obj/LevelDBModel.o
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) $< obj/LevelDBModel.o -o $@ $(LIBS)

build/hashtrie$(EXE_EXT): src/TestHashTrie.cpp obj/LevelDBModel.o src/HashTrie.h
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) $< obj/LevelDBModel.o -o $@ $(LIBS)

clean:
	-rm -f $(TESTS) $(OBJS)
