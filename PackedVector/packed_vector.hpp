#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <utility>

template<typename T, size_t N>
class PackedVector {
private:
    static const size_t BITS = sizeof(T) * 8;
    static const size_t ELEMS = BITS / N;
    static const T MASK = (T(1) << N) - 1;

    T* data_;
    size_t size_;
    size_t capacity_;

    size_t blocks(size_t n) const { return n ? (n + ELEMS - 1) / ELEMS : 0; }

    T read(size_t i) const {
        size_t b = i / ELEMS, off = (i % ELEMS) * N;
        return (data_[b] >> off) & MASK;
    }

    void write(size_t i, T v) {
        size_t b = i / ELEMS, off = (i % ELEMS) * N;
        data_[b] &= ~(MASK << off);
        data_[b] |= ((v & MASK) << off);
    }

public:
    class reference {
        PackedVector* p;
        size_t i;
        friend class PackedVector;
    public:
        reference(PackedVector* v, size_t idx) : p(v), i(idx) {}
        reference& operator=(T v) { p->write(i, v); return *this; }
        reference& operator=(const reference& r) {
            if (this != &r) p->write(i, r.p->read(r.i));
            return *this;
        }
        operator T() const { return p->read(i); }
    };

    class iterator {
        PackedVector* p;
        size_t i;
        friend class PackedVector;
        friend class const_iterator;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = PackedVector::reference;

        iterator() : p(nullptr), i(0) {}
        iterator(PackedVector* v, size_t idx) : p(v), i(idx) {}

        reference operator*() const { return reference(p, i); }
        reference operator[](difference_type n) const { return reference(p, i + n); }

        iterator& operator++() { ++i; return *this; }
        iterator operator++(int) { auto t = *this; ++i; return t; }
        iterator& operator--() { --i; return *this; }
        iterator operator--(int) { auto t = *this; --i; return t; }

        iterator& operator+=(difference_type n) { i += n; return *this; }
        iterator& operator-=(difference_type n) { i -= n; return *this; }

        iterator operator+(difference_type n) const { return {p, i + n}; }
        iterator operator-(difference_type n) const { return {p, i - n}; }
        friend iterator operator+(difference_type n, const iterator& it) {
            return {it.p, it.i + n};
        }
        difference_type operator-(const iterator& o) const {
            return static_cast<difference_type>(i) - static_cast<difference_type>(o.i);
        }

        bool operator==(const iterator& o) const { return i == o.i; }
        bool operator!=(const iterator& o) const { return i != o.i; }
        bool operator<(const iterator& o) const { return i < o.i; }
        bool operator>(const iterator& o) const { return i > o.i; }
        bool operator<=(const iterator& o) const { return i <= o.i; }
        bool operator>=(const iterator& o) const { return i >= o.i; }
    };

    class const_iterator {
        const PackedVector* p;
        size_t i;
        friend class PackedVector;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = T;

        const_iterator() : p(nullptr), i(0) {}
        const_iterator(const PackedVector* v, size_t idx) : p(v), i(idx) {}
        const_iterator(const iterator& it) : p(it.p), i(it.i) {}

        T operator*() const { return p->read(i); }
        T operator[](difference_type n) const { return p->read(i + n); }

        const_iterator& operator++() { ++i; return *this; }
        const_iterator operator++(int) { auto t = *this; ++i; return t; }
        const_iterator& operator--() { --i; return *this; }
        const_iterator operator--(int) { auto t = *this; --i; return t; }

        const_iterator& operator+=(difference_type n) { i += n; return *this; }
        const_iterator& operator-=(difference_type n) { i -= n; return *this; }

        const_iterator operator+(difference_type n) const { return {p, i + n}; }
        const_iterator operator-(difference_type n) const { return {p, i - n}; }
        friend const_iterator operator+(difference_type n, const const_iterator& it) {
            return {it.p, it.i + n};
        }
        difference_type operator-(const const_iterator& o) const {
            return static_cast<difference_type>(i) - static_cast<difference_type>(o.i);
        }

        bool operator==(const const_iterator& o) const { return i == o.i; }
        bool operator!=(const const_iterator& o) const { return i != o.i; }
        bool operator<(const const_iterator& o) const { return i < o.i; }
        bool operator>(const const_iterator& o) const { return i > o.i; }
        bool operator<=(const const_iterator& o) const { return i <= o.i; }
        bool operator>=(const const_iterator& o) const { return i >= o.i; }
    };

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    PackedVector() noexcept : data_(nullptr), size_(0), capacity_(0) {}

    explicit PackedVector(size_t n, T v = 0) : data_(nullptr), size_(0), capacity_(0) {
        if (n) {
            reserve(n);
            size_ = n;
            for (size_t i = 0; i < n; ++i) write(i, v);
        }
    }

    PackedVector(const PackedVector& o) : data_(nullptr), size_(0), capacity_(0) {
        if (o.capacity_) {
            size_t bc = blocks(o.capacity_);
            data_ = new T[bc];
            std::copy(o.data_, o.data_ + bc, data_);
            size_ = o.size_;
            capacity_ = o.capacity_;
        }
    }

    PackedVector(PackedVector&& o) noexcept
        : data_(o.data_), size_(o.size_), capacity_(o.capacity_) {
        o.data_ = nullptr;
        o.size_ = 0;
        o.capacity_ = 0;
    }

    PackedVector& operator=(const PackedVector& o) {
        if (this != &o) {
            PackedVector t(o);
            swap(t);
        }
        return *this;
    }

    PackedVector& operator=(PackedVector&& o) noexcept {
        if (this != &o) {
            delete[] data_;
            data_ = o.data_;
            size_ = o.size_;
            capacity_ = o.capacity_;
            o.data_ = nullptr;
            o.size_ = 0;
            o.capacity_ = 0;
        }
        return *this;
    }

    ~PackedVector() { delete[] data_; }

    reference operator[](size_t i) { return reference(this, i); }
    T operator[](size_t i) const { return read(i); }

    reference at(size_t i) {
        if (i >= size_) throw std::out_of_range("Неверный индекс - at!");
        return reference(this, i);
    }
    T at(size_t i) const {
        if (i >= size_) throw std::out_of_range("Неверный индекс!- at");
        return read(i);
    }

    void reserve(size_t n) {
        if (n > capacity_) {
            size_t bc = blocks(n);
            T* nd = new T[bc]();
            if (data_) {
                std::copy(data_, data_ + blocks(capacity_), nd);
                delete[] data_;
            }
            data_ = nd;
            capacity_ = n;
        }
    }

    void resize(size_t n, T v = 0) {
        if (n > capacity_) reserve(n);
        for (size_t i = size_; i < n; ++i) write(i, v);
        size_ = n;
    }

    void shrink_to_fit() {
        if (size_ < capacity_) {
            if (!size_) {
                delete[] data_;
                data_ = nullptr;
                capacity_ = 0;
            } else {
                size_t bc = blocks(size_);
                T* nd = new T[bc];
                std::copy(data_, data_ + bc, nd);
                delete[] data_;
                data_ = nd;
                capacity_ = size_;
            }
        }
    }

    bool empty() const noexcept { return size_ == 0; }
    size_t size() const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }

    void push_back(T v) {
        if (size_ == capacity_) reserve(capacity_ ? capacity_ * 2 : 1);
        write(size_++, v);
    }

    iterator insert(const_iterator pos, T v) {
        size_t idx = pos - cbegin();
        if (idx > size_) throw std::out_of_range("Неверный индекс!- insert");
        if (size_ == capacity_) reserve(capacity_ ? capacity_ * 2 : 1);
        for (size_t i = size_; i > idx; --i) write(i, read(i - 1));
        write(idx, v);
        ++size_;
        return {this, idx};
    }

    iterator erase(const_iterator pos) {
        size_t idx = pos - cbegin();
        if (idx >= size_) throw std::out_of_range("Неверный индекс!- erase");
        for (size_t i = idx; i < size_ - 1; ++i) write(i, read(i + 1));
        --size_;
        return {this, idx};
    }

    void swap(PackedVector& o) noexcept {
        std::swap(data_, o.data_);
        std::swap(size_, o.size_);
        std::swap(capacity_, o.capacity_);
    }

    iterator begin() noexcept { return {this, 0}; }
    iterator end() noexcept { return {this, size_}; }
    const_iterator begin() const noexcept { return {this, 0}; }
    const_iterator end() const noexcept { return {this, size_}; }
    const_iterator cbegin() const noexcept { return {this, 0}; }
    const_iterator cend() const noexcept { return {this, size_}; }

    reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
    const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }
};