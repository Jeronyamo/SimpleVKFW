#include "main/timer.h"

namespace Simple {

    TimeHMS::TimeHMS(int64_t _h, uint8_t _m, uint8_t _s) : h{_h}, m{_m}, s{_s} {}

    TimeHMS::TimeHMS(const TimeHMS &_t) : h{_t.h}, m{_t.m}, s{_t.s} {}

    TimeHMS::TimeHMS(double _t) {
        double __s = 0.;
        Math::modf(_t, &__s);
        h = (int64_t) __s;
        s = h % 60;
        h /= 60;
        m = h % 60;
        h /= 60;
    }

    TimeHMSf::TimeHMSf(int64_t _h, uint16_t _m, uint16_t _s, float _f) : h{_h}, m{_m}, s{_s}, fract{_f} {}

    TimeHMSf::TimeHMSf(const TimeHMSf &_t) : h{_t.h}, m{_t.m}, s{_t.s}, fract{_t.fract} {}

    TimeHMSf::TimeHMSf(double _t) {
        double __s = 0.;
        fract = (float) Math::modf(_t, &__s);
        h = (int64_t) __s;
        s = h % 60;
        h /= 60;
        m = h % 60;
        h /= 60;
    }

    uint32_t TimeHMSf::ms() { return 1000       * fract; }

    uint32_t TimeHMSf::us() { return 1000000    * fract; }

    uint32_t TimeHMSf::ns() { return 1000000000 * fract; }

    uint64_t TimeHMSf::fraction(uint64_t _coef) { return uint64_t(_coef * fract); }

    Clock::Clock() : clock_start{TClock::now()} {}

    double Clock::get() const { return std::chrono::duration<double, std::chrono::seconds::period>(TClock::now() - clock_start).count(); }

    void Clock::reset() { clock_start = TClock::now(); }

    float Clock::diff(double _sub) const { return get() - _sub; }

    Timer::Timer(double _time) : time{Math::abs(_time)} {}

    double Timer::update() { time -= clock_tick.tick(); return time; }

    double Timer::lastTime() const { return time; }

    float Timer::diff(double _sub) const { return time - _sub; }

    void Timer::set(double _new_time) {
        if (_new_time <= 0.)
            fprintf(svkfwwarn, SVKFW_WRAPWARN("Timer :: set", "negative time, using absolute value"));
        time = Math::abs(_new_time);
        clock_tick.tick();
    }

    TimerEven::TimerEven(double _time) : time{_time + Start} {
        if (time <= Start || time > End)
            throw std::invalid_argument(SVKFW_WRAPERR("TimerEven constructor", "time value must lie in (0, 2^32]"));
    }

    double TimerEven::update() { time -= clock_tick.tick(); return time - Start; }

    double TimerEven::lastTime() const { return time - Start; }

    float TimerEven::diff(double _sub) const { return lastTime() - _sub; }

    void TimerEven::set(double _new_time) {
        if (_new_time <= 0. || _new_time > Start)
            throw std::invalid_argument(SVKFW_WRAPERR("TimerEven :: set", "time value must lie in (0, 2^32]"));
        time = _new_time + Start;
        clock_tick.tick();
    }

    const double TimerEven::Start = 4294967296.;

    const double TimerEven::End   = 8589934592.;
}; // Simple END
