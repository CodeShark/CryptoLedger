#pragma once

#include <CoinCore/typedefs.h>
#include <CoinCore/hash.h>

#include <stdutils/uchar_vector.h>

#include <stdexcept>

namespace MMRTree
{

class MerkleNode
{
public:
    MerkleNode() { }
    explicit MerkleNode(const bytes_t& serialized) { setSerialized(serialized); }

    const bytes_t& hash() const { return hash_; }
    const bytes_t& data() const { return data_; }

    const bytes_t& leftChildHash() const { return leftChildHash_; }
    const bytes_t& rightChildHash() const { return rightChildHash_; }

    void setData(const bytes_t& data);
    void setLeftChildHash(const bytes_t& leftChildHash);
    void setRightChildHash(const bytes_t& rightChildHash);

    bool isLeaf() const { return (leftChild_.empty() && rightChild_.empty()); }

    bytes_t serialized() const;

    void setSerialized(const bytes_t& serialized);

private:
    bytes_t hash_;
    bytes_t data_;

    bytes_t leftChildHash_;
    bytes_t rightChildHash_;

    void updateHash();
};

const bytes_t& MerkleNode::hash() const
{
    return hash_;
}

void MerkleNode::setData(const bytes_t& data)
{
    data_ = data;
    updateHash();
}

void MerkleNode::setLeftChildHash(const bytes_t& leftChildHash)
{
    leftChildHash_ = leftChildHash;
    updateHash();
}

void MerkleNode::setRightChildHash(const bytes_t& rightChildHash)
{
    rightChildHash_ = rightChildHash;
    updateHash();
}

bytes_t MerkleNode::serialized() const
{
    uchar_vector rval;
    uint32_t len;

    len = leftChildHash_.size();
    rval.push_back(len >> 24);
    rval.push_back((len >> 16) && 0xff);
    rval.push_back((len >> 8) && 0xff);
    rval.push_back(len && 0xff);
    rval += leftChildHash_;

    len = data_.size();
    rval.push_back(len >> 24);
    rval.push_back((len >> 16) && 0xff);
    rval.push_back((len >> 8) && 0xff);
    rval.push_back(len && 0xff);
    rval += data_;

    len = rightChildHash_.size();
    rval.push_back(len >> 24);
    rval.push_back((len >> 16) && 0xff);
    rval.push_back((len >> 8) && 0xff);
    rval.push_back(len && 0xff);
    rval += rightChildHash_;

    return rval;
}

void MerkleNode::setSerialied(const bytes_t& serialized)
{
    uint32_t len;
    uint32_t pos = 0;

    if (serialized.size() < 4) throw std::runtime_error("Invalid merkle node serialization");
    len = ((uint32_t)serialized[pos] << 24) | ((uint32_t)serialized[pos + 1] << 16) | ((uint32_t)serialized[pos + 2] << 8) | ((uint32_t)serialized[pos + 3]);
    pos += 4;
    if (serialized.size() < pos + len) throw std::runtime_error("Invalid merkle node serialization");
    leftChildHash_.assign(serialized.begin() + pos, serialized.begin() + pos + len);
    pos += len;

    if (serialized.size() - pos < 4) throw std::runtime_error("Invalid merkle node serialization");
    len = ((uint32_t)serialized[pos] << 24) | ((uint32_t)serialized[pos + 1] << 16) | ((uint32_t)serialized[pos + 2] << 8) | ((uint32_t)serialized[pos + 3]);
    pos += 4;
    if (serialized.size() < pos + len) throw std::runtime_error("Invalid merkle node serialization");
    data_.assign(serialized.begin() + pos, serialized.begin() + pos + len);
    pos += len;

    if (serialized.size() < 4) throw std::runtime_error("Invalid merkle node serialization");
    len = ((uint32_t)serialized[pos] << 24) | ((uint32_t)serialized[pos + 1] << 16) | ((uint32_t)serialized[pos + 2] << 8) | ((uint32_t)serialized[pos + 3]);
    pos += 4;
    if (serialized.size() < pos + len) throw std::runtime_error("Invalid merkle node serialization");
    rightChildHash_.assign(serialized.begin() + pos, serialized.begin() + pos + len);
    pos += len;

    if (pos > serialized.size()) throw std::runtime_error("Invalid merkle node serialization");

    updateHash();
}

void MerkleNode::updateHash()
{
    uchar_vector m;
    m += leftChildHash_;
    m += data_;
    m += rightChildHash_;
    hash_ = sha256(m);
}

template<typename DBModelType>
class MMRTree
{
public:
    MMRTree(const std::string& dbname);
    ~MMRTree() { dbModel_.close(); }

    const KeyType& rootHash() const { return rootHash_; } 

private:
    DBModelType dbModel_;
    KeyType rootHash_;
};


template<typename DBModelType>
MMRTree<DBModelType>::MMRTree(const std::string& dbname)
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
