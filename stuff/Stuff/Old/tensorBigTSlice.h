#ifndef SOGLFW_MULTIDMAT_H
#define SOGLFW_MULTIDMAT_H

#include "utilities.h"

#include <vector>
#include <iostream> // delete after debugging


namespace Simple {
    const long SliceBorderL = LONG_MIN, SliceBorderR = LONG_MAX;

    struct TSlice {
        long curr = 0, beg = 0, fin, step = 1;

        template <bool _scalar = true>
        TSlice(long _end, BoolFlag<_scalar> _s = True) :
                    beg{_scalar ? _end : 0}, fin{_end + (long)_scalar}, step{!_scalar} {}

        TSlice(long _beg, long _end, BoolCl _validate = FALSE, BoolCl _reinterpret = FALSE) :
                    curr{_beg}, beg{_beg}, fin{_end}, step{_beg <= _end ? 1 : -1} {
            if (_validate.value) *this = evaluate(0, 1, 0, _reinterpret.value);
        }
        TSlice(long _beg, long _end, long _step, BoolCl _validate = FALSE, BoolCl _reinterpret = FALSE) :
                    curr{_beg}, beg{_beg}, fin{_end}, step{_step} {
            if (_validate.value) *this = evaluate(0, 1, 0, _reinterpret.value);
        }
        TSlice(long _beg, long _end, long _step, unsigned _dsize, unsigned _Mul = 1u, unsigned _Sum = 0u) :
                    curr{_beg}, beg{_beg}, fin{_end}, step{_step} {
            *this = evaluate(_dsize, _Mul, _Sum);
        }
        TSlice(const TSlice &_ind, long _curr = 0, bool _currCopy = false) :
                    curr{_currCopy ? _curr : _ind.curr}, beg{_ind.beg}, fin{_ind.fin}, step{_ind.step} {}


        bool      operator!=(const TSlice &_ind)  { return curr != _ind.fin;  }
        long      operator[](unsigned _ind) const  { return beg + _ind * step; }
        long&     operator*()       { return curr; }
        long      operator*() const { return curr; }
        TSlice& operator++() {
            curr += step;
            return *this;
        }

        TSlice begin() const { return { *this, beg,  true }; }
        TSlice   end() const { return {        fin, False }; }

        void  reset() { curr = beg; }
        void finish() { curr = fin; }
        unsigned size() const { return (fin - beg) / step; }
        TSlice evaluate(unsigned _dsize = 0, unsigned lowDimMul = 1, unsigned lowDimSum = 0, bool _reinterpret = false) const {
            long _b = beg >= 0 ? beg : _dsize + beg,
                 _e = fin >= 0 ? fin : _dsize + fin,
                 _s = step;

            if (_reinterpret || _dsize) {
                if (fin == SliceBorderL) _e = -1;
                if (beg < 0 || (fin < 0 && fin != SliceBorderL) || fin == SliceBorderR) {
                    if (fin == SliceBorderR) _e = _dsize;
                    if (!_dsize) throw std::invalid_argument("Error: MDSlice - validation required, but dsize is not set/set to 0");
                    if (_b <  0 || _b >= (long)_dsize)   throw std::runtime_error("Error: MDSlice - init value (beg) out of range");
                    if (_e < -1 || _e >  (long)_dsize)   throw std::runtime_error("Error: MDSlice - last value (end) out of range");
                }
            }

            if (_s == 0) throw std::runtime_error("Error: MDSlice - step is 0");
            if ((_b <= _e) != (_s > 0)) _s = -_s;
            if (long t = (_e - _b) % _s) _e += _s - t;
            return { _b * (long)lowDimMul + (long)lowDimSum, _e * (long)lowDimMul + (long)lowDimSum, _s * (long)lowDimMul };
        }

        TSlice narrowDown(TSlice _ind) const {
            const unsigned _size = size();
            if (_ind.step == 0) {
                long _tmp = _ind.beg >= 0 ? _ind.beg : _size + _ind.beg;
                if (_tmp < 0 || _tmp >= _size) throw std::invalid_argument("Error: MDSlice - single index out of range");
                return { *this[_tmp] };
            }
            _ind = _ind.evaluate(_size, 1, 0);
            return { beg + _ind.beg * step, beg + _ind.fin * step, _ind.step * step };
        }
    };


//REDO 'cause TSlice also was redone
    struct TDim {
        std::vector <unsigned> shape;
        std::vector <TSlice> msize;
        unsigned offset = 0u;

        TDim(const std::vector <unsigned> &_sh, const std::vector <TSlice> &_ind, unsigned _offs = 0u) :
                    shape{_sh}, msize{_ind}, offset{_offs} {
            if (shape.size() != msize.size()) throw std::invalid_argument("Error: MDMatDim constructor - vector sizes are not equal");
            for (unsigned i = 0u; i < _sh.size(); ++i) {
                if (shape[i] == 0) throw std::invalid_argument("Error: MDMatDim constructor - shape has dimension of size 0");
                msize[i].evaluate(shape[i]);
            }
        }
        TDim(const std::vector <unsigned> &_sh, const std::vector <TSlice> &_ind, const TSlice &_last, unsigned _offs) :
                    shape{_sh}, msize{_ind}, offset{_offs} {
            msize.push_back(_last);
        }

        TDim operator[](unsigned _ind) {
            if (!msize.size()) throw std::runtime_error("Error: too many indices for matrix"); // must rename most of these errors
            if (_ind >= msize.back().beg) throw std::runtime_error("Error: index out of range");
            return { { shape.begin(), --shape.end() }, { msize.begin(), --msize.end() }, msize.back()[_ind] + offset };
        }

        TDim operator[](const TSlice &_ind) {
            if (!msize.size()) throw std::runtime_error("Error: too many indices for matrix");
            return { shape, { msize.begin(), --msize.end() }, msize.back().narrowDown(_ind), offset };
        }

        TDim operator[](const std::initializer_list<TSlice> &_ind) {
            if (msize.size() < _ind.size()) throw std::runtime_error("Error: too many indices for matrix");
            TDim _tmp{ shape, msize, offset };

            auto _elem = _tmp.msize.rbegin();
            for (auto _narr = _ind.begin(); _narr != _ind.end(); ++_narr, ++_elem) {
                *_elem = _elem->narrowDown(*_narr);
            }
            return _tmp;
        }
    };

    template <typename T>
    struct Tensor {
        T *data = nullptr;
        TDim  matsize;
    };
};

#endif