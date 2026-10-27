#ifndef SVKFW_MATRICES_H
#define SVKFW_MATRICES_H

#include "vectors.h"

#include <stdexcept>


namespace Simple {
    struct MatrixSize {
        unsigned dim = 0u;
        unsigned *size = nullptr;


        MatrixSize();
        MatrixSize(unsigned _s);
        MatrixSize(const std::initializer_list<unsigned> &_size);
        MatrixSize(const MatrixSize &_size);
        ~MatrixSize();


        unsigned &operator[](unsigned ind);
        unsigned  operator[](unsigned ind) const;

        const MatrixSize &operator=(unsigned _s);

        const MatrixSize &operator=(const std::initializer_list<unsigned> &_size);

        const MatrixSize &operator=(const MatrixSize &_size);

        bool operator==(const MatrixSize &_size);

        unsigned long elemsize() const;

        bool reshape(unsigned _s);

        bool reshape(const std::initializer_list<unsigned> &_size);

        bool reshape(const MatrixSize &_size);
    }; // MatrixSize

    //TODO: mInd() factory function
    struct MatrixIndex {
        long index, end, step;

        MatrixIndex(Mat::Placeholder);
        MatrixIndex(long _ind, Mat::Placeholder);
        MatrixIndex(long _ind, Mat::Placeholder, long _step);
        MatrixIndex(long _ind);
        MatrixIndex(long _ind, long _end);
        MatrixIndex(long _ind, long _end, long _step);

        // returns {index, end, step}
        vec3l calcParams(unsigned _dimsize) const;

        unsigned len(unsigned _dimsize) const;
    }; // MatrixIndex END

    struct MatrixIterNDim {
        unsigned long index = 0ul, first = 0ul;
        unsigned dim = 0u;
        vec3l *left_stepsize_len = nullptr;

        MatrixIterNDim();
        MatrixIterNDim(const MatrixIterNDim &_rng);
        MatrixIterNDim(const MatrixSize &_s, const std::initializer_list<MatrixIndex> &_i, unsigned long _beg = 0ul);
        ~MatrixIterNDim();

        void restart();

        MatrixSize newSize();

        MatrixIterNDim& operator++();
        //RangeNDim operator++(int) { RangeNDim retval{*this}; this->operator++(); return retval; }
        //bool operator==(const RangeNDim &other) const { return !left_stepsize_len[dim - 1].x; }

        //TODO
        MatrixIterNDim operator<<(const std::initializer_list<MatrixIndex> &_i);

        bool operator!=(const MatrixIterNDim &other) const;
        unsigned long operator*() const;
        MatrixIterNDim begin();
        MatrixIterNDim end();
    }; // MatrixIterNDim END



    template <typename T>
    struct MatrixBase {
        MatrixSize shape;
        T *M = nullptr;
        MatrixIterNDim iter;
        bool is_view = false;

        MatrixBase() {}
        MatrixBase(const MatrixSize &_size, T _val = 0) : shape{_size}, M{new T[_size.elemsize()]} {
            for (unsigned long i = 0ul; i < _size.elemsize(); ++i)
                M[i] = _val;
        }
        template <typename Y>
        MatrixBase(const MatrixBase<Y> &_mat, bool shallow_copy = false, const MatrixIterNDim &_iter = {}) :
                    shape{_mat.shape}, M{new T[_mat.shape.elemsize()]}, is_view{shallow_copy}, iter{_iter} {
            for (unsigned long i = 0ul; i < shape.elemsize(); ++i)
                M[i] = _mat.M[i];
        }

        template <typename Y> MatrixBase<Y> to() { return *this; }

        MatrixBase<T&> operator[](const std::initializer_list<MatrixIndex> &indices) {}
        MatrixBase<T > operator[](const std::initializer_list<MatrixIndex> &indices) const {}
    }; // MatrixBase END
}; // Simple END

#endif