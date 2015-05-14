INCLUDE_PATH += -Isrc

include mk/os.mk mk/cxx_flags.mk

LIBS = \
    -lleveldb \
    -lcrypto

OBJS = \
    obj/LevelDBModel.o

TESTS = \
    build/leveldbmodel$(EXE_EXT) \
    build/hashtrie$(EXE_EXT) \
    build/txouttree$(EXE_EXT)

all: lib/libCryptoLedger.a $(TESTS)

obj/LevelDBModel.o: src/LevelDBModel.cpp src/LevelDBModel.h src/DBModel.h
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) -c $< -o $@

build/leveldbmodel$(EXE_EXT): src/TestLevelDBModel.cpp obj/LevelDBModel.o
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) $< obj/LevelDBModel.o -o $@ $(LIBS)

build/hashtrie$(EXE_EXT): src/TestHashTrie.cpp obj/LevelDBModel.o src/HashTrie.h
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) $< obj/LevelDBModel.o -o $@ $(LIBS)

build/txouttree$(EXE_EXT): src/TestTxOutTree.cpp obj/LevelDBModel.o src/TxOutTree.h src/HashTrie.h
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) $< obj/LevelDBModel.o -o $@ $(LIBS)

lib/libCryptoLedger.a: $(OBJS)
	$(ARCHIVER) rcs $@ $^

install:
	-mkdir -p $(SYSROOT)/include/CryptoLedger
	-rsync -u src/*.h $(SYSROOT)/include/CryptoLedger/
	-mkdir -p $(SYSROOT)/lib
	-rsync -u lib/libCryptoLedger.a $(SYSROOT)/lib/

remove:
	-rm -rf $(SYSROOT)/include/CryptoLedger
	-rm $(SYSROOT)/lib/libCryptoLedger.a

clean:
	-rm -f lib/libCryptoLedger.a $(TESTS) $(OBJS)
