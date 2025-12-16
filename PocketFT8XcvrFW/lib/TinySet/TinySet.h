/**
 * @brief Tiny Set Container
 *
 * This functionality is remotely similar to unordered_set but much
 * reduced to preserve RAM on the Teensy embedded MCU
 *
 * Notes
 *  + Only a few methods, insert() and erase() element, getSize() and isMember() are implemented.
 *  + TinySet implements iterators, the assignment operator, and a copy constructor.
 *  + A TinySet automatically resizes (expands) as elements are added, but
 *    never shrinks when elements are erased.
 *
 */
#pragma once

#include <type_traits>  // Required for std::is_class

template <typename T>
class TinySet {
   public:
    // Enable read/write iterators on a TinySet object
    class iterator {
       public:
        iterator(T* ptr) : ptr_(ptr) {}

        // Dereference operators
        T& operator*() { return *ptr_; }
        const T& operator*() const { return *ptr_; }
        T* operator->() { return ptr_; }
        const T* operator->() const { return ptr_; }

        // Increment operators
        iterator& operator++() {
            ++ptr_;
            return *this;
        }  // prefix
        iterator operator++(int) {
            iterator tmp(*this);
            ++ptr_;
            return tmp;
        }  // postfix

        // Comparison operators
        bool operator==(const iterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const iterator& other) const { return ptr_ != other.ptr_; }

       private:
        T* ptr_;
    };  // class iterator

    // Enable const iterators on a TinySet object
    class const_iterator {
       public:
        const_iterator(const T* ptr) : ptr_(ptr) {}

        // Dereference operators
        const T& operator*() const { return *ptr_; }
        const T* operator->() const { return ptr_; }

        // Increment operators
        const_iterator& operator++() {
            ++ptr_;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp(*this);
            ++ptr_;
            return tmp;
        }

        // Comparison operators
        bool operator==(const const_iterator& other) const { return ptr_ == other.ptr_; }
        bool operator!=(const const_iterator& other) const { return ptr_ != other.ptr_; }

       private:
        const T* ptr_;
    };  // class const_iterator

    // TinySet public methods
    TinySet();                                                                    // Constructor using default capacity
    TinySet(unsigned capacity);                                                   // Constructor for specified capacity
    TinySet(const TinySet& other) : capacity(other.capacity), size(other.size) {  // Copy constructor
        data = new T[capacity];
        for (unsigned i = 0; i < size; ++i) {
            data[i] = other.data[i];
        }
    }  // TinySet() Copy constructor
    ~TinySet();             // Destructor cleans up dynamic allocations
    bool insert(T member);  // Insert member into set
    bool erase(T member);   // Remove member from set

    // Iterator methods
    iterator begin() { return iterator(data); }
    iterator end() { return iterator(data + size); }
    const_iterator begin() const { return const_iterator(data); }
    const_iterator end() const { return const_iterator(data + size); }
    const_iterator cbegin() const { return const_iterator(data); }
    const_iterator cend() const { return const_iterator(data + size); }

    // Utility methods
    unsigned getSize() const { return size; }  // #elements currenty in set
    bool empty() const { return size == 0; }   // Is set empty?
    bool isMember(T key);                      // Determine if key is a member of set

    // Assignment operator (allows assignment of one set object to another)
    TinySet& operator=(const TinySet& other) {
        if (this != &other) {  // Self-assignment check
            delete[] data;     // Free current memory

            capacity = other.capacity;
            size = other.size;
            data = new T[capacity];

            for (unsigned i = 0; i < size; ++i) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }  // Operator =

   private:
    unsigned capacity;  // Current capacity of data[]
    unsigned size;      // #elements actually inserted into data[]
    bool resizeData();  // Resizes data[]
    T* data;            // data[capacity]
};  // TinySet

// Template TinySet method implementations

/**
 * @brief Template implementation for constructor specifying initial capacity
 * @tparam T
 * @param capacity Specifies initial capacity (#elements)
 */
template <typename T>
TinySet<T>::TinySet(unsigned capacity) : capacity(capacity), size(0) {
    data = new T[capacity];
}  // TinySet()

/**
 * @brief Default constructor
 * @tparam T
 */
template <typename T>
TinySet<T>::TinySet() : TinySet(10) {  // Delegating constructor
}  // TinySet()

/**
 * @brief Destructor frees dynamically allocated memory
 * @tparam T
 */
template <typename T>
TinySet<T>::~TinySet() {
    delete[] data;   // Deallocate the data array
    data = nullptr;  // Prevent double-delete
    capacity = 0;    // Prevent use of stale pointers
    size = 0;        // Prevent use of stale pointers
}  //~TinySet()

/**
 * @brief Insert element in set
 * @tparam T
 * @param element Element to insert
 * @return true==success, false==failure (duplicate or out of memory)
 *
 * @note Attempts to insert a duplicate element (already in the set) return false
 * @note The data[] is resized if element will not fit
 */
template <typename T>
bool TinySet<T>::insert(T key) {
    // Ignore duplicate insertions
    if (isMember(key)) return false;  // ✅ Return false for duplicate (no insertion occurred)

    // Expand data[] if new element won't fit
    if (size >= capacity) {
        if (!resizeData()) return false;  // ✅ Return false if resize failed
    }

    // Add element to set
    data[size++] = key;
    return true;  // ✅ Return true for successful insertion
}  // insert()

/**
 * @brief Determine if key is a member of set
 * @tparam T
 * @param key Search for key amongst set members
 * @return true if key is a member, else false
 */
template <typename T>
bool TinySet<T>::isMember(T key) {
    for (auto i = begin(); i != end(); ++i) {
        if (*i == key) return true;
    }
    return false;
}  // isMember()

/**
 * @brief Resize this TinySet's data[]
 * @tparam T
 * @return true==success, false==failure (out of memory)
 */
template <typename T>
bool TinySet<T>::resizeData() {
    // Allocate memory for new data[]
    unsigned newCapacity = (size * 3) / 2;  // 50% more capacity

    T* newData = new T[newCapacity];  // Get memory for new data[newCapacity]

    // Check if allocation failed (embedded systems often return nullptr instead of throwing)
    if (newData == nullptr) {
        return false;  // ✅ Return false for out of memory
    }

    // Copy old data[] into newData[]
    for (unsigned i = 0; i < size; i++) {
        newData[i] = data[i];
    }

    // Clean up loose ends
    capacity = newCapacity;  // Expanded capacity
    delete[] data;           // Free memory for old data[]
    data = newData;          // Now using new data[]
    return true;             // ✅ Return true for success
}  // resizeData()

/**
 * @brief Remove the specified element from set
 * @tparam T
 * @param key Element to remove
 * @return true==success, false==failure (element not found)
 *
 * @note Erasing an element removes it from the set but does not delete the element object
 */
template <typename T>
bool TinySet<T>::erase(T key) {
    unsigned i;  // Locates index of key in data[]
    unsigned j;  // Scans data[] following key

    // Find index of key in data[]
    for (i = 0; i < size; i++) {
        if (data[i] == key) break;
    }

    // Not found?
    if (i >= size) return false;  // ✅ Return false for element not found

    // Remove key from data[]
    for (j = i + 1; j < size; j++) {
        data[i++] = data[j];
    }

    // Finished
    size--;       // Update count of elements
    return true;  // ✅ Return true for successful removal
}  // erase()