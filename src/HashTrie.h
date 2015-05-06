#pragma once

#include <CoinCore/typedefs.h>

#include <stdexcept>

namespace HashTrie
{

template<typename DBModelType, typename KeyType = bytes_t, typename ValueType = bytes_t>
class HashTrie
{
public:
    HashTrie(const std::string& dbname);
    ~HashTrie() { dbModel_.close(); }

    const KeyType& rootHash() const { return rootHash_; } 

private:
    DBModelType dbModel_;
    KeyType rootHash_;
};


template<typename DBModelType, typename KeyType, typename ValueType>
HashTrie<DBModelType, KeyType, ValueType>::HashTrie(const std::string& dbname)
{
    dbModel_.open(dbname);
    try
    {
        dbModel_.get(bytes_t(), rootHash_);
    }
    catch (...)
    {
        rootHash_.clear();
        dbModel_.insert(bytes_t(), rootHash_);
    }
}

}
