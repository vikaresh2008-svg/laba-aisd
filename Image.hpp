#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <iostream>
#include <random>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <type_traits>

template <typename T>
class Image {
private:
    static constexpr double EPSILON = 1e-6;
    size_t width_;
    size_t height_;
    T* data_;

    T addSaturated(T val1, T val2) const {
        if constexpr (std::is_same_v<T, bool>) {
            return val1 || val2;
        } else {
            T maxVal = std::numeric_limits<T>::max();
            T minVal = std::numeric_limits<T>::lowest();
            if (val2 > static_cast<T>(0) && val1 > maxVal - val2) return maxVal;
            if (val2 < static_cast<T>(0) && val1 < minVal - val2) return minVal;
            return static_cast<T>(val1 + val2);
        }
    }

    T multiplySaturated(T val1, T val2) const {
        if constexpr (std::is_same_v<T, bool>) {
            return val1 && val2;
        } else {
            T maxVal = std::numeric_limits<T>::max();
            T minVal = std::numeric_limits<T>::lowest();
            double prod = static_cast<double>(val1) * static_cast<double>(val2);
            if (prod > static_cast<double>(maxVal)) return maxVal;
            if (prod < static_cast<double>(minVal)) return minVal;
            return static_cast<T>(prod);
        }
    }

public:
    explicit Image(size_t width = 0, size_t height = 0, bool randomFill = false)
        : width_(width), height_(height), data_(nullptr) {
        if (width_ > 0 && height_ > 0) {
            data_ = new T[width_ * height_]();
            if (randomFill) {
                std::random_device rd;
                std::mt19937 gen(rd());
                if constexpr (std::is_same_v<T, bool>) {
                    std::bernoulli_distribution dist(0.5);
                    for (size_t i = 0; i < width_ * height_; ++i) data_[i] = dist(gen);
                } else if constexpr (std::is_integral_v<T>) {
                    using DistType = std::conditional_t<sizeof(T) == 1, int, T>;
                    std::uniform_int_distribution<DistType> dist(0, 50);
                    for (size_t i = 0; i < width_ * height_; ++i) data_[i] = static_cast<T>(dist(gen));
                } else {
                    std::uniform_real_distribution<T> dist(0.0, 1.0);
                    for (size_t i = 0; i < width_ * height_; ++i) data_[i] = dist(gen);
                }
            }
        }
    }

    Image(const Image& other)
        : width_(other.width_), height_(other.height_), data_(nullptr) {
        if (width_ > 0 && height_ > 0) {
            data_ = new T[width_ * height_];
            for (size_t i = 0; i < width_ * height_; ++i) data_[i] = other.data_[i];
        }
    }

    Image& operator=(const Image& other) {
        if (this != &other) {
            T* newData = nullptr;
            if (other.width_ > 0 && other.height_ > 0) {
                newData = new T[other.width_ * other.height_];
                for (size_t i = 0; i < other.width_ * other.height_; ++i) newData[i] = other.data_[i];
            }
            delete[] data_;
            data_ = newData;
            width_ = other.width_;
            height_ = other.height_;
        }
        return *this;
    }

    ~Image() { delete[] data_; }

    size_t getWidth() const { return width_; }
    size_t getHeight() const { return height_; }

    T& operator()(size_t row, size_t col) {
        if (row >= height_ || col >= width_) throw std::out_of_range("Index out of bounds");
        return data_[row * width_ + col];
    }

    const T& operator()(size_t row, size_t col) const {
        if (row >= height_ || col >= width_) throw std::out_of_range("Index out of bounds");
        return data_[row * width_ + col];
    }

    bool operator==(const Image& other) const {
        if (width_ != other.width_ || height_ != other.height_) return false;
        for (size_t i = 0; i < width_ * height_; ++i) {
            if constexpr (std::is_floating_point_v<T>) {
                if (std::abs(data_[i] - other.data_[i]) > EPSILON) return false;
            } else {
                if (data_[i] != other.data_[i]) return false;
            }
        }
        return true;
    }

    bool operator!=(const Image& other) const { return !(*this == other); }

    Image operator+(const Image& other) const {
        size_t resWidth = std::max(width_, other.width_);
        size_t resHeight = std::max(height_, other.height_);
        Image result(resWidth, resHeight, false);
        for (size_t r = 0; r < resHeight; ++r) {
            for (size_t c = 0; c < resWidth; ++c) {
                T val1 = (r < height_ && c < width_) ? (*this)(r, c) : static_cast<T>(0);
                T val2 = (r < other.height_ && c < other.width_) ? other(r, c) : static_cast<T>(0);
                result(r, c) = addSaturated(val1, val2);
            }
        }
        return result;
    }

    Image operator*(const Image& other) const {
        size_t resWidth = std::max(width_, other.width_);
        size_t resHeight = std::max(height_, other.height_);
        Image result(resWidth, resHeight, false);
        for (size_t r = 0; r < resHeight; ++r) {
            for (size_t c = 0; c < resWidth; ++c) {
                T val1 = (r < height_ && c < width_) ? (*this)(r, c) : static_cast<T>(0);
                T val2 = (r < other.height_ && c < other.width_) ? other(r, c) : static_cast<T>(0);
                result(r, c) = multiplySaturated(val1, val2);
            }
        }
        return result;
    }

    Image operator+(T val) const {
        Image result(width_, height_, false);
        for (size_t i = 0; i < width_ * height_; ++i) result.data_[i] = addSaturated(data_[i], val);
        return result;
    }

    Image operator*(T val) const {
        Image result(width_, height_, false);
        for (size_t i = 0; i < width_ * height_; ++i) result.data_[i] = multiplySaturated(data_[i], val);
        return result;
    }

    Image operator!() const {
        Image result(width_, height_, false);
        for (size_t i = 0; i < width_ * height_; ++i) {
            if constexpr (std::is_same_v<T, bool>) result.data_[i] = !data_[i];
            else result.data_[i] = static_cast<T>(-data_[i]);
        }
        return result;
    }

    double getFillFactor() const {
        if (width_ == 0 || height_ == 0) return 0.0;
        double sum = 0.0;
        for (size_t i = 0; i < width_ * height_; ++i) sum += static_cast<double>(data_[i]);
        double maxVal = std::is_same_v<T, bool> ? 1.0 : static_cast<double>(std::numeric_limits<T>::max());
        return sum / (static_cast<double>(width_ * height_) * maxVal);
    }

    friend std::ostream& operator<<(std::ostream& os, const Image& img) {
        for (size_t r = 0; r < img.height_; ++r) {
            for (size_t c = 0; c < img.width_; ++c) {
                if constexpr (std::is_same_v<T, char> || std::is_same_v<T, unsigned char>) {
                    os << static_cast<int>(img(r, c)) << "\t";
                } else {
                    os << img(r, c) << "\t";
                }
            }
            os << "\n";
        }
        return os;
    }
};

template <typename T>
Image<T> operator+(T val, const Image<T>& img) { return img + val; }

template <typename T>
Image<T> operator*(T val, const Image<T>& img) { return img * val; }

template <typename T>
void drawFilledCircle(Image<T>& img, int centerX, int centerY, int radius, T fillColor) {
    if (radius < 0) throw std::invalid_argument("Radius cannot be negative");
    int imgWidth = static_cast<int>(img.getWidth());
    int imgHeight = static_cast<int>(img.getHeight());
    int startY = std::max(0, centerY - radius);
    int endY = std::min(imgHeight - 1, centerY + radius);
    int startX = std::max(0, centerX - radius);
    int endX = std::min(imgWidth - 1, centerX + radius);
    for (int y = startY; y <= endY; ++y) {
        for (int x = startX; x <= endX; ++x) {
            int dx = x - centerX;
            int dy = y - centerY;
            if (dx * dx + dy * dy <= radius * radius) {
                img(static_cast<size_t>(y), static_cast<size_t>(x)) = fillColor;
            }
        }
    }
}

#endif