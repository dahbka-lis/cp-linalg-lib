#pragma once

#include "matrix_view.h"
#include "types_details.h"

namespace matrix_lib {
template <utils::FloatOrComplex T = long double>
class ConstMatrixView {
    friend class Matrix<T>;
    friend class MatrixView<T>;

    using IndexType = details::Types::IndexType;
    using Segment = details::Types::Segment;
    using MatrixState = details::Types::MatrixState;
    using ConstFunction = details::Types::ConstFunction<T>;
    using ConstFunctionIndexes = details::Types::ConstFunctionIndexes<T>;

public:
    explicit ConstMatrixView(const Matrix<T> &matrix, Segment row = {-1, -1},
                             Segment col = {-1, -1},
                             MatrixState state = {false, false})
        : ptr_(&matrix), state_(state) {
        row_ = MakeSegment(row, (state_.is_transposed) ? matrix.Columns()
                                                       : matrix.Rows());
        column_ = MakeSegment(col, (state_.is_transposed) ? matrix.Rows()
                                                          : matrix.Columns());
    }

    ConstMatrixView(const ConstMatrixView &rhs) = default;

    ConstMatrixView(ConstMatrixView &&rhs) noexcept
        : ptr_(std::exchange(rhs.ptr_, nullptr)),
          row_(std::exchange(rhs.row_, {0, 1})),
          column_(std::exchange(rhs.column_, {0, 1})),
          state_(std::exchange(rhs.state_, MatrixState{})){};

    ConstMatrixView &operator=(const ConstMatrixView &lhs) = default;

    ConstMatrixView &operator=(ConstMatrixView &&rhs) noexcept {
        ptr_ = std::exchange(rhs.ptr_, nullptr);
        row_ = std::exchange(rhs.row_, {0, 1});
        column_ = std::exchange(rhs.column_, {0, 1});
        state_ = std::exchange(rhs.state_, MatrixState{});
        return *this;
    }

    friend Matrix<T> operator+(const ConstMatrixView<T> &lhs,
                               const ConstMatrixView<T> &rhs) {
        Matrix<T> res = lhs;
        res += rhs;
        return res;
    }

    friend Matrix<T> operator+(const ConstMatrixView<T> &lhs,
                               const MatrixView<T> &rhs) {
        return lhs + rhs.ConstView();
    }

    friend Matrix<T> operator+(const ConstMatrixView<T> &lhs,
                               const Matrix<T> &rhs) {
        return lhs + rhs.View();
    }

    friend Matrix<T> operator-(const ConstMatrixView<T> &lhs,
                               const ConstMatrixView<T> &rhs) {
        assert(lhs.Rows() == rhs.Rows() && lhs.Columns() == rhs.Columns() &&
               "Matrices must be of the same size for addition.");
        Matrix<T> res = lhs;
        res -= rhs;
        return res;
    }

    friend Matrix<T> operator-(const ConstMatrixView<T> &lhs,
                               const MatrixView<T> &rhs) {
        return lhs - rhs.ConstView();
    }

    friend Matrix<T> operator-(const ConstMatrixView<T> &lhs,
                               const Matrix<T> &rhs) {
        return lhs - rhs.View();
    }

    friend Matrix<T> operator*(const ConstMatrixView &lhs,
                               const ConstMatrixView &rhs) {
        assert(lhs.Columns() == rhs.Rows() &&
               "Matrix multiplication mismatch.");

        if (lhs.Rows() == 0 || rhs.Columns() == 0) {
            return Matrix<T>();
        }

        Matrix<T> result(lhs.Rows(), rhs.Columns());

        for (IndexType i = 0; i < lhs.Rows(); ++i) {
            for (IndexType j = 0; j < rhs.Columns(); ++j) {
                T sum = 0;
                for (IndexType k = 0; k < lhs.Columns(); ++k) {
                    sum += lhs(i, k) * rhs(k, j);
                }
                result(i, j) = sum;
            }
        }

        result.RoundZeroes();
        return result;
    }

    friend Matrix<T> operator*(const ConstMatrixView &lhs,
                               const MatrixView<T> &rhs) {
        return lhs * rhs.ConstView();
    }

    friend Matrix<T> operator*(const ConstMatrixView &lhs,
                               const Matrix<T> &rhs) {
        return lhs * rhs.View();
    }

    friend Matrix<T> operator*(const ConstMatrixView &lhs, T scalar) {
        Matrix<T> res = lhs;
        res *= scalar;
        return res;
    }

    friend Matrix<T> operator*(T scalar, const ConstMatrixView &lhs) {
        return lhs * scalar;
    }

    friend Matrix<T> operator/(const ConstMatrixView &lhs, T scalar) {
        Matrix<T> res = lhs;
        res /= scalar;
        return res;
    }

    friend Matrix<T> operator/(T scalar, const ConstMatrixView &lhs) {
        return lhs / scalar;
    }

    friend bool operator==(const ConstMatrixView &lhs,
                           const ConstMatrixView &rhs) {
        if (lhs.Rows() != rhs.Rows() || lhs.Columns() != rhs.Columns()) {
            return false;
        }

        bool is_equal = true;
        lhs.ApplyToEach([&](const T &val, IndexType i, IndexType j) {
            if (!utils::IsEqualFloating(val, rhs(i, j))) {
                is_equal = false;
                return;
            }
        });

        return is_equal;
    }

    friend bool operator==(const ConstMatrixView &lhs,
                           const MatrixView<T> &rhs) {
        return lhs == rhs.ConstView();
    }

    friend bool operator==(const ConstMatrixView &lhs, const Matrix<T> &rhs) {
        return lhs == rhs.View();
    }

    friend bool operator!=(const ConstMatrixView &lhs,
                           const ConstMatrixView &rhs) {
        return !(lhs == rhs);
    }

    friend bool operator!=(const ConstMatrixView &lhs,
                           const MatrixView<T> &rhs) {
        return !(lhs == rhs);
    }

    friend bool operator!=(const ConstMatrixView &lhs, const Matrix<T> &rhs) {
        return !(lhs == rhs);
    }

    T operator()(IndexType row_idx, IndexType col_idx) const {
        assert(!IsNullMatrixPointer() && "Matrix pointer is null.");

        if constexpr (utils::details::IsFloatComplexT<T>::value) {
            if (state_.is_transposed && state_.is_conjugated) {
                return std::conj(
                    (*ptr_)(row_.begin + col_idx, column_.begin + row_idx));
            } else if (state_.is_transposed) {
                return (*ptr_)(row_.begin + col_idx, column_.begin + row_idx);
            } else if (state_.is_conjugated) {
                return std::conj(
                    (*ptr_)(row_.begin + row_idx, column_.begin + col_idx));
            }

            return (*ptr_)(row_.begin + row_idx, column_.begin + col_idx);
        }

        if (state_.is_transposed) {
            return (*ptr_)(row_.begin + col_idx, column_.begin + row_idx);
        }

        return (*ptr_)(row_.begin + row_idx, column_.begin + col_idx);
    }

    [[nodiscard]] IndexType Rows() const {
        return row_.end - row_.begin;
    }

    [[nodiscard]] IndexType Columns() const {
        return column_.end - column_.begin;
    }

    const ConstMatrixView &ApplyToEach(ConstFunction func) const {
        assert(!IsNullMatrixPointer() && "Matrix pointer is null.");

        for (IndexType i = 0; i < Rows(); ++i) {
            for (IndexType j = 0; j < Columns(); ++j) {
                func((*this)(i, j));
            }
        }

        return *this;
    }

    const ConstMatrixView &ApplyToEach(ConstFunctionIndexes func) const {
        assert(!IsNullMatrixPointer() && "Matrix pointer is null.");

        for (IndexType i = 0; i < Rows(); ++i) {
            for (IndexType j = 0; j < Columns(); ++j) {
                func((*this)(i, j), i, j);
            }
        }

        return *this;
    }

    T GetEuclideanNorm() const {
        assert(Rows() == 1 ||
               Columns() == 1 && "Euclidean norm only for vectors.");

        T sq_sum = T{0};
        ApplyToEach([&](const T &value) { sq_sum += std::norm(value); });
        return std::sqrt(sq_sum);
    }

    Matrix<T> GetDiag() const {
        auto size = std::min(Rows(), Columns());

        Matrix<T> res(size, 1);
        for (IndexType i = 0; i < size; ++i) {
            res(i, 0) = (*this)(i, i);
        }

        return res;
    }

    ConstMatrixView GetRow(IndexType index) const {
        assert(!IsNullMatrixPointer() && "Matrix pointer is null.");
        assert(index < Rows() &&
               "Index must be less than the number of matrix rows.");

        return ConstMatrixView(*ptr_,
                               {row_.begin + index, row_.begin + index + 1},
                               {column_.begin, column_.end}, state_);
    }

    ConstMatrixView GetColumn(IndexType index) const {
        assert(!IsNullMatrixPointer() && "Matrix pointer is null.");
        assert(index < Columns() &&
               "Index must be less than the number of matrix columns.");

        return ConstMatrixView(
            *ptr_, {row_.begin, row_.end},
            {column_.begin + index, column_.begin + index + 1}, state_);
    }

    ConstMatrixView<T> GetSubmatrix(Segment row, Segment col) const {
        auto [r_from, r_to] = MakeSegment(row, Rows());
        auto [c_from, c_to] = MakeSegment(col, Columns());

        assert(!IsNullMatrixPointer() && "Matrix pointer is null.");
        assert(row_.begin + r_from < Rows() && "Invalid row index.");
        assert(row_.begin + r_to <= Rows() && "Invalid row index.");
        assert(column_.begin + c_from < Columns() && "Invalid column index.");
        assert(column_.begin + c_to <= Columns() && "Invalid column index.");

        return ConstMatrixView<T>(
            *ptr_, {row_.begin + r_from, row_.begin + r_to},
            {column_.begin + c_from, column_.begin + c_to}, state_);
    }

    friend std::ostream &operator<<(std::ostream &ostream,
                                    const ConstMatrixView &matrix) {
        ostream << '(';
        for (std::size_t i = 0; i < matrix.Rows(); ++i) {
            ostream << '(';
            for (std::size_t j = 0; j < matrix.Columns(); ++j) {
                ostream << matrix(i, j);
                if (j + 1 < matrix.Columns()) {
                    ostream << ' ';
                }
            }

            ostream << ')';
            if (i + 1 < matrix.Rows()) {
                ostream << '\n';
            }
        }
        ostream << ')';
        return ostream;
    }

private:
    [[nodiscard]] bool IsNullMatrixPointer() const {
        return ptr_ == nullptr;
    }

    static Segment MakeSegment(Segment seg, IndexType max_value) {
        if (seg.end <= 0 || seg.end > max_value) {
            seg.end = max_value;
        }

        if (seg.begin >= seg.end || seg.begin >= max_value || seg.begin < 0) {
            seg.begin = 0;
        }

        return seg;
    }

    const Matrix<T> *ptr_;
    Segment row_;
    Segment column_;
    MatrixState state_;
};
} // namespace matrix_lib
