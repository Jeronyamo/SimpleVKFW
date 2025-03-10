#ifndef SVKFW_BASIC_MATH_H
#define SVKFW_BASIC_MATH_H

#include <cmath>
#include <climits>
#include <stdexcept>
#include <cfloat>


// Absolute epsilon for float check
// such that (1.0 / SVKFW_1EPS_FLT < FLT_MAX) == true
#define SVKFW_AEPS_FLT 1e-38f
// Absolute epsilon for double check
// such that (1.0 / SVKFW_1EPS_DBL < DBL_MAX) == true
#define SVKFW_AEPS_DBL 1e-308
// Absolute epsilon for vec_flt_t (aka float) check
// such that (1.0 / SVKFW_1EPS_FLT < FLT_MAX) == true
#define SVKFW_AEPS SVKFW_AEPS_FLT

#define SVKFW_EPS4 1e-4f
#define SVKFW_EPS5 1e-5f
#define SVKFW_EPS6 1e-6f
#define SVKFW_EPS  SVKFW_EPS6


#define SVKFW_DEG_TO_RAD (M_PI / 180)
#define SVKFW_RAD_TO_DEG (180 / M_PI)


namespace Simple {
    namespace Math {
        using std::abs;
        using std::swap;
        using std::modf;

        using std::pow;
        using std::sqrt;
        using std::cbrt;

        using std::sin;
        using std::cos;
        using std::tan;
        using std::asin;
        using std::acos;
        using std::atan2;

        using std::log;
        using std::exp;


        template <typename T> inline T  sqr(T _val) { return _val * _val; }
        template <typename T> inline T cube(T _val) { return _val * _val * _val; }
        template <typename T> inline T sign(T _val) { return _val < 0 ? -1 : 1; }

        // Zero check using absolute epsilon, use if value must be finite when inversed: (1.0 / _val)
        inline bool notZero (float  _val) { return _val <= -SVKFW_AEPS_FLT || _val >= SVKFW_AEPS_FLT; }
        inline bool notZero (double _val) { return _val <= -SVKFW_AEPS_DBL || _val >= SVKFW_AEPS_DBL; }
        template <class T>
        inline bool notZero (T _val, T _eps = (T) SVKFW_EPS) { return _val <= -_eps || _val >= _eps; }

        // Zero check for _val and _eps > 0
        template <typename T> inline bool closeToZero(T _val) { return _val < SVKFW_EPS && _val > -SVKFW_EPS; }
        template <typename T> inline bool closeToZero(T _val, T _eps) {
            if (_eps <= 0)
                return _val < -_eps && _val > _eps;
            return _val < _eps && _val > -_eps;
        }


        // Check if _val is in the interval [_left, _right] (CLosed)
        template <typename T>
        inline bool is_between_CL(T _val, T _left, T _right) { return _val >= _left && _val <= _right; }
        // Check if _val is in the interval [_left, _right) (Right Open)
        template <typename T>
        inline bool is_between_RO(T _val, T _left, T _right) { return _val >= _left && _val <  _right; }
        // Check if _val is in the interval (_left, _right] (Left Open)
        template <typename T>
        inline bool is_between_LO(T _val, T _left, T _right) { return _val >  _left && _val <= _right; }
        // Check if _val is in the interval (_left, _right) (OPen)
        template <typename T>
        inline bool is_between_OP(T _val, T _left, T _right) { return _val >  _left && _val <  _right; }

        // Clamp _val in the interval [_left, _right] (CLosed)
        template <typename T>
        T clamp_CL(T _val, T _left, T _right)              { return _val <= _left ? _left : (_val >= _right ? _right : _val); }
        // Clamp _val in the interval [_left, _right) (Right Open)
        template <typename T>
        T clamp_RO(T _val, T _left, T _right, T _step = 1) { return _val <= _left ? _left : (_val >= _right ? _right - _step : _val); }
        // Clamp _val in the interval (_left, _right] (Left Open)
        template <typename T>
        T clamp_LO(T _val, T _left, T _right, T _step = 1) { return _val <= _left ? _left + _step : (_val >= _right ? _right : _val); }
        // Clamp _val in the interval (_left, _right) (OPen)
        template <typename T>
        T clamp_OP(T _val, T _left, T _right, T _step = 1) { return _val <= _left ? _left + _step : (_val >= _right ? _right - _step : _val); }


        template <typename T>
        T smoothstep(T _left, T _val, T _right) {
            if (_val <=  _left) return  _left;
            if (_val >= _right) return _right;

            T __res = (_val - _left) / (_right - _left);
            return __res * __res * (3 - 2 * __res);
        }


        template <typename Y, class T>
        Y sum(const T&_iter) {
            Y __sum = 0;
            for (auto i : _iter)
                __sum += i;
            return __sum;
        }

        template <typename Y, class T>
        Y mul(const T&_iter) {
            Y __sum = 1;
            for (auto i : _iter)
                __sum *= i;
            return __sum;
        }


    // Basic angle transforms

        inline float  toRad(float  _angle_deg) { return _angle_deg * SVKFW_DEG_TO_RAD; }
        inline double toRad(double _angle_deg) { return _angle_deg * SVKFW_DEG_TO_RAD; }
        inline float  toDeg(float  _angle_rad) { return _angle_rad * SVKFW_RAD_TO_DEG; }
        inline double toDeg(double _angle_rad) { return _angle_rad * SVKFW_RAD_TO_DEG; }
    }; // Math END

    template <typename T>
    struct Range {
        T from = 0, to, step = 1;
        uint32_t num = 1ul;

        Range(const Range &_r) = default;
        Range(T _to) : to{_to} {
            if (from > _to)
                step = -step;
            calcItersNum();
        }
        Range(T _from, T _to) : from{_from}, to{_to} {
            if (from > _to)
                step = -step;
            calcItersNum();
        }
        Range(T _from, T _to, T _step) : from{_from}, to{_to}, step{_step} {
            calcItersNum();
        }

        Range& operator++() { from += step; --num; return *this; }
        bool operator!=(const Range &other) const { return num; }
        T operator*() const { return from; }

        Range begin() { return *this; }
        Range end() { return to; }


        void calcItersNum() {
            num = (uint32_t) std::floor((to - from) / step);
        }
    }; // Range END
}; // Simple END

#endif