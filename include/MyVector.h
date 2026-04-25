#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <utility>

template <typename T>
class MyVector {
    T* data_{nullptr};
    std::size_t size_{0};
    std::size_t capacity_{0};

    static void destroy_range(T* p, std::size_t n) noexcept {
        for (std::size_t i = 0; i < n; ++i) {
            p[i].~T();
        }
    }

    void grow() {
        std::size_t new_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
        T* new_data = static_cast<T*>(::operator new(new_cap * sizeof(T)));

        for (std::size_t i = 0; i < size_; ++i) {
            new (new_data + i) T(std::move(data_[i]));
        }
        destroy_range(data_, size_);
        ::operator delete(data_);

        data_ = new_data;
        capacity_ = new_cap;
    }

public:
    MyVector() = default;

    ~MyVector() {
        destroy_range(data_, size_);
        ::operator delete(data_);
    }

    MyVector(const MyVector& other) : size_(other.size_), capacity_(other.size_) {
        if (capacity_ == 0) {
            return;
        }
        data_ = static_cast<T*>(::operator new(capacity_ * sizeof(T)));
        for (std::size_t i = 0; i < size_; ++i) {
            new (data_ + i) T(other.data_[i]);
        }
    }

    MyVector& operator=(const MyVector& other) {
        if (this == &other) {
            return *this;
        }
        MyVector tmp(other);
        swap(tmp);
        return *this;
    }

    MyVector(MyVector&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    MyVector& operator=(MyVector&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        destroy_range(data_, size_);
        ::operator delete(data_);
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    void swap(MyVector& other) noexcept {
        using std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

    /** Текущее число элементов. */
    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    /** Зарезервированная ёмкость (число элементов, для которых выделена память). */
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
    /** Доступ по индексу без проверки границ. */
    [[nodiscard]] T& operator[](std::size_t index) noexcept { return data_[index]; }
    [[nodiscard]] const T& operator[](std::size_t index) const noexcept { return data_[index]; }
    /** Добавить элемент в конец (при необходимости увеличить буфер). */
    void push_back(const T& value) {
        if (size_ == capacity_) {
            grow();
        }
        new (data_ + size_) T(value);
        ++size_;
    }

    void push_back(T&& value) {
        if (size_ == capacity_) {
            grow();
        }
        new (data_ + size_) T(std::move(value));
        ++size_;
    }

    void pop_back() noexcept {
        if (size_ == 0) {
            return;
        }
        --size_;
        data_[size_].~T();
    }
};


template <>
class MyVector<bool> {
    std::uint8_t* bytes_{nullptr};
    std::size_t size_{0};      // число логических bool
    std::size_t capacity_{0};  // число бит, под которые выделена память

    [[nodiscard]] static constexpr std::size_t bits_to_bytes(std::size_t bits) noexcept {
        return (bits + 7) / 8;
    }

    void grow() {
        std::size_t new_cap_bits = (capacity_ == 0) ? 8 : capacity_ * 2;
        std::uint8_t* new_bytes = new std::uint8_t[bits_to_bytes(new_cap_bits)]();
        const std::size_t old_byte_len = bits_to_bytes(capacity_);
        const std::size_t new_byte_len = bits_to_bytes(new_cap_bits);
        if (bytes_ != nullptr && old_byte_len > 0) {
            std::memcpy(new_bytes, bytes_, old_byte_len);
        }
        // Обнуляем хвост при расширении
        if (new_byte_len > old_byte_len) {
            std::memset(new_bytes + old_byte_len, 0, new_byte_len - old_byte_len);
        }
        delete[] bytes_;
        bytes_ = new_bytes;
        capacity_ = new_cap_bits;
    }

public:
    
    class Reference {
        std::uint8_t* byte_;
        unsigned bit_;  // 0..7, младший бит — нулевой элемент в байте

        friend class MyVector<bool>;

        Reference(std::uint8_t* b, unsigned bit) noexcept : byte_(b), bit_(bit) {}

    public:
        /** Чтение как bool. */
        [[nodiscard]] operator bool() const noexcept {
            return static_cast<bool>((*byte_ >> bit_) & 1u);
        }

        Reference& operator=(bool value) noexcept {
            if (value) {
                *byte_ = static_cast<std::uint8_t>(*byte_ | (1u << bit_));
            } else {
                *byte_ = static_cast<std::uint8_t>(*byte_ & static_cast<std::uint8_t>(~(1u << bit_)));
            }
            return *this;
        }

        Reference& operator=(const Reference& other) noexcept {
            return operator=(static_cast<bool>(other));
        }
    };

    MyVector() = default;

    ~MyVector() { delete[] bytes_; }

    MyVector(const MyVector& other) : size_(other.size_), capacity_(other.size_) {
        if (capacity_ == 0) {
            return;
        }
        const std::size_t n = bits_to_bytes(capacity_);
        bytes_ = new std::uint8_t[n]();
        const std::size_t src_n = bits_to_bytes(other.capacity_);
        if (other.bytes_ != nullptr && src_n > 0) {
            std::memcpy(bytes_, other.bytes_, std::min(n, src_n));
        }
        const std::size_t tail = size_ % 8;
        if (tail != 0 && n > 0) {
            const std::uint8_t mask =
                static_cast<std::uint8_t>((1u << tail) - 1u);
            bytes_[n - 1] = static_cast<std::uint8_t>(bytes_[n - 1] & mask);
        }
    }

    MyVector& operator=(const MyVector& other) {
        if (this == &other) {
            return *this;
        }
        MyVector tmp(other);
        swap(tmp);
        return *this;
    }

    MyVector(MyVector&& other) noexcept : bytes_(other.bytes_), size_(other.size_), capacity_(other.capacity_) {
        other.bytes_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    MyVector& operator=(MyVector&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        delete[] bytes_;
        bytes_ = other.bytes_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.bytes_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    void swap(MyVector& other) noexcept {
        using std::swap;
        swap(bytes_, other.bytes_);
        swap(size_, other.size_);
        swap(capacity_, other.capacity_);
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }

    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }

    [[nodiscard]] Reference operator[](std::size_t index) noexcept {
        const std::size_t byte_index = index / 8;
        const unsigned bit = static_cast<unsigned>(index % 8);
        return Reference(bytes_ + byte_index, bit);
    }

    [[nodiscard]] bool operator[](std::size_t index) const noexcept {
        const std::size_t byte_index = index / 8;
        const unsigned bit = static_cast<unsigned>(index % 8);
        return static_cast<bool>((bytes_[byte_index] >> bit) & 1u);
    }

    void push_back(bool value) {
        if (size_ == capacity_) {
            grow();
        }
        operator[](size_) = value;
        ++size_;
    }

    void pop_back() noexcept {
        if (size_ == 0) {
            return;
        }
        operator[](size_ - 1) = false;
        --size_;
    }
};
