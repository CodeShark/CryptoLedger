#include <iostream>

#include "HashTrie.h"
#include "LevelDBModel.h"

using namespace CryptoLedger;
using namespace std;

void showPath(const vector<bool>& path)
{
    for (auto b: path) { cout << (b ? "R" : "L"); }
    cout << endl;
}

int main(int argc, char* argv[])
{
    try
    {
        MMRTree<LevelDBModel> tree("TestTree");

        if (argc > 1)
        {
            if (string(argv[1]) == "p")
            {
                if (argc != 3) throw runtime_error("No item index specified for option p.");
                uint64_t i = strtoull(argv[2], NULL, 0);
                vector<bool> path = tree.path(i);
                showPath(path);
                return 0;
            }
            
            for (int i = 1; i < argc; i++)
            {
                if (string(argv[i]) == "-") { tree.removeItem(); }
                else                        { tree.appendItem(uchar_vector(argv[i])); }
            }

            tree.commit();
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
