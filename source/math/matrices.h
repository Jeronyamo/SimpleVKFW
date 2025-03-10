#ifndef SVKFW_MATRICES_H
#define SVKFW_MATRICES_H

#include "vectors.h"

#include <stdexcept>


namespace Simple {
    struct MatrixSize {
        unsigned dim = 0u;
        unsigned *size = nullptr;


        MatrixSize() {}
        MatrixSize(unsigned _s) : dim(1u), size(new unsigned[1] {_s}) {}
        MatrixSize(const std::initializer_list<unsigned> &_size) : dim(_size.size()), size(new unsigned[_size.size()]) {
            memcpy(size, _size.begin(), dim * sizeof(unsigned));
        }
        MatrixSize(const MatrixSize &_size) : dim(_size.dim), size(new unsigned[_size.dim]) {
            memcpy(size, _size.size, dim);
        }
        ~MatrixSize() {
            if (size != nullptr) delete[] size;
        }


        unsigned &operator[](unsigned ind) { return size[ind]; }
        unsigned  operator[](unsigned ind) const { return size[ind]; }

        const MatrixSize &operator=(unsigned _s) {
            if (dim != 1u) {
                dim = 1u;
                if (size != nullptr) delete[] size;
                size = new unsigned[1];
            }
            *size = _s;
            return *this;
        }

        const MatrixSize &operator=(const std::initializer_list<unsigned> &_size) {
            if (dim != _size.size()) {
                dim = _size.size();
                if (size != nullptr) delete[] size;
                size = new unsigned[_size.size()];
            }
            memcpy(size, _size.begin(), dim);
            return *this;
        }

        const MatrixSize &operator=(const MatrixSize &_size) {
            if (dim != _size.dim) {
                dim = _size.dim;
                if (size != nullptr) delete[] size;
                size = new unsigned[_size.dim];
            }
            for (unsigned i = 0u; i < dim; ++i)
                size[i] = _size.size[i];
            return *this;
        }

        bool operator==(const MatrixSize &_size) {
            if (dim == _size.dim) {
                for (unsigned i = 0u; i < dim; ++i)
                    if (size[i] != _size.size[i]) return false;
                return true;
            }
            return false;
        }

        unsigned long elemsize() const {
            unsigned long tmp = 0u;
            if (dim > 0u) {
                tmp = size[0];
                for (unsigned i = 1u; i < dim; ++i) tmp *= size[i];
            }
            return tmp;
        }

        bool reshape(unsigned _s) {
            if (this->elemsize() == _s) {
                *this = _s;
                return true;
            }
            return false;
        }

        bool reshape(const std::initializer_list<unsigned> &_size) {
            unsigned long s = 1ul;
            for (auto elem : _size) s *= elem;

            if (this->elemsize() == s) {
                *this = _size;
                return true;
            }
            return false;
        }

        bool reshape(const MatrixSize &_size) {
            if (this->elemsize() == _size.elemsize()) {
                *this = _size;
                return true;
            }
            return false;
        }
    }; // MatrixSize

    //TODO: mInd() factory function
    struct MatrixIndex {
        long index, end, step;

        MatrixIndex(Mat::Placeholder) : index(0), end(0), step(1) {}
        MatrixIndex(long _ind, Mat::Placeholder) : index(_ind), end(0), step(_ind >= 0 ? 1 : -1) {}
        MatrixIndex(long _ind, Mat::Placeholder, long _step) : index(_ind), end(0), step(_step) {}
        MatrixIndex(long _ind) : index(_ind), end(_ind + (_ind >= 0 ? 1 : -1)), step(1) {}
        MatrixIndex(long _ind, long _end) : index(_ind), end(_end), step(1) {}
        MatrixIndex(long _ind, long _end, long _step) : index(_ind), end(_end), step(_step) {}

        // returns {index, end, step}
        vec3l calcParams(unsigned _dimsize) const {
            long _end = end ? (end > 0 ? end : (long)_dimsize + end) : (index >= 0 ? (long)_dimsize : -1l);
            long _index = index >= 0 ? index : (long)_dimsize + index;
            return {_index, _end, (_index <= _end) == (step >= 0) ? step : -step};
        }

        unsigned len(unsigned _dimsize) const {
            vec3l res{calcParams(_dimsize)};

            if (res.x >= _dimsize || res.x < 0 || res.y > (long)_dimsize || res.y < -1)
                throw std::range_error("Error: matrix index outside of range");
            return (res.y - res.x) / res.z;
        }
    }; // MatrixIndex END

    struct MatrixIterNDim {
        unsigned long index = 0ul, first = 0ul;
        unsigned dim = 0u;
        vec3l *left_stepsize_len = nullptr;

        MatrixIterNDim() {}
        MatrixIterNDim(const MatrixIterNDim &_rng) : index(_rng.index), first{_rng.first}, dim(_rng.dim), left_stepsize_len(new vec3l[_rng.dim]) {
            memcpy(left_stepsize_len, _rng.left_stepsize_len, dim * sizeof(vec3l));
        }
        MatrixIterNDim(const MatrixSize &_s, const std::initializer_list<MatrixIndex> &_i, unsigned long _beg = 0ul) : first(_beg), dim(_i.size()) {
            if (_s.dim != dim) throw std::runtime_error("Error: MatrixIndex and Matrix dimensions don't match");

            left_stepsize_len = new vec3l[dim];
            unsigned long tmpstep = 1u;
            for (unsigned i = 0; i < dim; ++i) {
                unsigned _size = _s.size[dim - 1 - i];
                vec3l tmp{(_i.end() - 1 - i)->calcParams(_size)};
                first += tmpstep * tmp.x;
                left_stepsize_len[i].y = tmpstep * tmp.z;
                left_stepsize_len[i]["xz"] = (_i.end() - 1 - i)->len(_size);
                tmpstep *= _size;
            }
            index = first;
        }
        ~MatrixIterNDim() { if (left_stepsize_len != nullptr) delete[] left_stepsize_len; }

        void restart() {
            index = first;
            for (unsigned i = 0u; i < dim; ++i) {
                left_stepsize_len[i].x = left_stepsize_len[i].z;
            }
        }

        MatrixSize newSize() {
            long i = -1l;
            while (++i < dim && left_stepsize_len[i].z == 1);
            if (i == -1l) return;

            i -= i == (long)dim;
            MatrixSize _size(dim - i);
            for (unsigned k = 0u; i < dim; ++k)
                _size.size[k] = left_stepsize_len[i++].z;
            
            return _size;
        }

        MatrixIterNDim& operator++() {
            for (unsigned d = 0u; d < dim; ++d) {
                if (!--left_stepsize_len[d].x) {
                    left_stepsize_len[d].x = d < (dim - 1) ? left_stepsize_len[d].z : 0;
                    index -= left_stepsize_len[d].y * (left_stepsize_len[d].z - 1);
                }
                else {
                    index += left_stepsize_len[d].y;
                    break;
                }
            }
            return *this;
        }
        //RangeNDim operator++(int) { RangeNDim retval{*this}; this->operator++(); return retval; }
        //bool operator==(const RangeNDim &other) const { return !left_stepsize_len[dim - 1].x; }

        //TODO
        MatrixIterNDim operator<<(const std::initializer_list<MatrixIndex> &_i) {
            MatrixSize _newsize{newSize()};
            if (_i.size() != _newsize.dim) throw std::runtime_error("Error: new MatrixIndex and matrix view dimensions don't match");

            for (unsigned i = 0u; i < _newsize.dim; ++i) {
                vec3l _tmp{(_i.begin() + i)->calcParams(_newsize[i])};
                
            }
        }

        bool operator!=(const MatrixIterNDim &other) const { return  left_stepsize_len[dim - 1].x; }
        unsigned long operator*() const { return index; }
        MatrixIterNDim begin() { restart(); return *this; }
        MatrixIterNDim end() { return {}; }
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