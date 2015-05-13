#include <iostream>

#include "HashTrie.h"
#include "LevelDBModel.h"

using namespace HashTrie;
using namespace std;

int main(int argc, char* argv[])
{
/*
    if (argc != 2)
    {
        cerr << "# Usage: " << argv[0] << " <hex data>" << endl;
        return -1;
    }
*/

    try
    {
        MMRTree<LevelDBModel> tree("TestTree");

        if (argc > 1)
        {
            tree.appendItem(uchar_vector(argv[1]));
        }

        cout << tree.json() << endl;
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return -2;
    }

    return 0;
}
