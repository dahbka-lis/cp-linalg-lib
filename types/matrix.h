#pragma once

#include "matrix_view.h"
#include "types_details.h"

#include <ostream>
#include <vector>

namespace matrix_lib {
template <utils::FloatOrComplex T = long double>
class Matrix {
    using Data = std::vector<T>;
    using IndexType = details::Types::IndexType;
    using Segment = details::Types::Segment;
    using Function = details::Types::Function<T>;
    using FunctionIndexes = details::Types::FunctionIndexes<T>;
    using ConstFunction = details::Types::ConstFunction<T>;
    using ConstFunctionIndexes = details::Types::ConstFunctionIndexes<T>;

public:
    Matrix() = default;

    explicit Matrix(IndexType sq_size)
        : cols_(CorrectSize(sq_size)), buffer_(cols_ * cols_, T{0}) {
    }

    Matrix(IndexType row_cnt, IndexType col_cnt, T value = T{0})
        : cols_(CorrectSize(col_cnt)),
          buffer_(cols_ * CorrectSize(row_cnt), value) {
        if (row_cnt == IndexType{0}) {
            cols_ = IndexType{0};
        }
    }

    Matrix(std::initializer_list<std::initializer_list<T>> list)
        : cols_(list.begin()->size()) {
        buffer_.reserve(Columns() * list.size());

        for (auto sublist : list) {
            assert(
                sublist.size() == cols_ &&
                "Size of matrix rows must be equal to the number of columns.");

            for (auto value : sublist) {
                buffer_.push_back(value);
            }
        }
    }

    Matrix(const ConstMatrixView<T> &rhs) : Matrix(rhs.Rows(), rhs.Columns()) {
        rhs.ApplyToEach([&](const T &val, IndexType i, IndexType j) {
            (*this)(i, j) = val;
        });
    }

    Matrix(const MatrixView<T> &rhs) : Matrix(rhs.ConstView()) {
    }

    Matrix(const Matrix &rhs) = default;

    Matrix(Matrix &&rhs) noexcept
        : cols_(std::exchange(rhs.cols_, 0)), buffer_(std::move(rhs.buffer_)) {
    }

    Matrix &operator=(const Matrix &rhs) = default;

    Matrix &operator=(Matrix &&rhs) noexcept {
        cols_ = std::exchange(rhs.cols_, 0);
        buffer_ = std::move(rhs.buffer_);
        return *this;
    }

    Matrix &operator+=(const ConstMatrixView<T> &rhs) {
        View() += rhs;
        return *this;
    }

    Matrix &operator+=(const MatrixView<T> &rhs) {
        return *this += rhs.ConstView();
    }

    Matrix &operator+=(const Matrix &rhs) {
        return *this += rhs.View();
    }

    friend Matrix operator+(const Matrix<T> &lhs, const Matrix<T> &rhs) {
        return lhs.View() + rhs.View();
    }

    friend Matrix operator+(const Matrix<T> &lhs,
                            const ConstMatrixView<T> &rhs) {
        return rhs + lhs;
    }

    friend Matrix operator+(const Matrix<T> &lhs, const MatrixView<T> &rhs) {
        return rhs + lhs;
    }

    Matrix &operator-=(const ConstMatrixView<T> &rhs) {
        View() -= rhs;
        return *this;
    }

    Matrix &operator-=(const MatrixView<T> &rhs) {
        return *this -= rhs.ConstView();
    }

    Matrix &operator-=(const Matrix &rhs) {
        return *this -= rhs.View();
    }

    friend Matrix operator-(const Matrix<T> &lhs, const Matrix<T> &rhs) {
        return lhs.View() - rhs.View();
    }

    friend Matrix operator-(const Matrix<T> &lhs,
                            const ConstMatrixView<T> &rhs) {
        return lhs.View() - rhs;
    }

    friend Matrix operator-(const Matrix<T> &lhs, const MatrixView<T> &rhs) {
        return lhs.View() - rhs.ConstView();
    }

    Matrix &operator*=(const ConstMatrixView<T> &rhs) {
        return *this = *this * rhs;
    }

    Matrix &operator*=(const MatrixView<T> &rhs) {
        return *this *= rhs.ConstView();
    }

    Matrix &operator*=(const Matrix &rhs) {
        return *this *= rhs.View();
    }

    friend Matrix operator*(const Matrix &lhs, const Matrix &rhs) {
        return lhs.View() * rhs.View();
    }

    friend Matrix operator*(const Matrix &lhs, const MatrixView<T> &rhs) {
        return lhs.View() * rhs.ConstView();
    }

    friend Matrix operator*(const Matrix &lhs, const ConstMatrixView<T> &rhs) {
        return lhs.View() * rhs;
    }

    Matrix &operator*=(T scalar) {
        View() *= scalar;
        return *this;
    }

    friend Matrix<T> operator*(const Matrix &lhs, T scalar) {
        return lhs.View() * scalar;
    }

    friend Matrix<T> operator*(T scalar, const Matrix &lhs) {
        return lhs.View() * scalar;
    }

    Matrix &operator/=(T scalar) {
        View() /= scalar;
        return *this;
    }

    friend Matrix<T> operator/(const Matrix &lhs, T scalar) {
        return lhs.View() / scalar;
    }

    friend Matrix<T> operator/(T scalar, const Matrix &lhs) {
        return lhs.View() / scalar;
    }

    friend bool operator==(const Matrix &lhs, const Matrix &rhs) {
        return lhs.buffer_ == rhs.buffer_;
    }

    friend bool operator==(const Matrix &lhs, const ConstMatrixView<T> &rhs) {
        return lhs.View() == rhs;
    }

    friend bool operator==(const Matrix &lhs, const MatrixView<T> &rhs) {
        return lhs.View() == rhs.ConstView();
    }

    friend bool operator!=(const Matrix &lhs, const Matrix &rhs) {
        return !(lhs == rhs);
    }

    friend bool operator!=(const Matrix &lhs, const ConstMatrixView<T> &rhs) {
        return !(lhs == rhs);
    }

    friend bool operator!=(const Matrix &lhs, const MatrixView<T> &rhs) {
        return !(lhs == rhs);
    }

    T &operator()(IndexType row_idx, IndexType col_idx) {
        assert(Columns() * row_idx + col_idx < buffer_.size() &&
               "Requested indexes are outside the matrix boundaries.");
        return buffer_[Columns() * row_idx + col_idx];
    }

    T operator()(IndexType row_idx, IndexType col_idx) const {
        assert(Columns() * row_idx + col_idx < buffer_.size() &&
               "Requested indexes are outside the matrix boundaries.");
        return buffer_[Columns() * row_idx + col_idx];
    }

    [[nodiscard]] IndexType Rows() const {
        return (cols_ == 0) ? IndexType{0} : buffer_.size() / cols_;
    }

    [[nodiscard]] IndexType Columns() const {
        return cols_;
    }

    MatrixView<T> View() {
        return MatrixView<T>(*this);
    }

    ConstMatrixView<T> View() const {
        return ConstMatrixView<T>(*this);
    }

    Matrix &ApplyToEach(Function func) {
        View().ApplyToEach(func);
        return *this;
    }

    Matrix &ApplyToEach(FunctionIndexes func) {
        View().ApplyToEach(func);
        return *this;
    }

    Matrix &ApplyToEach(ConstFunction func) const {
        View().ApplyToEach(func);
        return *this;
    }

    Matrix &ApplyToEach(ConstFunctionIndexes func) const {
        View().ApplyToEach(func);
        return *this;
    }

    T GetEuclideanNorm() const {
        return View().GetEuclideanNorm();
    }

    Matrix GetDiag() const {
        return View().GetDiag();
    }

    MatrixView<T> GetRow(IndexType index) {
        return View().GetRow(index);
    }

    MatrixView<T> GetColumn(IndexType index) {
        return View().GetColumn(index);
    }

    MatrixView<T> GetSubmatrix(Segment row, Segment col) {
        return View().GetSubmatrix(row, col);
    }

    ConstMatrixView<T> GetRow(IndexType index) const {
        return View().GetRow(index);
    }

    ConstMatrixView<T> GetColumn(IndexType index) const {
        return View().GetColumn(index);
    }

    ConstMatrixView<T> GetSubmatrix(Segment row, Segment col) const {
        return View().GetSubmatrix(row, col);
    }

    Matrix &Transpose() {
        std::vector<bool> visited(buffer_.size(), false);
        IndexType last_idx = buffer_.size() - 1;

        for (IndexType i = 1; i < buffer_.size(); ++i) {
            if (visited[i]) {
                continue;
            }

            auto swap_idx = i;
            do {
                swap_idx = (swap_idx == last_idx)
                               ? last_idx
                               : (Rows() * swap_idx) % last_idx;
                std::swap(buffer_[swap_idx], buffer_[i]);
                visited[swap_idx] = true;
            } while (swap_idx != i);
        }

        cols_ = Rows();
        return *this;
    }

    Matrix &Conjugate() {
        Transpose();

        if constexpr (utils::details::IsFloatComplexT<T>::value) {
            ApplyToEach([](T &val) { val = std::conj(val); });
        }

        return *this;
    }

    Matrix &Normalize() {
        View().Normalize();
        return *this;
    }

    Matrix &RoundZeroes() {
        View().RoundZeroes();
        return *this;
    }

    static MatrixView<T> Transposed(MatrixView<T> &rhs) {
        auto view = rhs;
        view.Transpose();
        return view;
    }

    static MatrixView<T> Transposed(Matrix<T> &rhs) {
        MatrixView<T> view = rhs.View();
        return Matrix::Transposed(view);
    }

    static ConstMatrixView<T> Transposed(const ConstMatrixView<T> &rhs) {
        ConstMatrixView<T> res = ConstMatrixView<T>(
            *rhs.ptr_, rhs.column_, rhs.row_,
            {!rhs.state_.is_transposed, rhs.state_.is_conjugated});
        return res;
    }

    static ConstMatrixView<T> Transposed(const MatrixView<T> &rhs) {
        ConstMatrixView<T> view = rhs.ConstView();
        return Matrix::Transposed(view);
    }

    static ConstMatrixView<T> Transposed(const Matrix<T> &rhs) {
        ConstMatrixView<T> view = rhs.View();
        return Matrix::Transposed(view);
    }

    static MatrixView<T> Conjugated(MatrixView<T> &rhs) {
        auto view = rhs;
        view.Conjugate();
        return view;
    }

    static MatrixView<T> Conjugated(Matrix<T> &rhs) {
        auto view = rhs.View();
        return Matrix::Conjugated(view);
    }

    static ConstMatrixView<T> Conjugated(const ConstMatrixView<T> &rhs) {
        ConstMatrixView<T> res = ConstMatrixView<T>(
            *rhs.ptr_, rhs.column_, rhs.row_,
            {!rhs.state_.is_transposed, !rhs.state_.is_conjugated});
        return res;
    }

    static ConstMatrixView<T> Conjugated(const MatrixView<T> &rhs) {
        ConstMatrixView<T> view = rhs.ConstView();
        return Matrix::Conjugated(view);
    }

    static ConstMatrixView<T> Conjugated(const Matrix<T> &rhs) {
        ConstMatrixView<T> view = rhs.View();
        return Matrix::Conjugated(view);
    }

    static Matrix Normalized(const Matrix &rhs) {
        return Matrix::Normalized(rhs.View());
    }

    static Matrix Normalized(const MatrixView<T> &rhs) {
        return Matrix::Normalized(rhs.ConstView());
    }

    static Matrix Normalized(const ConstMatrixView<T> &rhs) {
        Matrix res = rhs;
        res.Normalize();
        return res;
    }

    static Matrix Identity(IndexType size) {
        Matrix res(size);
        res.ApplyToEach([&](T &el, IndexType i, IndexType j) {
            el = (i == j) ? T{1} : T{0};
        });
        return res;
    }

    static Matrix Diagonal(const Matrix &vec) {
        return Diagonal(vec.View());
    }

    static Matrix Diagonal(const MatrixView<T> &vec) {
        return Diagonal(vec.ConstView());
    }

    static Matrix Diagonal(const ConstMatrixView<T> &vec) {
        assert(vec.Rows() == 1 ||
               vec.Columns() == 1 &&
                   "Creating a diagonal matrix for vectors only.");

        Matrix<T> res(std::max(vec.Rows(), vec.Columns()));
        vec.ApplyToEach([&](const T &val, IndexType i, IndexType j) {
            auto idx = std::max(i, j);
            res(idx, idx) = val;
        });

        return res;
    }

    friend std::ostream &operator<<(std::ostream &ostream,
                                    const Matrix &matrix) {
        ostream << '[';
        for (std::size_t i = 0; i < matrix.Rows(); ++i) {
            ostream << '[';
            for (std::size_t j = 0; j < matrix.Columns(); ++j) {
                ostream << matrix(i, j);
                if (j + 1 < matrix.Columns()) {
                    ostream << ' ';
                }
            }

            ostream << ']';
            if (i + 1 < matrix.Rows()) {
                ostream << '\n';
            }
        }
        ostream << ']';
        return ostream;
    }

private:
    static IndexType CorrectSize(IndexType size) {
        return std::max(IndexType{0}, size);
    }

    IndexType cols_ = 0;
    Data buffer_;
};
} // namespace matrix_lib
