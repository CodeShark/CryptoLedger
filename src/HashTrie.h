#pragma once

#include <CoinCore/typedefs.h>
#include <CoinCore/hash.h>

#include <stdutils/uchar_vector.h>

#include <memory>
#include <stdexcept>

namespace HashTrie
{

template<typename DBModelType>
class MerkleNode : std::enable_shared_from_this<MerkleNode<DBModelType>>
{
public:
    MerkleNode() : size_(0) { }
    explicit MerkleNode(const bytes_t& serialized) { setSerialized(serialized); }

    const bytes_t& hash() const { return hash_; }
    const bytes_t& data() const { return data_; }
    const uint64_t& size() const { return size_; }

    const bytes_t& leftChildHash() const { return leftChildHash_; }
    const bytes_t& rightChildHash() const { return rightChildHash_; }

    std::shared_ptr<MerkleNode<DBModelType>> getLeftChild(const DBModelType& db) const;
    std::shared_ptr<MerkleNode<DBModelType>> getRightChild(const DBModelType& db) const;

    void setData(const bytes_t& data);
    void setLeftChildHash(const bytes_t& leftChildHash);
    void setRightChildHash(const bytes_t& rightChildHash);

    bool isLeaf() const { return (size_ == 1); }

    bytes_t getSerialized() const;
    void setSerialized(const bytes_t& serialized);

private:
    bytes_t hash_;
    bytes_t data_;
    uint64_t size_;

    bytes_t leftChildHash_;
    bytes_t rightChildHash_;

    void updateHash();
};

template<typename DBModelType>
void MerkleNode<DBModelType>::setData(const bytes_t& data)
{
    data_ = data;
    updateHash();
}

template<typename DBModelType>
void MerkleNode<DBModelType>::setLeftChildHash(const bytes_t& leftChildHash)
{
    leftChildHash_ = leftChildHash;
    updateHash();
}

template<typename DBModelType>
void MerkleNode<DBModelType>::setRightChildHash(const bytes_t& rightChildHash)
{
    rightChildHash_ = rightChildHash;
    updateHash();
}

template<typename DBModelType>
std::shared_ptr<MerkleNode<DBModelType>> MerkleNode<DBModelType>::getLeftChild(const DBModelType& db) const
{
    if (leftChildHash_.empty()) throw std::runtime_error("Node does not have a left child.");

    bytes_t serialized;
    db.get(leftChildHash_, serialized);
    return std::make_shared<MerkleNode<DBModelType>>(serialized);
}

template<typename DBModelType>
std::shared_ptr<MerkleNode<DBModelType>> MerkleNode<DBModelType>::getRightChild(const DBModelType& db) const
{
    if (rightChildHash_.empty()) throw std::runtime_error("Node does not have a right child.");

    bytes_t serialized;
    db.get(rightChildHash_, serialized);
    return std::make_shared<MerkleNode<DBModelType>>(serialized);
}

template<typename DBModelType>
bytes_t MerkleNode<DBModelType>::getSerialized() const
{
    uchar_vector rval;
    uint32_t len;

    // TODO: More compact encoding
    rval.push_back(size_ >> 56);
    rval.push_back((size_ >> 48) & 0xff);
    rval.push_back((size_ >> 40) & 0xff);
    rval.push_back((size_ >> 32) & 0xff);
    rval.push_back((size_ >> 24) & 0xff);
    rval.push_back((size_ >> 16) & 0xff);
    rval.push_back((size_ >> 8) & 0xff);
    rval.push_back(size_ & 0xff);

    len = leftChildHash_.size();
    rval.push_back(len >> 24);
    rval.push_back((len >> 16) & 0xff);
    rval.push_back((len >> 8) & 0xff);
    rval.push_back(len & 0xff);
    rval += leftChildHash_;

    len = data_.size();
    rval.push_back(len >> 24);
    rval.push_back((len >> 16) & 0xff);
    rval.push_back((len >> 8) & 0xff);
    rval.push_back(len & 0xff);
    rval += data_;

    len = rightChildHash_.size();
    rval.push_back(len >> 24);
    rval.push_back((len >> 16) & 0xff);
    rval.push_back((len >> 8) & 0xff);
    rval.push_back(len & 0xff);
    rval += rightChildHash_;

    return rval;
}

template<typename DBModelType>
void MerkleNode<DBModelType>::setSerialized(const bytes_t& serialized)
{
    uint32_t len;
    uint32_t pos = 0;

    if (serialized.size() < 8) throw std::runtime_error("Invalid merkle node serialization");
    size_ = ((uint64_t)serialized[pos] << 56) | ((uint64_t)serialized[pos + 1] << 48) | ((uint64_t)serialized[pos + 2] << 40) | ((uint64_t)serialized[pos + 3] << 32)
          | ((uint64_t)serialized[pos + 4] << 24) | ((uint64_t)serialized[pos + 5] << 16) | ((uint64_t)serialized[pos + 6] << 8) | ((uint64_t)serialized[pos + 7]);
    pos += 8;

    if (serialized.size() < pos + 4) throw std::runtime_error("Invalid merkle node serialization");
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

template<typename DBModelType>
void MerkleNode<DBModelType>::updateHash()
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

    const bytes_t& rootHash() const { return rootHash_; }

private:
    DBModelType dbModel_;
    bytes_t rootHash_;
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
