#include "primer/hyperloglog.h"
#include <cmath>

namespace bustub {

template <typename KeyType>
HyperLogLog<KeyType>::HyperLogLog(int16_t n_bits)
    : cardinality_(0), bits_(n_bits), buckets_(1 << n_bits, 0) {}

template <typename KeyType>
auto HyperLogLog<KeyType>::ComputeBinary(const hash_t &hash) const -> std::bitset<BITSET_CAPACITY> {
  return std::bitset<BITSET_CAPACITY>(hash);
}

template <typename KeyType>
auto HyperLogLog<KeyType>::PositionOfLeftmostOne(const std::bitset<BITSET_CAPACITY> &bset) const -> uint64_t {
  // 边界检查
  if (bits_ >= 64) {
    return 1;  // 没有剩余位
  }
  
  int64_t max_bit = 64 - static_cast<int64_t>(bits_) - 1;
  
  for (int64_t i = max_bit; i >= 0; i--) {
    if (bset[i] == 1) {
      return max_bit - i + 1;
    }
  }
  return max_bit + 2;  // 全是 0
}

template <typename KeyType>
auto HyperLogLog<KeyType>::AddElem(KeyType val) -> void {
  hash_t hash = CalculateHash(val);
  
  // 边界检查：防止右移 64 位
  size_t bucket_idx = 0;
  hash_t remaining = hash;
  
  if (bits_ > 0 && bits_ < 64) {
    bucket_idx = hash >> (64 - bits_);
    remaining = hash & ((1ULL << (64 - bits_)) - 1);
  } else if (bits_ == 64) {
    bucket_idx = hash;  // 所有位都是索引
    remaining = 0;
  }
  // bits_ == 0 时，bucket_idx = 0, remaining = hash
  
  std::bitset<BITSET_CAPACITY> binary = ComputeBinary(remaining);
  uint64_t leading_zeros = PositionOfLeftmostOne(binary);
  
  buckets_[bucket_idx] = std::max(buckets_[bucket_idx], static_cast<uint8_t>(leading_zeros));
}

template <typename KeyType>
auto HyperLogLog<KeyType>::ComputeCardinality() -> void {
  double sum = 0.0;
  int zero_count = 0;  // 统计值为 0 的桶数量
  
  for (const auto &bucket : buckets_) {
    if (bucket == 0) {
      zero_count++;
    }
    sum += std::pow(2.0, -static_cast<double>(bucket));
  }
  
  double m = static_cast<double>(buckets_.size());
  
  // 修复：如果所有桶都是 0，基数为 0
  if (zero_count == static_cast<int>(m)) {
    cardinality_ = 0;
    return;
  }
  
  double alpha = CONSTANT * m * m;
  cardinality_ = static_cast<size_t>(alpha / sum);
}

template class HyperLogLog<int64_t>;
template class HyperLogLog<std::string>;

}  // namespace bustub
