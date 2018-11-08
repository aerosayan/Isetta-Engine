/*
 * Copyright (c) 2018 Isetta
 */
#pragma once
#include <initializer_list>
#include <limits>
#include <utility>
#include "Core/Debug/Assert.h"
#include "Core/Math/Util.h"
#include "Core/Memory/MemoryManager.h"
#include "ISETTA_API.h"

namespace Isetta {
template <typename T>
class Array {
 public:
  class iterator {
   public:
    typedef T value_type;
    typedef T &reference;
    typedef T *pointer;
    typedef std::forward_iterator_tag iterator_category;
    typedef int difference_type;
    explicit iterator(pointer ptr) : ptr_(ptr) {}
    iterator operator+(int lhs) {
      iterator i = *this;
      i.ptr_ += lhs;
      return i;
    }
    iterator operator-(int lhs) {
      iterator i = *this;
      i.ptr_ -= lhs;
      return i;
    }
    difference_type operator-(iterator lhs) { return ptr_ - lhs.ptr_; }
    iterator operator++() {
      iterator i = *this;
      ptr_++;
      return i;
    }
    iterator operator++(int) {
      ptr_++;
      return *this;
    }
    iterator operator--() {
      iterator i = *this;
      ptr_--;
      return i;
    }
    iterator operator--(int) {
      ptr_--;
      return *this;
    }
    reference operator*() { return *ptr_; }
    pointer operator->() { return ptr_; }
    bool operator==(const iterator &rhs) { return ptr_ == rhs.ptr_; }
    bool operator!=(const iterator &rhs) { return ptr_ != rhs.ptr_; }

   private:
    pointer ptr_;
  };
  class const_iterator {
   public:
    typedef T value_type;
    typedef T &reference;
    typedef T *pointer;
    typedef int difference_type;
    typedef std::forward_iterator_tag iterator_category;
    explicit const_iterator(pointer ptr) : ptr_(ptr) {}
    const_iterator operator+(int lhs) {
      const_iterator i = *this;
      i.ptr_ -= lhs;
      return i;
    }
    const_iterator operator-(int lhs) {
      const_iterator i = *this;
      i.ptr_ -= lhs;
      return i;
    }
    difference_type operator-(const_iterator lhs) { return ptr_ - lhs.ptr_; }
    const_iterator operator++() {
      const_iterator i = *this;
      ptr_++;
      return i;
    }
    const_iterator operator++(int) {
      ptr_++;
      return *this;
    }
    const_iterator operator--() {
      const_iterator i = *this;
      ptr_--;
      return i;
    }
    const_iterator operator--(int) {
      ptr_--;
      return *this;
    }
    const reference operator*() { return *ptr_; }
    const pointer operator->() { return ptr_; }
    bool operator==(const const_iterator &rhs) { return ptr_ == rhs.ptr_; }
    bool operator!=(const const_iterator &rhs) { return ptr_ != rhs.ptr_; }

   private:
    pointer ptr_;
  };
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  typedef U64 size_type;
  typedef ptrdiff_t difference_type;
  typedef size_t size_type;
  typedef T value_type;
  typedef T *pointer;
  typedef T &reference;
  typedef const T &const_reference;

 private:
  T *data;
  size_type size_, capacity;

  void ReservePow2(int capacity);

 public:
  Array() : size_{0}, capacity{0}, data{nullptr} {}
  explicit Array(size_type capacity) : size_{0}, capacity{capacity} {
    data = MemoryManager::NewArrOnFreeList<T>(capacity);
  }
  Array(size_type capacity, const value_type &val)
      : size_{capacity}, capacity{capacity} {
    data = MemoryManager::NewArrOnFreeList<T>(capacity);
    for (int i = 0; i < size_; i++) data[i] = val;
  }
  Array(std::initializer_list<T> list) : size_{0}, capacity{list.size()} {
    data = MemoryManager::NewArrOnFreeList<T>(capacity);
    Insert(begin(), list.begin(), list.end());
  }
  Array(iterator beginIter, iterator endIter)
      : size_{0}, capacity{static_cast<size_type>(endIter - beginIter)} {
    data = MemoryManager::NewArrOnFreeList<T>(capacity);
    Insert(begin(), beginIter, endIter);
  }
  ~Array();

  explicit Array(const std::vector<T> &inVector);
  // explicit operator std::vector<T>();

  Array(const Array &inVector)
      : size_{inVector.size_}, capacity{inVector.capacity} {
    data = MemoryManager::NewArrOnFreeList<T>(capacity);
    for (int i = 0; i < size_; i++) data[i] = inVector[i];
  }
  Array(Array &&inVector) noexcept
      : data{inVector.data},
        size_{inVector.size_},
        capacity{inVector.capacity} {
    inVector.size_ = inVector.capacity = 0;
    inVector.data = nullptr;
  }
  inline Array &operator=(const Array &inVector) {
    size_ = inVector.size_;
    capacity = inVector.capacity;
    data = MemoryManager::NewArrOnFreeList<T>(capacity);
    for (int i = 0; i < size_; i++) data[i] = inVector[i];
    return *this;
  }
  inline Array &operator=(Array &&inVector) noexcept {
    capacity = inVector.capacity;
    size_ = inVector.size_;
    data = inVector.data;
    inVector.size_ = inVector.capacity = 0;
    inVector.data = nullptr;
    return *this;
  }

  iterator begin() { return iterator(data); }
  const_iterator begin() const { return const_iterator(data); }
  const_iterator cbegin() const noexcept { return const_iterator(data); }
  iterator end() { return iterator(data + size_); }
  const_iterator end() const { return const_iterator(data + size_); }
  const_iterator cend() const noexcept { return const_iterator(data + size_); }
  reverse_iterator rbegin() { return reverse_iterator(data); }
  const_reverse_iterator rbegin() const { return reverse_iterator(data); }
  const_reverse_iterator crbegin() const noexcept {
    return reverse_iterator(data);
  }
  reverse_iterator rend() { return reverse_iterator(data + size_); }
  const_reverse_iterator rend() const { return reverse_iterator(data + size_); }
  const_reverse_iterator crend() const noexcept {
    return reverse_iterator(data + size_);
  }

  inline bool operator==(const Array &rhs) const;

  inline size_type Size() const { return size_; }
  inline size_type MaxSize() const { return std::numeric_limits<U64>::max(); }
  inline size_type Capacity() const { return capacity; }
  bool IsEmpty() const { return size_ == 0; }
  void Resize(int size, value_type val = value_type());
  void Reserve(int capacity);
  void Shrink();

  inline reference operator[](size_type i);
  inline const_reference operator[](size_type i) const;
  inline reference At(size_type i);
  inline const_reference At(size_type i) const;
  inline reference Front();
  inline const_reference Front() const;
  inline reference Back();
  inline const_reference Back() const;
  inline value_type *Data() { return data; }
  inline const value_type *Data() const { return data; }

  inline void Assign(size_type cnt, const value_type &val);
  inline void Assign(iterator begin, iterator end);
  inline void Assign(const T *begin, const T *end);
  inline void Assign(std::initializer_list<T> list);
  inline void PushBack(const value_type &val);
  inline void PopBack();
  inline iterator Insert(iterator position, const value_type &val);
  inline iterator Insert(iterator position, size_type cnt,
                         const value_type &val);
  inline iterator Insert(iterator position, iterator begin, iterator end);
  inline iterator Insert(iterator position, const T *begin, const T *end);
  inline iterator Insert(iterator position, std::initializer_list<T> list);
  inline iterator Erase(iterator position);
  inline iterator Erase(iterator begin, iterator end);
  inline void Swap(Array &x);
  inline void Clear();
  template <typename... Args>
  inline iterator Emplace(iterator position, Args &&... args);
  template <typename... Args>
  inline iterator Emplace(const_iterator position, Args &&... args);
  template <typename... Args>
  inline void EmplaceBack(Args &&... args);

  inline bool empty() const { return size_ == 0; }
  inline size_type size() const { return size_; }
  inline reference front() { return data[0]; }
  inline const_reference front() const { return data[0]; }
  inline void push_back(const value_type &val) { PushBack(val); }
  inline void pop_back() { PopBack(); }

  //  template<typename A, typename B>
  // friend std::priority_queue<
};

template <typename T>
inline Array<T>::~Array() {
  for (int i = 0; i < size_; i++) data[i].~T();
  if (capacity > 0) MemoryManager::FreeOnFreeList(data);
  data = nullptr;
  size_ = capacity = 0;
}

template <typename T>
inline Array<T>::Array(const std::vector<T> &inVector) {
  size_ = inVector.size();
  capacity = inVector.capacity();
  data = MemoryManager::NewArrOnFreeList<T>(capacity);
  for (int i = 0; i < size_; i++) data[i] = std::move(inVector[i]);
}

template <typename T>
inline bool Array<T>::operator==(const Array &rhs) const {
  if (size_ != rhs.size_) return false;
  for (const_iterator lhsIt = begin(), rhsIt = rhs.begin();
       lhsIt != end(), rhsIt != rhs.end(); lhsIt++, rhsIt++) {
    if (*lhsIt != *rhsIt) return false;
  }
  return true;
}

template <typename T>
inline void Array<T>::Resize(int inSize, value_type val) {
  if (inSize > capacity) ReservePow2(inSize);
  for (int i = inSize; i < size_; i++) data[i].~T();
  for (int i = size_; i < inSize; i++) data[i] = val;
  size_ = inSize;
}

template <typename T>
inline void Array<T>::ReservePow2(int inCapacity) {
  inCapacity = Math::Util::NextPowerOfTwo(inCapacity);
  if (inCapacity < capacity) return;
  T *tmpData = MemoryManager::NewArrOnFreeList<T>(inCapacity);
  for (int i = 0; i < size_; i++) tmpData[i] = data[i];
  if (capacity > 0) MemoryManager::FreeOnFreeList(data);
  capacity = inCapacity;
  data = tmpData;
}

template <typename T>
inline void Array<T>::Reserve(int inCapacity) {
  if (inCapacity < capacity) return;
  T *tmpData = MemoryManager::NewArrOnFreeList<T>(inCapacity);
  for (int i = 0; i < size_; i++) tmpData[i] = data[i];
  if (capacity > 0) MemoryManager::FreeOnFreeList(data);
  capacity = inCapacity;
  data = tmpData;
}
template <typename T>
inline void Array<T>::Shrink() {
  if (size_ == capacity) return;
  // TODO(Jacob) + TODO(Yidi) + TODO(Caleb) can I just shift the pointer?
  // realloc free list
  // MemoryManager::FreeOnFreeList(data + size * sizeof(T));
  T *tmpData = MemoryManager::NewArrOnFreeList<T>(size_);
  for (int i = 0; i < size_; i++) tmpData[i] = data[i];
  if (capacity > 0) MemoryManager::FreeOnFreeList(data);
  capacity = size_;
  data = tmpData;
}
template <typename T>
inline typename Array<T>::reference Array<T>::operator[](size_type i) {
  ASSERT(i < size_);
  return data[i];
}
template <typename T>
inline
    typename Array<T>::const_reference Array<T>::operator[](size_type i) const {
  ASSERT(i < size_);
  return data[i];
}
template <typename T>
inline typename Array<T>::reference Array<T>::At(size_type i) {
  if (i >= size_)
    throw new std::out_of_range("Vector::At => index must be less than size");
  return data[i];
}
template <typename T>
inline typename Array<T>::const_reference Array<T>::At(size_type i) const {
  if (i >= size_)
    throw new std::out_of_range("Vector::At => index must be less than size");
  return data[i];
}
template <typename T>
inline typename Array<T>::reference Array<T>::Front() {
  if (size_ == 0)
    throw new std::out_of_range("Vector::Front => size must be greater than 0");
  return data[0];
}
template <typename T>
inline typename Array<T>::const_reference Array<T>::Front() const {
  if (size_ == 0)
    throw new std::out_of_range("Vector::Front => size must be greater than 0");
  return data[0];
}
template <typename T>
inline typename Array<T>::reference Array<T>::Back() {
  if (size_ == 0)
    throw new std::out_of_range("Vector::Back => size must be greater than 0");
  return data[size_ - 1];
}
template <typename T>
inline typename Array<T>::const_reference Array<T>::Back() const {
  if (size_ == 0)
    throw new std::out_of_range("Vector::Back => size must be greater than 0");
  return data[size_ - 1];
}
template <typename T>
inline void Array<T>::Assign(size_type cnt, const value_type &val) {
  if (cnt > capacity) ReservePow2(cnt);
  for (int i = 0; i < size_; i++) data[i].~T();
  for (int i = 0; i < cnt; i++) data[i] = val;
  size_ = cnt;
}
template <typename T>
inline void Array<T>::Assign(iterator beginIter, iterator endIter) {
  if (endIter - beginIter > capacity) ReservePow2(endIter - beginIter);
  size_ = endIter - beginIter;
  iterator itThis = begin();
  for (iterator it = beginIter; it != endIter; it++, itThis++) {
    (*itThis).~T();
    *itThis = *it;
  }
  for (; itThis != end(); itThis++) itThis->~T();
}
template <typename T>
inline void Array<T>::Assign(const T *beginPtr, const T *endPtr) {
  if (endPtr - beginPtr > capacity) ReservePow2(endPtr - beginPtr);
  size_ = endPtr - beginPtr;
  iterator itThis = begin();
  for (const T *it = beginPtr; it != endPtr; it++, itThis++) {
    (*itThis).~T();
    *itThis = *it;
  }
  for (; itThis != end(); itThis++) (*itThis).~T();
}
template <typename T>
inline void Array<T>::Assign(std::initializer_list<T> list) {
  if (list.size() > capacity) ReservePow2(list.size());
  size_ = list.size();
  iterator itThis = begin();
  for (auto it = list.begin(); it != list.end(); it++, itThis++) {
    (*itThis).~T();
    *itThis = *it;
  }
  for (; itThis != end(); itThis++) (*itThis).~T();
}
template <typename T>
inline void Array<T>::PushBack(const value_type &val) {
  if (size_ + 1 > capacity) ReservePow2(size_ + 1);
  data[size_++] = val;
}
template <typename T>
inline void Array<T>::PopBack() {
  if (size_ == 0)
    throw new std::out_of_range(
        "Vector::PopBack => size must be greater than 0");
  data[size_--].~T();
}
template <typename T>
inline typename Array<T>::iterator Array<T>::Insert(iterator position,
                                                    const value_type &val) {
  iterator ret{nullptr};
  int newSize = size_ + 1;
  if (newSize > capacity) {
    int inCapacity = Math::Util::NextPowerOfTwo(newSize);
    T *tmpData = MemoryManager::NewArrOnFreeList<T>(inCapacity);
    iterator it = iterator(tmpData), itThis = begin();
    iterator end = iterator(tmpData + newSize);
    for (; it != end, itThis != position; it++, itThis++) *it = *itThis;
    *it = val;
    ret = it;
    it++;
    for (; it != end; it++, itThis++) *it = *itThis;
    if (capacity > 0) MemoryManager::FreeOnFreeList(data);
    capacity = inCapacity;
    data = tmpData;
  } else {
    for (iterator it = end(); it != position; it--) *it = *(it - 1);
    *position = val;
    ret = position;
  }
  size_ = newSize;
  return ret;
}
template <typename T>
inline typename Array<T>::iterator Array<T>::Insert(iterator position,
                                                    size_type cnt,
                                                    const value_type &val) {
  iterator ret{nullptr};
  int newSize = size_ + cnt;
  if (newSize > capacity) {
    int inCapacity = Math::Util::NextPowerOfTwo(newSize);
    T *tmpData = MemoryManager::NewArrOnFreeList<T>(inCapacity);
    iterator it = iterator(tmpData), itThis = begin();
    iterator end = iterator(tmpData + newSize);
    for (; it != end, itThis != position; it++, itThis++) *it = *itThis;
    for (int i = 0; i < cnt; i++, it++) *it = val;
    ret = it;
    for (; it != end; it++, itThis++) *it = *itThis;
    if (capacity > 0) MemoryManager::FreeOnFreeList(data);
    capacity = inCapacity;
    data = tmpData;
  } else {
    for (iterator it = end(); it != position; it--) *it = *(it - 1);
    for (int i = 0; i < cnt; i++) *(position + i) = val;
    ret = position + cnt - 1;
  }
  size_ = newSize;
  return ret;
}
template <typename T>
inline typename Array<T>::iterator Array<T>::Insert(iterator position,
                                                    iterator beginIter,
                                                    iterator endIter) {
  iterator ret{nullptr};
  int newSize = size_ + (endIter - beginIter);
  if (newSize > capacity) {
    int inCapacity = Math::Util::NextPowerOfTwo(newSize);
    T *tmpData = MemoryManager::NewArrOnFreeList<T>(inCapacity);
    iterator it = iterator(tmpData), itThis = begin();
    iterator end = iterator(tmpData + newSize);
    for (; it != end, itThis != position; it++, itThis++) *it = *itThis;
    for (iterator nit = beginIter; nit != endIter; nit++, it++) *it = *nit;
    ret = it;
    for (; it != end; it++, itThis++) *it = *itThis;
    if (capacity > 0) MemoryManager::FreeOnFreeList(data);
    capacity = inCapacity;
    data = tmpData;
  } else {
    for (iterator it = end(); it != position; it--) *it = *(it - 1);
    for (iterator nit = beginIter; nit != endIter; nit++, position++)
      *position = *nit;
    ret = position;
  }
  size_ = newSize;
  return ret;
}
template <typename T>
inline typename Array<T>::iterator Array<T>::Insert(iterator position,
                                                    const T *beginPtr,
                                                    const T *endPtr) {
  iterator ret{nullptr};
  int newSize = size_ + (endPtr - beginPtr);
  if (newSize > capacity) {
    int inCapacity = Math::Util::NextPowerOfTwo(newSize);
    T *tmpData = MemoryManager::NewArrOnFreeList<T>(inCapacity);
    iterator it = iterator(tmpData), itThis = begin();
    iterator end = iterator(tmpData + newSize);
    for (; it != end, itThis != position; it++, itThis++) *it = *itThis;
    for (const T *nit = beginPtr; nit != endPtr; nit++, it++) *it = *nit;
    ret = it;
    for (; it != end; it++, itThis++) *it = *itThis;
    if (capacity > 0) MemoryManager::FreeOnFreeList(data);
    capacity = inCapacity;
    data = tmpData;
  } else {
    for (iterator it = end(); it != position; it--) *it = *(it - 1);
    for (const T *nit = beginPtr; nit != endPtr; nit++, position++)
      *position = *nit;
    ret = position;
  }
  size_ = newSize;
  return ret;
}
template <typename T>
inline typename Array<T>::iterator Array<T>::Insert(
    iterator position, std::initializer_list<T> list) {
  return Insert(position, list.begin(), list.end());
}
template <typename T>
inline typename Array<T>::iterator Array<T>::Erase(iterator position) {
  iterator ret = position + 1;
  (*position).~T();
  for (iterator it = position; it != end() - 1; it++) *it = *(it + 1);
  size_--;
  return ret;
}
template <typename T>
inline typename Array<T>::iterator Array<T>::Erase(iterator beginIter,
                                                   iterator endIter) {
  for (iterator it = beginIter; it != endIter; it++) (*it).~T();
  iterator itEnd = endIter;
  for (iterator itBeg = beginIter; itEnd != end(); itBeg++, itEnd++)
    *itBeg = *itEnd;
  size_ -= (endIter - beginIter);
  return itEnd;
}
template <typename T>
inline void Array<T>::Swap(Array &x) {
  std::swap(data, x.data);
  std::swap(size_, x.size_);
  std::swap(capacity, x.capacity);
}
template <typename T>
inline void Array<T>::Clear() {
  for (int i = 0; i < size_; i++) data[i].~T();
  size_ = 0;
}
template <typename T>
template <typename... Args>
inline typename Array<T>::iterator Array<T>::Emplace(iterator position,
                                                     Args &&... args) {
  iterator ret{nullptr};
  int newSize = size_ + 1;
  if (newSize > capacity) {
    int inCapacity = Math::Util::NextPowerOfTwo(newSize);
    T *tmpData = MemoryManager::NewArrOnFreeList<T>(inCapacity);
    iterator it = iterator(tmpData), itThis = begin();
    iterator end = iterator(tmpData + newSize);
    for (; it != end, itThis != position; it++, itThis++) *it = *itThis;
    *it = T(std::forward<Args>(args)...);
    ret = it;
    it++;
    for (; it != end; it++, itThis++) *it = *itThis;
    if (capacity > 0) MemoryManager::FreeOnFreeList(data);
    capacity = inCapacity;
    data = tmpData;
  } else {
    for (iterator it = end(); it != position; it--) *it = *(it - 1);
    *position = T(std::forward<Args>(args)...);
    ret = position;
  }
  size_ = newSize;
  return ret;
}
template <typename T>
template <typename... Args>
inline typename Array<T>::iterator Array<T>::Emplace(const_iterator position,
                                                     Args &&... args) {
  iterator ret{nullptr};
  int newSize = size_ + 1;
  if (newSize > capacity) {
    int inCapacity = Math::Util::NextPowerOfTwo(newSize);
    T *tmpData = MemoryManager::NewArrOnFreeList<T>(inCapacity);
    iterator it = iterator(tmpData), itThis = begin();
    iterator end = iterator(tmpData + newSize);
    for (; it != end, itThis != position; it++, itThis++) *it = *itThis;
    *it = T(std::forward<Args>(args)...);
    ret = it;
    it++;
    for (; it != end; it++, itThis++) *it = *itThis;
    if (capacity > 0) MemoryManager::FreeOnFreeList(data);
    capacity = inCapacity;
    data = tmpData;
  } else {
    for (iterator it = end(); it != position; it--) *it = *(it - 1);
    *position = T(args);
    ret = position;
  }
  size_ = newSize;
  return ret;
}
template <typename T>
template <typename... Args>
inline void Array<T>::EmplaceBack(Args &&... args) {
  if (size_ + 1 > capacity) ReservePow2(size_ + 1);
  data[size_++] = T(std::forward<Args>(args)...);
}
}  // namespace Isetta