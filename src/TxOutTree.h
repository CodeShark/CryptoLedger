#pragma once

#include "HashTrie.h"

namespace CryptoLedger
{

class TxOutItem
{
public:
    TxOutItem(uint32_t version, uint64_t height, bool isCoinBase, bool isSpent, const bytes_t& script)
        : version_(version), height_(height), isCoinBase_(isCoinBase), isSpent_(isSpent), script_(script) { }

    explicit TxOutItem(const bytes_t& serialized) { setSerialized(serialized); }

    uint32_t version() const { return version_; }
    void setVersion(uint32_t version) { version_ = version; }

    uint64_t height() const { return height_; }
    void setHeight(uint64_t height) { height_ = height; }

    bool isCoinBase() const { return isCoinBase_; }
    void setCoinBase(bool isCoinBase) { isCoinBase_ = isCoinBase; }

    bool isSpent() const { return isSpent_; }
    void setSpent(bool isSpent) { isSpent_ = isSpent; }

    const bytes_t& script() const { return script_; }
    void setScript(const bytes_t& script) { script_ = script; }

    bytes_t getSerialized() const;
    void setSerialized(const bytes_t& serialized);

private:
    uint32_t version_;
    uint64_t height_;
    bool isCoinBase_;
    bool isSpent_;
    bytes_t script_;
};

bytes_t TxOutItem::getSerialized() const
{
    uchar_vector rval;

    // TODO: more compact representation
    rval.push_back(version_ >> 24);
    rval.push_back((version_ >> 16) & 0xff);
    rval.push_back((version_ >> 8) & 0xff);
    rval.push_back(version_ & 0xff);

    rval.push_back(height_ >> 56);
    rval.push_back((height_ >> 48) & 0xff);
    rval.push_back((height_ >> 40) & 0xff);
    rval.push_back((height_ >> 32) & 0xff);
    rval.push_back((height_ >> 24) & 0xff);
    rval.push_back((height_ >> 16) & 0xff);
    rval.push_back((height_ >> 8) & 0xff);
    rval.push_back(height_ & 0xff);

    unsigned char flags = 0x00;
    if (isCoinBase_) { flags |= 0x01; }
    if (isSpent_)    { flags |= 0x02; }
    rval.push_back(flags);

    uint64_t scriptlen = script_.size();
    rval.push_back(scriptlen >> 56);
    rval.push_back((scriptlen >> 48) & 0xff);
    rval.push_back((scriptlen >> 40) & 0xff);
    rval.push_back((scriptlen >> 32) & 0xff);
    rval.push_back((scriptlen >> 24) & 0xff);
    rval.push_back((scriptlen >> 16) & 0xff);
    rval.push_back((scriptlen >> 8) & 0xff);
    rval.push_back(scriptlen & 0xff);
    rval += script_;

    return rval; 
}

void TxOutItem::setSerialized(const bytes_t& serialized)
{
    uint32_t pos = 0;

    if (serialized.size() < pos + 4) throw std::runtime_error("Invalid TxOutItem serialization.");
    version_ = ((uint32_t)serialized[pos] << 24) | ((uint32_t)serialized[pos + 1] << 16) | ((uint32_t)serialized[pos + 2] << 8) | ((uint32_t)serialized[pos + 3]);
    pos += 4;

    if (serialized.size() < pos + 8) throw std::runtime_error("Invalid TxOutItem serialization.");
    height_ = ((uint64_t)serialized[pos] << 56) | ((uint64_t)serialized[pos + 1] << 48) | ((uint64_t)serialized[pos + 2] << 40) | ((uint64_t)serialized[pos + 3] << 32)
            | ((uint64_t)serialized[pos + 4] << 24) | ((uint64_t)serialized[pos + 5] << 16) | ((uint64_t)serialized[pos + 6] << 8) | ((uint64_t)serialized[pos + 7]);
    pos += 8;

    if (serialized.size() < pos + 1) throw std::runtime_error("Invalid TxOutItem serialization.");
    unsigned char flags = serialized[pos];
    isCoinBase_ = flags & 0x01;
    isSpent_ = flags & 0x02;
    pos += 1;

    if (serialized.size() < pos + 8) throw std::runtime_error("Invalid TxOutItem serialization.");
    uint64_t scriptlen = ((uint64_t)serialized[pos] << 56) | ((uint64_t)serialized[pos + 1] << 48) | ((uint64_t)serialized[pos + 2] << 40) | ((uint64_t)serialized[pos + 3] << 32)
                       | ((uint64_t)serialized[pos + 4] << 24) | ((uint64_t)serialized[pos + 5] << 16) | ((uint64_t)serialized[pos + 6] << 8) | ((uint64_t)serialized[pos + 7]);
    pos += 8;

    if (serialized.size() < pos + scriptlen) throw std::runtime_error("Invalid TxOutItem serialization.");
    script_.assign(serialized.begin() + pos, serialized.begin() + pos + scriptlen);
}


template<typename DBModelType>
class TxOutTree : public MMRTree<DBModelType>
{
public:
    explicit TxOutTree(const std::string& dbname) : MMRTree<DBModelType>(dbname) { }

    using MMRTree<DBModelType>::appendItem;
    void appendItem(const bytes_t& txhash, uint32_t txindex, const TxOutItem& txout);

    using MMRTree<DBModelType>::json;
    std::string json(const MerkleNodePtr<DBModelType>& root) const;
};

template<typename DBModelType>
void TxOutTree<DBModelType>::appendItem(const bytes_t& txhash, uint32_t txindex, const TxOutItem& txout)
{
    // TODO: more compact encoding
    bytes_t outpoint(txhash);
    outpoint.push_back(txindex >> 24);
    outpoint.push_back((txindex >> 16) & 0xff);
    outpoint.push_back((txindex >> 8) & 0xff);
    outpoint.push_back(txindex & 0xff);

    uint64_t size = this->size();
    bytes_t sizebytes;
    sizebytes.push_back(size >> 56);
    sizebytes.push_back((size >> 48) & 0xff);
    sizebytes.push_back((size >> 40) & 0xff);
    sizebytes.push_back((size >> 32) & 0xff);
    sizebytes.push_back((size >> 24) & 0xff);
    sizebytes.push_back((size >> 16) & 0xff);
    sizebytes.push_back((size >> 8) & 0xff);
    sizebytes.push_back(size & 0xff); 

    MMRTree<DBModelType>::appendItem(txout.getSerialized());
    this->db_.batchInsert(outpoint, sizebytes);
}

template<typename DBModelType>
std::string TxOutTree<DBModelType>::json(const MerkleNodePtr<DBModelType>& root) const
{
    if (!root) return "null";

    std::stringstream ss;
    ss << "{";
    ss << "\"size\":" << root->size() << ","
       << "\"hash\":\"" << uchar_vector(root->hash()).getHex() << "\",";
    if (root->isLeaf())
    {
        try
        {
            TxOutItem txout(root->data());
            ss << "\"version\":" << txout.version() << ","
               << "\"height\":" << txout.height() << ","
               << "\"coinbase\":" << (txout.isCoinBase() ? "true" : "false") << ","
               << "\"spent\":" << (txout.isSpent() ? "true" : "false" ) << ","
               << "\"script\":\"" << uchar_vector(txout.script()).getHex() << "\"";
        }
        catch (const std::exception& e)
        {
            ss << "\"error\":\"" << e.what() << "\"";
        }
    }
    else
    {
        ss << "\"left\":" << json(root->getLeftChild(this->db_)) << ","
           << "\"right\":" << json(root->getRightChild(this->db_));
    }
    ss << "}";

    return ss.str();
}

}
