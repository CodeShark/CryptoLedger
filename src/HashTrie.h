#pragma once

#include <CoinCore/typedefs.h>
#include <CoinCore/hash.h>

#include <stdutils/uchar_vector.h>

#include <memory>
#include <sstream>
#include <stdexcept>

namespace HashTrie
{

template<typename DBModelType>
class MerkleNode;

template<typename DBModelType>
using MerkleNodePtr = std::shared_ptr<MerkleNode<DBModelType>>;

template<typename DBModelType>
class MerkleNode : std::enable_shared_from_this<MerkleNode<DBModelType>>
{
public:
    MerkleNode() : size_(1) { }
    explicit MerkleNode(const bytes_t& serialized) { setSerialized(serialized); }
    MerkleNode(const MerkleNode<DBModelType>& leftChild, const MerkleNode<DBModelType>& rightChild);

    const bytes_t& hash() const { return hash_; }
    const bytes_t& data() const { return data_; }
    const uint64_t& size() const { return size_; }

    const bytes_t& leftChildHash() const { return leftChildHash_; }
    const bytes_t& rightChildHash() const { return rightChildHash_; }

    MerkleNodePtr<DBModelType> getLeftChild(const DBModelType& db) const;
    MerkleNodePtr<DBModelType> getRightChild(const DBModelType& db) const;

    void setData(const bytes_t& data);
    void setLeftChildHash(const bytes_t& leftChildHash);
    void setRightChildHash(const bytes_t& rightChildHash);
    void save(DBModelType& db);
    void erase(DBModelType& db);

    bool isLeaf() const { return (size_ == 1); }
    bool isPerfect() const { return (/*(size_ != 0) &&*/ ((size_ & (~size_ + 1)) == size_)); } // size_ is a power of 2, size cannot be zero

    bytes_t getSerialized() const;
    void setSerialized(const bytes_t& serialized);

    MerkleNodePtr<DBModelType> appendItem(const bytes_t& data, DBModelType& db);
    MerkleNodePtr<DBModelType> appendTree(const MerkleNode<DBModelType>& root, DBModelType& db);

private:
    bytes_t hash_;
    bytes_t data_;
    uint64_t size_;

    bytes_t leftChildHash_;
    bytes_t rightChildHash_;

    void updateHash();
};

template<typename DBModelType>
MerkleNode<DBModelType>::MerkleNode(const MerkleNode<DBModelType>& leftChild, const MerkleNode<DBModelType>& rightChild)
{
    size_ = leftChild.size() + rightChild.size();
    leftChildHash_ = leftChild.hash();
    rightChildHash_ = rightChild.hash();
    updateHash();
}

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
void MerkleNode<DBModelType>::save(DBModelType& db)
{
    bytes_t serialized = getSerialized();
    db.insert(hash_, serialized);
}

template<typename DBModelType>
void MerkleNode<DBModelType>::erase(DBModelType& db)
{
    db.remove(hash_);
}

template<typename DBModelType>
MerkleNodePtr<DBModelType> MerkleNode<DBModelType>::getLeftChild(const DBModelType& db) const
{
    if (leftChildHash_.empty()) throw std::runtime_error("Node does not have a left child.");

    bytes_t serialized;
    db.get(leftChildHash_, serialized);
    return std::make_shared<MerkleNode<DBModelType>>(serialized);
}

template<typename DBModelType>
MerkleNodePtr<DBModelType> MerkleNode<DBModelType>::getRightChild(const DBModelType& db) const
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
MerkleNodePtr<DBModelType> MerkleNode<DBModelType>::appendItem(const bytes_t& data, DBModelType& db)
{
    MerkleNode newRightChild;
    newRightChild.setData(data);
    newRightChild.save(db);

    if (size_ == 1)
    {
        MerkleNodePtr<DBModelType> newRoot = std::make_shared<MerkleNode<DBModelType>>(*this, newRightChild);
        newRoot->save(db);
        return newRoot;
    }
    else if (size_ & 0x1)
    {
        // size is odd, append to right child and merge into left child if possible
        MerkleNodePtr<DBModelType> rightChild = getRightChild(db);
        rightChild->erase(db);
        rightChild = rightChild->appendItem(data, db);
        rightChild->save(db);

        erase(db);
        return getLeftChild(db)->appendTree(rightChild, db);
    }     
    else
    {
        // size is even, create new root with this for left child and new item for right child
        MerkleNodePtr<DBModelType> newRoot = std::make_shared<MerkleNode<DBModelType>>(*this, newRightChild);
        newRoot->save(db);
        return newRoot;
    }
}

template<typename DBModelType>
MerkleNodePtr<DBModelType> MerkleNode<DBModelType>::appendTree(const MerkleNode<DBModelType>& root, DBModelType& db)
{
    if (size_ < root->size()) throw std::runtime_error("Cannot merge larger tree into smaller one.");

    if (!(size_ & root->size()))
    {
        // No trees of same size, just append tree as new right child
        MerkleNodePtr<DBModelType> newRoot = std::make_shared<MerkleNode<DBModelType>>(*this, root);
        newRoot->save(db);
        return newRoot;
    }
    else if (size_ == root->size())
    {
        if (!isPerfect()) throw std::runtime_error("Cannot merge into nonperfect tree.");

        MerkleNodePtr<DBModelType> newRoot = std::make_shared<MerkleNode<DBModelType>>(*this, root);
        newRoot->save(db);
        return newRoot;
    }
    else
    {
        // Recurse on right side
        MerkleNodePtr<DBModelType> newRoot = getRightChild(db)->appendTree(root, db);

        // Recurse on left side
        newRoot = getLeftChild(db)->appendTree(*newRoot, db);

        erase(db); // the original tree root is removed 
        return newRoot;
    }
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
    ~MMRTree() { db_.close(); }

    const MerkleNodePtr<DBModelType>& root() const { return root_; }
    const bytes_t& rootHash() const { return root_ ? root_->hash() : bytes_t(); }
    uint64_t size() const { return root_ ? root_->size() : 0; }

    void appendItem(const bytes_t& data);

    static std::string json(const MerkleNodePtr<DBModelType>& root);
    std::string json() const { return json(root_); }

private:
    DBModelType db_;
    MerkleNodePtr<DBModelType> root_;
};


template<typename DBModelType>
MMRTree<DBModelType>::MMRTree(const std::string& dbname)
{
    db_.open(dbname);
    try
    {
        bytes_t rootHash;
        db_.get(bytes_t(), rootHash);
        if (rootHash.empty()) return;

        bytes_t serialized;
        db_.get(rootHash, serialized);
        root_->setSerialized(serialized);
    }
    catch (...)
    {
        db_.insert(bytes_t(), bytes_t());
    }
}

template<typename DBModelType>
void MMRTree<DBModelType>::appendItem(const bytes_t& data)
{
    if (root_)
    {
        root_->appendItem(data, db_);
    }
    else
    {
        root_ = std::make_shared<MerkleNode<DBModelType>>();
        root_.setData(data);
        root_.save(db_);
        db_.insert(bytes_t(), root_.hash());
    }
}

template<typename DBModelType>
std::string MMRTree<DBModelType>::json(const MerkleNodePtr<DBModelType>& root)
{
    if (!root) return "null";

    std::stringstream ss;
    ss << "{";
    ss << "hash:" << uchar_vector(root->hash()).getHex() << ",";
    if (root->isLeaf())
    {
        ss << "data:" << uchar_vector(root->getData()).getHex();
    }
    else
    {
        ss << "left:" << json(root->getLeftChild()) << ","
           << "right:" << json(root->getRightChild());
    }
    ss << "}";

    return ss.str();
}

}
