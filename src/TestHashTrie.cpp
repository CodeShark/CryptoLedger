#include <iostream>

#include "HashTrie.h"
#include "LevelDBModel.h"

using namespace HashTrie;
using namespace std;

int main(int argc, char* argv[])
{
    HashTrie<LevelDBModel> hashTrie("roar");
    return 0;
}
