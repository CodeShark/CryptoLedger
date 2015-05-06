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

    string strvalue;
    Status status = db_->Get(ReadOptions(), Slice(string(reinterpret_cast<const char*>(&key[0]), key.size())), &strvalue);
    if (!status.ok()) throw runtime_error(status.ToString());

    value.assign(strvalue.begin(), strvalue.end());
}
