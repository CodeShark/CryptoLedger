#pragma once

#include "DBModel.h"

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <map>
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

    void batchInsert(const bytes_t& key, const bytes_t& value);
    void batchRemove(const bytes_t& key);

    void commit();
    void rollback();

private:
    leveldb::DB* db_;
    leveldb::WriteBatch updates_;
    std::map<bytes_t, bytes_t> insertionMap_;
};

}
