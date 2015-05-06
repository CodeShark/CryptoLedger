#pragma once

#include "DBModel.h"

#include <leveldb/db.h>

#include <stdexcept>

namespace HashTrie
{

class LevelDBModel : public DBModel
{
public:
    LevelDBModel() : DBModel(), db_(nullptr) { }
    ~LevelDBModel();

    void open(const std::string& dbname);
    void close();
    bool isOpen() const { return (db_ != nullptr); }

    void insert(const bytes_t& key, const bytes_t& value);
    void remove(const bytes_t& key);
    void get(const bytes_t& key, bytes_t& value) const;

private:
    leveldb::DB* db_;
};

}
