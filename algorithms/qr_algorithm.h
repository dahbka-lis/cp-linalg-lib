#pragma once

#include "../utils/checks.h"
#include "givens.h"
#include "qr_decomposition.h"

namespace matrix_lib::algorithms {
template <utils::FloatOrComplex T>
T GetWilkinsonShift(const MatrixView<T> &matrix) {
    assert(matrix.Rows() == 2 && "Wilkinson shift for 2x2 matrix.");
    assert(matrix.Columns() == 2 && "Wilkinson shift for 2x2 matrix.");
    assert(matrix(0, 1) == matrix(1, 0) &&
           "Wilkinson shift for symmetric matrix.");

    auto d = (matrix(0, 0) - matrix(1, 1)) / 2;
    auto coefficient =
        std::abs(d) + std::sqrt(d * d + matrix(0, 1) * matrix(0, 1));

    return matrix(1, 1) -
           (utils::Sign(d) * matrix(0, 1) * matrix(0, 1)) / coefficient;
}

template <utils::FloatOrComplex T>
T GetWilkinsonShift(const Matrix<T> &matrix) {
    return GetWilkinsonShift(matrix.View());
}

template <utils::FloatOrComplex T = long double>
Matrix<T> GetSchurDecomposition(const MatrixView<T> &matrix,
                                std::size_t it_cnt = 50) {
    assert(utils::IsHermitian(matrix) &&
           "Schur decomposition for hermitian matrix.");

    auto copy = matrix.Copy();
    auto last_minor = copy.GetSubmatrix(copy.Rows() - 2, copy.Rows(),
                                        copy.Columns() - 2, copy.Columns());
    auto shift = 0; // todo
    auto shift_I = Matrix<T>::Identity(copy.Rows(), shift);

    for (std::size_t i = 0; i < it_cnt; ++i) {
        auto [Q, R] = HouseholderQR(copy - shift_I);
        copy = R * Q + shift_I;
        copy.RoundZeroes();
    }

    return copy;
}

template <utils::FloatOrComplex T = long double>
Matrix<T> GetSchurDecomposition(const Matrix<T> &matrix,
                                std::size_t it_cnt = 50) {
    return GetSchurDecomposition(matrix.View(), it_cnt);
}

template <utils::FloatOrComplex T>
struct DiagBasisQR {
    Matrix<T> U;
    Matrix<T> diag;
    Matrix<T> VT;
};

template <utils::FloatOrComplex T>
DiagBasisQR<T> BidiagonalAlgorithmQR(const MatrixView<T> &B,
                                     IndexType it_cnt = 30) {
    assert(B.Rows() == 2 && B.Columns() == 2 &&
           "Bigiagonal QR algorithm for 2x2 matrix.");
    assert(utils::IsBidiagonal(B) &&
           "Bigiagonal QR algorithm for bidiagonal matrix.");

    auto S = B.Copy();
    IndexType r = S.Rows();
    IndexType c = S.Columns();
    IndexType size = std::min(r, c);

    auto U = Matrix<T>::Identity(r);
    auto VT = Matrix<T>::Identity(c);

    while (it_cnt--) {
        auto minor = S.GetSubmatrix(r - 2, r, c - 2, c);
        auto BB = Matrix<T>(2);

        BB(0, 0) = minor(0, 0) * minor(0, 0) +
                   ((r >= 3) ? S(r - 3, c - 2) * S(r - 3, c - 2) : T{0});
        BB(1, 0) = minor(0, 0) * minor(0, 1);
        BB(0, 1) = BB(1, 0);
        BB(1, 1) = minor(0, 1) * minor(0, 1) + minor(1, 1) * minor(1, 1);

        auto shift = GetWilkinsonShift(BB);

        for (IndexType i = 0; i < size; ++i) {
            if (i + 1 < S.Columns()) {
                auto f_elem = (i > 0) ? S(i - 1, i) : S(0, 0) * S(0, 0) - shift;
                auto s_elem = (i > 0) ? S(i - 1, i + 1) : S(0, 1) * S(0, 0);

                GivensLeftRotation(VT, i, i + 1, f_elem, s_elem);
                GivensRightRotation(S, i, i + 1, f_elem, s_elem);
            }

            if (i + 1 < S.Rows()) {
                GivensRightRotation(U, i, i + 1, S(i, i), S(i + 1, i));
                GivensLeftRotation(S, i, i + 1, S(i, i), S(i + 1, i));
            }
        }

        S.RoundZeroes();
    }

    return {U, S, VT};
}

template <utils::FloatOrComplex T>
DiagBasisQR<T> BidiagonalAlgorithmQR(const Matrix<T> &B,
                                     IndexType it_cnt = 30) {
    return BidiagonalAlgorithmQR(B.View(), it_cnt);
}
} // namespace matrix_lib::algorithms
