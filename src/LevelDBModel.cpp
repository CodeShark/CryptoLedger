#include "LevelDBModel.h"

using namespace leveldb;
using namespace std;

using namespace HashTrie;

LevelDBModel::~LevelDBModel()
{
    close();
}

void LevelDBModel::open(const string& dbname)
{
    if (db_) throw runtime_error("DB is already open.");

    Options options;
    options.create_if_missing = true;
    Status status = DB::Open(options, dbname, &db_);
    if (!status.ok()) throw runtime_error(status.ToString()); 
}

void LevelDBModel::close()
{
    if (db_)
    {
        delete db_;
        db_ = nullptr;
    }
}

void LevelDBModel::insert(const bytes_t& key, const bytes_t& value)
{
    if (!db_) throw runtime_error("DB is not open.");

    Status status = db_->Put(WriteOptions(), Slice(string(reinterpret_cast<const char*>(&key[0]), key.size())), Slice(string(reinterpret_cast<const char*>(&value[0]), value.size())));
    if (!status.ok()) throw runtime_error(status.ToString()); 
}

void LevelDBModel::remove(const bytes_t& key)
{
    if (!db_) throw runtime_error("DB is not open.");

    Status status = db_->Delete(WriteOptions(), Slice(string(reinterpret_cast<const char*>(&key[0]), key.size())));
    if (!status.ok()) throw runtime_error(status.ToString()); 
}

void LevelDBModel::get(const bytes_t& key, bytes_t& value) const
{
    if (!db_) throw runtime_error("DB is not open.");

    auto it = insertionMap_.find(key);
    if (it == insertionMap_.end())
    {
        string strvalue;
        Status status = db_->Get(ReadOptions(), Slice(string(reinterpret_cast<const char*>(&key[0]), key.size())), &strvalue);
        if (!status.ok()) throw runtime_error(status.ToString());

        value.assign(strvalue.begin(), strvalue.end());
    }
    else
    {
        value = it->second;
    }
}

void LevelDBModel::batchInsert(const bytes_t& key, const bytes_t& value)
{
    insertionMap_[key] = value;
    updates_.Put(Slice(string(reinterpret_cast<const char*>(&key[0]), key.size())), Slice(string(reinterpret_cast<const char*>(&value[0]), value.size())));
}

void LevelDBModel::batchRemove(const bytes_t& key)
{
    insertionMap_.erase(key);
    updates_.Delete(Slice(string(reinterpret_cast<const char*>(&key[0]), key.size())));
}

void LevelDBModel::commit()
{
    if (!db_) throw runtime_error("DB is not open.");

    Status status = db_->Write(WriteOptions(), &updates_);
    if (!status.ok()) throw runtime_error(status.ToString());

    insertionMap_.clear();
}

void LevelDBModel::rollback()
{
    insertionMap_.clear();
    updates_.Clear();
}

