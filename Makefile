INCLUDE_PATH += -Isrc

include mk/os.mk mk/cxx_flags.mk

LIBS = \
    -lleveldb

OBJS = \
    obj/LevelDBModel.o

all: $(OBJS)

obj/LevelDBModel.o: src/LevelDBModel.cpp src/LevelDBModel.h src/DBModel.h
	$(CXX) $(CXX_FLAGS) $(INCLUDE_PATH) -c $< -o $@
