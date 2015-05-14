#include <iostream>

#include "TxOutTree.h"
#include "LevelDBModel.h"

#include <stdutils/stringutils.h>

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
        TxOutTree<LevelDBModel> tree("TestTree");

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
                else
                {
                    vector<string> txoutFields;
                    stdutils::explode(string(argv[i]), ',', back_inserter(txoutFields));
                    if (txoutFields.size() != 7) throw runtime_error("Invalid txout.");
                    uchar_vector txhash(txoutFields[0]);
                    uint32_t txindex = strtoul(txoutFields[1].c_str(), NULL, 0);
                    uint32_t version = strtoul(txoutFields[2].c_str(), NULL, 0);
                    uint64_t height = strtoull(txoutFields[3].c_str(), NULL, 0);
                    bool isCoinBase = (txoutFields[4] == "true");
                    bool isSpent = (txoutFields[5] == "true");
                    uchar_vector script(txoutFields[6]);
                    tree.appendItem(txhash, txindex, TxOutItem(version, height, isCoinBase, isSpent, script));
                }
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
