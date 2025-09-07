#ifndef SVKFW_TIMER_H
#define SVKFW_TIMER_H

#include <cstdio>
#include <cstdint>
#include <chrono>

#include "common/basic_math.h"
#include "common/terminal.h"


namespace Simple {
//  ============  Time classes (for convenience)  ============  \\

    struct TimeHMS {
        int64_t h;
        uint8_t m, s;

        TimeHMS(int64_t _h, uint8_t _m, uint8_t _s) : h{_h}, m{_m}, s{_s} {}
        TimeHMS(const TimeHMS &_t) : h{_t.h}, m{_t.m}, s{_t.s} {}
        TimeHMS(double _t) {
            double __s = 0.;
            Math::modf(_t, &__s);
            h = (int64_t) __s;
            s = h % 60;
            h /= 60;
            m = h % 60;
            h /= 60;
        }
    };

    struct TimeHMSf {
        int64_t h;
        uint16_t m, s;
        float fract;

        TimeHMSf(int64_t _h, uint16_t _m, uint16_t _s, float _f = 0.f) : h{_h}, m{_m}, s{_s}, fract{_f} {}
        TimeHMSf(const TimeHMSf &_t) : h{_t.h}, m{_t.m}, s{_t.s}, fract{_t.fract} {}
        TimeHMSf(double _t) {
            double __s = 0.;
            fract = (float) Math::modf(_t, &__s);
            h = (int64_t) __s;
            s = h % 60;
            h /= 60;
            m = h % 60;
            h /= 60;
        }

        uint32_t ms() { return 1000000    * fract; }
        uint32_t ns() { return 1000000000 * fract; }
        uint64_t fraction(uint64_t _coef) { return uint64_t(_coef * fract); }
    };


//  ============  Timer classes  ============  \\

    template<typename TClock, typename TResult>
    struct ClockTick {
        typename TClock::time_point time_pt;

        ClockTick() : time_pt{TClock::now()} {}
        ClockTick(const ClockTick &_t_diff) = default;

        TResult tick() {
            auto __new_time_pt = TClock::now();
            TResult __res = std::chrono::duration<TResult, std::chrono::seconds::period>(__new_time_pt - time_pt).count();
            time_pt = __new_time_pt;
            return __res;
        }
    }; // ClockTick END


    class Clock {
        using TClock = std::chrono::high_resolution_clock;
        TClock::time_point clock_start;
public:

        Clock() : clock_start{TClock::now()} {}
        Clock(const Clock &_t) = default;

        double get() const { return std::chrono::duration<double, std::chrono::seconds::period>(TClock::now() - clock_start).count(); }
        void reset() { clock_start = TClock::now(); }
        float diff(double _sub) const { return get() - _sub; }
        // TimeHMS  toTimeHMS ()   const { return get(); }
        // TimeHMSf toTimeHMSf()   const { return get(); }
    }; // Clock END


    class Timer {
        using TClock = std::chrono::high_resolution_clock;
        double time;
        ClockTick<TClock, double> clock_tick;
public:

        Timer(const Timer &_t) = default;
        Timer(double _time) : time{Math::abs(_time)} {}


        double update() { time -= clock_tick.tick(); return time; }
        double lastTime() const { return time; }
        float diff(double _sub) const { return time - _sub; }
        // TimeHMS  toTimeHMS ()   const { return time; }
        // TimeHMSf toTimeHMSf()   const { return time; }

        void set(double _new_time) {
            if (_new_time <= 0.)
                fprintf(svkfwwarn, SVKFW_WRAPWARN("Timer :: set", "negative time, using absolute value"));
            time = Math::abs(_new_time);
            clock_tick.tick();
        }
    }; // Timer END


    class TimerEven {
        using TClock = std::chrono::high_resolution_clock;
        double time;
        ClockTick<TClock, double> clock_tick;
public:
        static const double Start, End;

        TimerEven(const TimerEven &_t) = default;
        TimerEven(double _time) : time{_time + Start} {
            if (time <= Start || time > End)
                throw std::invalid_argument(SVKFW_WRAPERR("TimerEven constructor", "time value must lie in (0, 2^32]"));
        }


        double update() { time -= clock_tick.tick(); return time - Start; }
        double lastTime() const { return time - Start; }
        float diff(double _sub) const { return lastTime() - _sub; }
        // TimeHMS  toTimeHMS ()   const { return lastTime(); }
        // TimeHMSf toTimeHMSf()   const { return lastTime(); }

        void set(double _new_time) {
            if (_new_time <= 0. || _new_time > Start)
                throw std::invalid_argument(SVKFW_WRAPERR("TimerEven :: set", "time value must lie in (0, 2^32]"));
            time = _new_time + Start;
            clock_tick.tick();
        }
    }; // TimerEven END
    const double TimerEven::Start = 4294967296., TimerEven::End = 8589934592.;


// These might work, but there's nowhere to use them

// // Int timer
// // Use this class as time difference for TimerUint
//     struct TimerInt {
//         static const int64_t FRAC_IN_SEC;
//         int32_t seconds = 0l;
//         int64_t fraction = 0;


//         TimerInt() {}
//         TimerInt(const TimerEven &_t) {
//             double __s = 0.;
//             fraction = FRAC_IN_SEC * Math::modf(_t.get(), &__s);
//             seconds  = (int32_t) __s;
//         }
//         TimerInt(const TimerInt &_tu) : seconds{_tu.seconds}, fraction{_tu.fraction} {}
//         TimerInt(int32_t _s, int64_t _frac) : seconds{_s}, fraction{_frac} {}


//         bool evalFrac() {
//             if (fraction >= FRAC_IN_SEC || fraction <= -FRAC_IN_SEC) {
//                 int32_t __secs = fraction / FRAC_IN_SEC;
//                 int32_t __was_sec = seconds;

//                 seconds += __secs;
//                 if ((fraction >=  FRAC_IN_SEC && seconds < __was_sec) ||
//                     (fraction <= -FRAC_IN_SEC && seconds > __was_sec)) {
//                     // after    addition o/flow - subtracting minimal (neg) value, so seconds >= 0
//                     // after subtraction o/flow - subtracting maximal (pos) value, so seconds <= 0
//                     seconds -= int32_t(1ul << (8 * sizeof(int32_t) - 1)) - int32_t(fraction >= FRAC_IN_SEC);
//                     fraction %= FRAC_IN_SEC;
//                     return true;
//                 }
//             }
//             return false;
//         }

//         bool addSec(int32_t _sec) {
//             int32_t __new_sec = seconds + _sec;
//             bool __res = (_sec > 0 && __new_sec < seconds) ||
//                          (_sec < 0 && __new_sec > seconds);
//             if (__res)
//                 __new_sec -= int32_t(1ul << (8 * sizeof(int32_t) - 1)) - int(_sec > 0);
//             seconds = __new_sec;
//             return __res;
//         }

//         bool addFrac(int64_t _frac, bool _tosec_on_oflow = true) {
//             int64_t __new_frac = fraction + _frac;
//             bool __res = (_frac > 0 && __new_frac < fraction) ||
//                          (_frac < 0 && __new_frac > fraction);
//             if (__res) {
//                 if (_tosec_on_oflow) {
//                     int32_t __sec = (fraction / FRAC_IN_SEC) + (_frac / FRAC_IN_SEC);
//                     __new_frac    = (fraction % FRAC_IN_SEC) + (_frac % FRAC_IN_SEC);
//                     __res = addSec(__sec);
//                 }
//                 else
//                     __new_frac -= int64_t(1ul << (8 * sizeof(int64_t) - 1)) - int(_frac > 0);
//             }
//             fraction = __new_frac;
//             return __res;
//         }

//         bool add(int32_t _s, int64_t _frac, bool _eval = true) {
//             bool __res  = addSec(_s);
//             bool __res2 = addFrac(_frac);
//             bool __res3 = false;
//             if (_eval) __res3 = evalFrac();
//             return __res || __res2 || __res3;
//         }

//         bool set(int32_t _s, int64_t _frac, bool _eval = true) {
//             seconds  = _s;
//             fraction = _frac;
//             bool __res = false;
//             if (_eval) __res = evalFrac();
//             return __res;
//         }

//         TimerInt diff(int32_t _s, int64_t _frac) {
//             return { seconds - _s, fraction - _frac };
//         }
//     };
//     const int64_t TimerInt::FRAC_IN_SEC = 1e6;


// // Uint timer
//     struct TimerUint {
//         static const uint64_t FRAC_IN_SEC;
//         uint32_t seconds = 0ul;
//         uint64_t fraction = 0u;


//         TimerUint() {}
//         TimerUint(const Timer &_t) {
//             double __s = 0.;
//             fraction = FRAC_IN_SEC * Math::modf(_t.get(), &__s);
//             seconds  = (uint32_t) __s;
//         }
//         TimerUint(const TimerUint &_tu) : seconds{_tu.seconds}, fraction{_tu.fraction} {}
//         TimerUint(uint32_t _s, uint64_t _frac) : seconds{_s}, fraction{_frac} {}


//         bool evalFrac() {
//             if (fraction >= FRAC_IN_SEC) {
//                 uint32_t __secs = fraction / FRAC_IN_SEC;
//                 fraction %= FRAC_IN_SEC;
//                 seconds  += __secs;
//                 return seconds < __secs;
//             }
//             return false;
//         }

//         bool addSec(uint32_t _sec) {
//             uint32_t __old_sec = seconds;
//             seconds += _sec;
//             return seconds < __old_sec;
//         }

//         bool addFrac(uint64_t _frac, bool _tosec_on_oflow = true) {
//             uint64_t __new_frac = fraction + _frac;
//             bool __res = __new_frac < fraction;
//             if (__res && _tosec_on_oflow) {
//                 __new_frac =   (fraction % FRAC_IN_SEC) + (_frac % FRAC_IN_SEC);
//                 __res = addSec((fraction / FRAC_IN_SEC) + (_frac / FRAC_IN_SEC));
//             }
//             fraction = __new_frac;
//             return __res;
//         }

//         bool add(uint32_t _s, uint64_t _frac, bool _eval = true) {
//             bool __res  = addSec(_s);
//             bool __res2 = addFrac(_frac);
//             bool __res3 = false;
//             if (_eval) __res3 = evalFrac();
//             return __res || __res2 || __res3;
//         }

//         bool set(uint32_t _s, uint64_t _frac, bool _eval = true) {
//             seconds  = _s;
//             fraction = _frac;
//             bool __res = false;
//             if (_eval) __res = evalFrac();
//             return __res;
//         }

//         TimerInt diff(uint32_t _s, uint64_t _frac) {
//             return { int32_t(seconds - _s), int64_t(fraction - _frac) };
//         }
//     };
//     const uint64_t TimerUint::FRAC_IN_SEC = 1e6;
}; // Simple END

#endif