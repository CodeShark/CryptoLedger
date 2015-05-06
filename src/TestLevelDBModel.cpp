#include <iostream>

#include "LevelDBModel.h"

#include <stdutils/uchar_vector.h>

using namespace std;
using namespace HashTrie;

void showHelp(char* progname)
{
    cerr << "# Usage: " << progname << " <db name> [command] [args...]" << endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        showHelp(argv[0]);
        return -1;
    }

    LevelDBModel dbModel;

    try
    {
        dbModel.open(argv[1]);
        if (argc == 2) return 0;

        string command(argv[2]);
        if (command == "insert")
        {
            if (argc != 5)
            {
                showHelp(argv[0]);
                return -1;
            }

            uchar_vector key(argv[3]);
            uchar_vector value(argv[4]);
            dbModel.insert(key, value);
        }
        else if (command == "remove")
        {
            if (argc != 4)
            {
                showHelp(argv[0]);
                return -1;
            }

            uchar_vector key(argv[3]);
            dbModel.remove(key);
        }
        else if (command == "get")
        {
            if (argc != 4)
            {
                showHelp(argv[0]);
                return -1;
            }

            uchar_vector key(argv[3]);
            uchar_vector value;
            dbModel.get(key, value);
            cout << value.getHex() << endl;
        }
        else
        {
            cerr << "Invalid command." << endl;
            return -1;
        }
    }
    catch (const exception& e)
    {
        cerr << "Error: " << e.what() << endl;
        return -2;
    }

    return 0; 
}
