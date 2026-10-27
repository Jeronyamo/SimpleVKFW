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


        TSlice(long _fin);
        TSlice(long _beg, long _fin);
        TSlice(long _beg, long _fin, long _step);
        TSlice(long _beg, long _fin, long _step, uint32_t _dsize, uint32_t _Mul = 1u, uint32_t _Sum = 0u);
        TSlice(const TSlice &_ind);
        TSlice(const TSlice &_ind, long _curr);


        bool    operator!=(const TSlice &_ind);
        long&   operator*();
        long    operator*() const;
        long    operator[](uint32_t _ind) const;
        TSlice& operator++();


        TSlice begin() const;
        TSlice   end() const;

        void  reset();
        void finish();
        long getElemByInd(long _ind) const;
        uint32_t dimSize()  const;
        uint32_t getIndex() const;
        long getElem()  const;

        TSlice evaluate(uint32_t _dsize = 0) const;
// using offset and multiplier, delete if not used
        TSlice evaluate(uint32_t _dsize, uint32_t _lowDimMul, uint32_t _lowDimSum) const;
// evaluate() should be used before narrowDown()
        TSlice narrowDown(TSlice _ind) const;
    };


// Stores slices (by default each is [0, DimSize))
    struct TDim {
        std::vector <TSlice> mdims;
        uint32_t offset = 0u;

        TDim();
        TDim(const TDim &_dim);
        TDim(const std::vector <TSlice> &_ind, uint32_t _offs = 0u);
        TDim(const std::vector <TSlice> &_ind, const TSlice &_first, uint32_t _offs = 0u);

        TDim operator[](const TSlice &_ind) const;

        TDim operator[](const std::initializer_list<TSlice> &_ind) const;

        uint32_t dims()  const;
        bool empty() const;

        std::vector<uint32_t> getIndex() const;

        std::vector<long> getElem(uint32_t *_offs = nullptr) const;
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