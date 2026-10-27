#ifndef SVKFW_TENSOR_H
#define SVKFW_TENSOR_H

#include "common/utilities.h"
#include "common/basic_math.h"

#include <vector>
#include <memory>
#include <iostream> // delete after debugging


namespace Simple {
    const long SliceBorderL = LONG_MIN, SliceBorderR = LONG_MAX;

    struct TSlice {
        long curr = 0, beg = 0, fin, step = 1;


        TSlice(long _fin) : curr{_fin}, beg{_fin}, fin{_fin}, step{0} {}
        TSlice(long _beg, long _fin) : curr{_beg}, beg{_beg}, fin{_fin}, step{_beg <= _fin ? 1 : -1} {}
        TSlice(long _beg, long _fin, long _step) : curr{_beg}, beg{_beg}, fin{_fin}, step{_step} {}
        TSlice(long _beg, long _fin, long _step, uint32_t _dsize, uint32_t _Mul = 1u, uint32_t _Sum = 0u) :
                    curr{_beg}, beg{_beg}, fin{_fin}, step{_step} {  *this = evaluate(_dsize, _Mul, _Sum);  }
        TSlice(const TSlice &_ind) : curr{_ind.curr}, beg{_ind.beg}, fin{_ind.fin}, step{_ind.step} {}
        TSlice(const TSlice &_ind, long _curr) : curr{_curr}, beg{_ind.beg}, fin{_ind.fin}, step{_ind.step} {}


        bool    operator!=(const TSlice &_ind)  { return curr != _ind.fin;  }
        long&   operator*()       { return curr; }
        long    operator*() const { return curr; }
        long    operator[](uint32_t _ind) const  {
            if (_ind >= this->dimSize()) throw std::invalid_argument("Error: TSlice - single index out of range");
            return beg + _ind * step;
        }
        TSlice& operator++() {
            curr += step;
            if (step == 0) ++curr;
            return *this;
        }


        TSlice begin() const { return { *this, beg }; }
        TSlice   end() const { return { fin }; }

        void  reset() { curr = beg; }
        void finish() { curr = fin; }
        long getElemByInd(long _ind) const { return beg + _ind * step; }
        uint32_t dimSize()  const { return step ? (fin  - beg) / step : 1; }
        uint32_t getIndex() const { return step ? (curr - beg) / step : 0; }
        long getElem()  const { return curr; }

        TSlice evaluate(uint32_t _dsize = 0) const {
            long _b = beg >= 0 ? beg : _dsize + beg,
                 _e = fin >= 0 ? fin : _dsize + fin,
                 _s = step;

            if (_dsize) {
                _b = Math::clampRO(_b,  0l, (long)_dsize);
                _e = Math::clampCL(_e, -1l, (long)_dsize);
            }

            if (_s == 0) {
                if (_b != _e) throw std::runtime_error("Error: TSlice - step is 0. To set a scalar index, make sure beg == fin"); // redo
                _e += 1;
            }
            else if (long t = (_e - _b) % _s) _e += _s - t;
            if ((_b <= _e) != (_s >= 0)) _b = _e;
            return { _b, _e, _s };
        }
// using offset and multiplier, delete if not used
        TSlice evaluate(uint32_t _dsize, uint32_t _lowDimMul, uint32_t _lowDimSum) const {
            long _b = beg >= 0 ? beg : _dsize + beg,
                 _e = fin >= 0 ? fin : _dsize + fin,
                 _s = step;

            if (_dsize) {
                // Commented strict mode: limits are either SliceBorder or in the correct range, error otherwise
                //if (fin == SliceBorderL) _e = -1;
                //if (fin == SliceBorderR) _e = _dsize;
                //if (!Func::is_between_GE_L (_b,  0l, (long)_dsize)) throw std::runtime_error("Error: TSlice - init value (beg) out of range");
                //if (!Func::is_between_GE_LE(_e, -1l, (long)_dsize)) throw std::runtime_error("Error: TSlice - last value (end) out of range");
                _b = Math::clampRO(_b,  0l, (long)_dsize);
                _e = Math::clampCL(_e, -1l, (long)_dsize);
            }

            if (_s == 0) {
                if (_b != _e) throw std::runtime_error("Error: TSlice - step is 0. To set a scalar, make sure beg == fin"); // move sentence 2 from here
                _e += 1;
            }
            else if (long t = (_e - _b) % _s) _e += _s - t;
            if ((_b <= _e) != (_s >= 0)) {
                _b = _e;
                // for debugging, delete later
                throw std::runtime_error("Error: TSlice - step sign does not match the shortest way from beg to fin");
            }
            return { _b * (long)_lowDimMul + (long)_lowDimSum, _e * (long)_lowDimMul + (long)_lowDimSum, _s * (long)_lowDimMul };
        }
// evaluate() should be used before narrowDown()
        TSlice narrowDown(TSlice _ind) const {
            const uint32_t _size = dimSize();
            if (_ind.step == 0) {
                long __tmp = _ind.beg >= 0 ? _ind.beg : _size + _ind.beg;
                if (__tmp < 0 || __tmp >= _size) throw std::invalid_argument("Error: TSlice - single index out of range");
                return { this->getElemByInd(__tmp), this->getElemByInd(__tmp), 0 };
            }
            _ind = _ind.evaluate(_size);
            return { beg + _ind.beg * step, beg + _ind.fin * step, _ind.step * step };
        }
    };


// Stores slices (by default each is [0, DimSize))
    struct TDim {
        std::vector <TSlice> mdims;
        uint32_t offset = 0u;

        TDim() {}
        TDim(const TDim &_dim) : mdims{_dim.mdims}, offset{_dim.offset} {}
        TDim(const std::vector <TSlice> &_ind, uint32_t _offs = 0u) : mdims{_ind}, offset{_offs} {}
        TDim(const std::vector <TSlice> &_ind, const TSlice &_first, uint32_t _offs = 0u) : offset{_offs} {
            mdims.reserve(_ind.size() + 1);
            mdims.push_back(_first);
            mdims.insert(mdims.end(), _ind.begin(), _ind.end());
        }

        TDim operator[](const TSlice &_ind) const {
            if (!mdims.size()) throw std::runtime_error("Error: too many indices for matrix");
            if (_ind.step == 0) {
                if (_ind.curr >= mdims.back().dimSize()) throw std::runtime_error("Error: index out of range");
                return { { mdims.begin() + 1, mdims.end() }, mdims.front()[_ind.curr] + offset };
            }
            return { { mdims.begin() + 1, mdims.end() }, mdims.front().narrowDown(_ind), offset };
        }

        TDim operator[](const std::initializer_list<TSlice> &_ind) const {
            if (mdims.size() < _ind.size()) throw std::runtime_error("Error: too many indices for matrix");
            TDim __res{ mdims, offset };

            auto _elem = __res.mdims.begin();
            for (auto _narr = _ind.begin(); _narr != _ind.end(); ++_narr, ++_elem) {
                *_elem = _elem->narrowDown(*_narr);
            }
            return __res;
        }

        uint32_t dims()  const { return mdims.size();  }
        bool empty() const { return mdims.empty(); }

        std::vector<uint32_t> getIndex() const {
            std::vector<uint32_t> __res;
            __res.resize(dims());

            for (uint32_t i = 0; i < dims(); ++i) {
                __res[i] = mdims[i].getIndex();
            }

            return __res;
        }

        std::vector<long> getElem(uint32_t *_offs = nullptr) const {
            std::vector<long> __res;
            __res.resize(dims());

            for (uint32_t i = 0; i < dims(); ++i) {
                __res[i] = mdims[i].getElem();
            }
            if (_offs != nullptr) *_offs = offset;
            return __res;
        }
    };

    template <typename T>
    struct Tensor {
        std::shared_ptr<T> data = nullptr;
        std::vector<uint32_t> shape;
        TDim view;
        // [reshape]+, [view]+, reshape

        Tensor() {}
        Tensor(uint32_t _num_elements) {
            if (_num_elements > 0) {
                data = std::shared_ptr<T>(new T[_num_elements], std::default_delete<T[]>()); // Note: array deleter
                view.mdims.push_back({0l, (long)_num_elements});
            }
        }

        const Tensor<T>& operator=(const Tensor<T> &_t) {
            
            return *this;
        }

        // TODO:
        Tensor<T> deepcopy(bool _flatten = false) {
            return {};
        }

        template <typename Y>
        Tensor<Y> castcopy(bool _flatten = false) {
            return {}; // TODO:
        }


        void reshape(const std::vector<uint32_t> &_sh) {

        }
    };
};

#endif