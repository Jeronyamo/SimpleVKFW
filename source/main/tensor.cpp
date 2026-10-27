#include "main/tensor.h"

namespace Simple {

    TSlice::TSlice(long _fin) : curr{_fin}, beg{_fin}, fin{_fin}, step{0} {}

    TSlice::TSlice(long _beg, long _fin) : curr{_beg}, beg{_beg}, fin{_fin}, step{_beg <= _fin ? 1 : -1} {}

    TSlice::TSlice(long _beg, long _fin, long _step) : curr{_beg}, beg{_beg}, fin{_fin}, step{_step} {}

    TSlice::TSlice(long _beg, long _fin, long _step, uint32_t _dsize, uint32_t _Mul, uint32_t _Sum) :
                curr{_beg}, beg{_beg}, fin{_fin}, step{_step} {  *this = evaluate(_dsize, _Mul, _Sum);  }

    TSlice::TSlice(const TSlice &_ind) : curr{_ind.curr}, beg{_ind.beg}, fin{_ind.fin}, step{_ind.step} {}

    TSlice::TSlice(const TSlice &_ind, long _curr) : curr{_curr}, beg{_ind.beg}, fin{_ind.fin}, step{_ind.step} {}

    bool    TSlice::operator!=(const TSlice &_ind)  { return curr != _ind.fin;  }

    long&   TSlice::operator*()       { return curr; }

    long    TSlice::operator*() const { return curr; }

    long    TSlice::operator[](uint32_t _ind) const  {
        if (_ind >= this->dimSize()) throw std::invalid_argument("Error: TSlice - single index out of range");
        return beg + _ind * step;
    }

    TSlice& TSlice::operator++() {
        curr += step;
        if (step == 0) ++curr;
        return *this;
    }

    TSlice TSlice::begin() const { return { *this, beg }; }

    TSlice   TSlice::end() const { return { fin }; }

    void  TSlice::reset() { curr = beg; }

    void TSlice::finish() { curr = fin; }

    long TSlice::getElemByInd(long _ind) const { return beg + _ind * step; }

    uint32_t TSlice::dimSize()  const { return step ? (fin  - beg) / step : 1; }

    uint32_t TSlice::getIndex() const { return step ? (curr - beg) / step : 0; }

    long TSlice::getElem()  const { return curr; }

    TSlice TSlice::evaluate(uint32_t _dsize) const {
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

    TSlice TSlice::evaluate(uint32_t _dsize, uint32_t _lowDimMul, uint32_t _lowDimSum) const {
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

    TSlice TSlice::narrowDown(TSlice _ind) const {
        const uint32_t _size = dimSize();
        if (_ind.step == 0) {
            long __tmp = _ind.beg >= 0 ? _ind.beg : _size + _ind.beg;
            if (__tmp < 0 || __tmp >= _size) throw std::invalid_argument("Error: TSlice - single index out of range");
            return { this->getElemByInd(__tmp), this->getElemByInd(__tmp), 0 };
        }
        _ind = _ind.evaluate(_size);
        return { beg + _ind.beg * step, beg + _ind.fin * step, _ind.step * step };
    }

    TDim::TDim() {}

    TDim::TDim(const TDim &_dim) : mdims{_dim.mdims}, offset{_dim.offset} {}

    TDim::TDim(const std::vector <TSlice> &_ind, uint32_t _offs) : mdims{_ind}, offset{_offs} {}

    TDim::TDim(const std::vector <TSlice> &_ind, const TSlice &_first, uint32_t _offs) : offset{_offs} {
        mdims.reserve(_ind.size() + 1);
        mdims.push_back(_first);
        mdims.insert(mdims.end(), _ind.begin(), _ind.end());
    }

    TDim TDim::operator[](const TSlice &_ind) const {
        if (!mdims.size()) throw std::runtime_error("Error: too many indices for matrix");
        if (_ind.step == 0) {
            if (_ind.curr >= mdims.back().dimSize()) throw std::runtime_error("Error: index out of range");
            return { { mdims.begin() + 1, mdims.end() }, uint32_t(mdims.front()[_ind.curr] + offset) };
        }
        return { { mdims.begin() + 1, mdims.end() }, mdims.front().narrowDown(_ind), offset };
    }

    TDim TDim::operator[](const std::initializer_list<TSlice> &_ind) const {
        if (mdims.size() < _ind.size()) throw std::runtime_error("Error: too many indices for matrix");
        TDim __res{ mdims, offset };

        auto _elem = __res.mdims.begin();
        for (auto _narr = _ind.begin(); _narr != _ind.end(); ++_narr, ++_elem) {
            *_elem = _elem->narrowDown(*_narr);
        }
        return __res;
    }

    uint32_t TDim::dims()  const { return mdims.size();  }

    bool TDim::empty() const { return mdims.empty(); }

    std::vector<uint32_t> TDim::getIndex() const {
        std::vector<uint32_t> __res;
        __res.resize(dims());

        for (uint32_t i = 0; i < dims(); ++i) {
            __res[i] = mdims[i].getIndex();
        }

        return __res;
    }

    std::vector<long> TDim::getElem(uint32_t *_offs) const {
        std::vector<long> __res;
        __res.resize(dims());

        for (uint32_t i = 0; i < dims(); ++i) {
            __res[i] = mdims[i].getElem();
        }
        if (_offs != nullptr) *_offs = offset;
        return __res;
    }
}; // Simple END
