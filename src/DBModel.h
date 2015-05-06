#pragma once

#include <CoinCore/typedefs.h>

namespace HashTrie
{

class DBModel
{
public:
    DBModel() { }
    virtual ~DBModel() = 0;

    virtual void open(const std::string& dbname) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    virtual void insert(const bytes_t& key, const bytes_t& value) = 0;
    virtual void remove(const bytes_t& key) = 0;
    virtual void get(const bytes_t& key, bytes_t& value) const = 0; 
};

}
