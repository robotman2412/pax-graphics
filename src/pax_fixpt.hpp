
// SPDX-License-Identifier: MIT

#ifndef FIXPT_HPP
#define FIXPT_HPP

#include "pax_config.h"

#if !PAX_USE_FIXED_POINT
typedef float fixpt_t;

static inline constexpr fixpt_t operator""_fix(unsigned long long in) {
    return in;
}
static inline constexpr fixpt_t operator""_fix(long double in) {
    return in;
}
#else

#define PAX_FIXPT_INT_BITS  12
#define PAX_FIXPT_FRAC_BITS 20
#define PAX_FIXPT_MUL       0x00100000
#define FIXPT_MAX           ((long double)INT32_MAX / PAX_FIXPT_MUL)
#define FIXPT_MIN           ((long double)INT32_MIN / PAX_FIXPT_MUL)

class fixpt_t {
  public:
    int32_t raw_value;

  private:
    static constexpr int32_t _div(int32_t a, int32_t b) {
        int64_t tmp = b ? (((int64_t)a << PAX_FIXPT_FRAC_BITS) / (int64_t)b) : (a > 0 ? INT32_MAX : INT32_MIN);
        if (tmp <= INT32_MIN)
            return INT32_MIN;
        if (tmp >= INT32_MAX)
            return INT32_MAX;
        return tmp;
    }
    static constexpr int32_t _mul(int32_t a, int32_t b) {
        int64_t tmp = ((int64_t)a * (int64_t)b) >> PAX_FIXPT_FRAC_BITS;
        if (tmp <= INT32_MIN)
            return INT32_MIN;
        if (tmp >= INT32_MAX)
            return INT32_MAX;
        return tmp;
    }
    template <typename T> static constexpr int32_t _from(T in) {
        if (in >= FIXPT_MAX)
            return INT32_MAX;
        if (in <= FIXPT_MIN)
            return INT32_MIN;
        return in * PAX_FIXPT_MUL;
    }
    template <typename T> static constexpr T _to(int32_t in) {
        return (T)(in / (T)PAX_FIXPT_MUL);
    }
    constexpr fixpt_t(int32_t val, bool dummy) : raw_value(val) {
    }
    static int32_t constexpr saturate_add(int32_t a, int32_t b) {
        if (a > 0 && b > INT32_MAX - a) {
            return INT32_MAX;
        } else if (a <= 0 && b < INT32_MIN - a) {
            return INT32_MIN;
        } else {
            return a + b;
        }
    }

  public:
    // Create from raw value (instead of conversion).
    static constexpr fixpt_t from_raw(int32_t raw) {
        return fixpt_t(raw, false);
    }

    // Default (0).
    constexpr fixpt_t() : raw_value(0) {
    }
    // Keep default copy constructor.
    constexpr fixpt_t(fixpt_t const &in) = default;
    // Keep default move constructor.
    constexpr fixpt_t(fixpt_t &&in)      = default;
    // Create from integer.
    constexpr fixpt_t(int in) : raw_value(_from(in)) {
    }
    // Create from float.
    constexpr fixpt_t(float in) : raw_value(_from(in)) {
    }
    // Create from float.
    constexpr fixpt_t(double in) : raw_value(_from(in)) {
    }

    // Keep default assignment operator.
    fixpt_t &operator=(fixpt_t const &) = default;
    // Keep default move assignment operator.
    fixpt_t &operator=(fixpt_t &&)      = default;
    // Set from integer.
    fixpt_t &operator=(int in) {
        raw_value = _from(in);
        return *this;
    }
    // Set from float.
    fixpt_t &operator=(float in) {
        raw_value = _from(in);
        return *this;
    }
    // Set from float.
    fixpt_t &operator=(double in) {
        raw_value = _from(in);
        return *this;
    }

    /* ==== Implicit conversion operators ==== */
    constexpr operator bool() const {
        return raw_value;
    }
    constexpr operator char() const {
        return _to<int>(raw_value);
    }
    constexpr operator short() const {
        return _to<int>(raw_value);
    }
    constexpr operator int() const {
        return _to<int>(raw_value);
    }
    constexpr operator long() const {
        return _to<long>(raw_value);
    }
    constexpr operator long long() const {
        return _to<long long>(raw_value);
    }
    constexpr operator float() const {
        return _to<float>(raw_value);
    }
    constexpr operator double() const {
        return _to<double>(raw_value);
    }
    constexpr operator long double() const {
        return _to<long double>(raw_value);
    }

    /* ==== Comparison operators ==== */
    template <typename T> constexpr bool operator==(T other) const {
        return raw_value == fixpt_t(other).raw_value;
    }
    template <typename T> constexpr bool operator!=(T other) const {
        return raw_value != fixpt_t(other).raw_value;
    }
    template <typename T> constexpr bool operator>=(T other) const {
        return raw_value >= fixpt_t(other).raw_value;
    }
    template <typename T> constexpr bool operator<=(T other) const {
        return raw_value <= fixpt_t(other).raw_value;
    }
    template <typename T> constexpr bool operator>(T other) const {
        return raw_value > fixpt_t(other).raw_value;
    }
    template <typename T> constexpr bool operator<(T other) const {
        return raw_value < fixpt_t(other).raw_value;
    }

    /* ==== Binary math operators ==== */
    constexpr fixpt_t operator+(fixpt_t other) const {
        return from_raw(saturate_add(raw_value, other.raw_value));
    }
    constexpr fixpt_t operator-(fixpt_t other) const {
        return from_raw(saturate_add(raw_value, -other.raw_value));
    }
    constexpr fixpt_t operator*(fixpt_t other) const {
        return from_raw(_mul(raw_value, other.raw_value));
    }
    constexpr fixpt_t operator/(fixpt_t other) const {
        return from_raw(_div(raw_value, other.raw_value));
    }
    constexpr bool operator<<(int other) const {
        return from_raw(raw_value << other);
    }
    constexpr bool operator>>(int other) const {
        return from_raw(raw_value >> other);
    }

    /* ==== Unary math operators ==== */
    constexpr bool operator!() const {
        return !raw_value;
    }
    constexpr fixpt_t operator+() const {
        return *this;
    }
    constexpr fixpt_t operator-() const {
        if (raw_value == INT32_MIN)
            return from_raw(INT32_MAX);
        return from_raw(-raw_value);
    }
    fixpt_t &operator++() {
        raw_value = saturate_add(raw_value, PAX_FIXPT_MUL);
        return *this;
    }
    fixpt_t &operator--() {
        raw_value = saturate_add(raw_value, -PAX_FIXPT_MUL);
        return *this;
    }
    fixpt_t operator++(int) {
        auto pre  = *this;
        raw_value = saturate_add(raw_value, PAX_FIXPT_MUL);
        return pre;
    }
    fixpt_t operator--(int) {
        auto pre  = *this;
        raw_value = saturate_add(raw_value, -PAX_FIXPT_MUL);
        return pre;
    }

    /* ==== Assignment math operators ==== */
    fixpt_t &operator+=(fixpt_t other) {
        raw_value = saturate_add(raw_value, other.raw_value);
        return *this;
    }
    fixpt_t &operator-=(fixpt_t other) {
        raw_value = saturate_add(raw_value, -other.raw_value);
        return *this;
    }
    fixpt_t &operator*=(fixpt_t other) {
        raw_value = _mul(raw_value, other.raw_value);
        return *this;
    }
    fixpt_t &operator/=(fixpt_t other) {
        raw_value = _div(raw_value, other.raw_value);
        return *this;
    }
    fixpt_t &operator<<=(int other) {
        raw_value = raw_value << other;
        return *this;
    }
    fixpt_t &operator>>=(int other) {
        raw_value = raw_value >> other;
        return *this;
    }
};

/* ==== Miscellaneous math functions ==== */
static inline constexpr fixpt_t abs(fixpt_t in) {
    return fixpt_t::from_raw(in.raw_value < 0 ? -in.raw_value : in.raw_value);
}

/* ==== Literal operator ==== */
static inline constexpr fixpt_t operator""_fix(unsigned long long in) {
    return fixpt_t((int)in);
}
static inline constexpr fixpt_t operator""_fix(long double in) {
    return fixpt_t((double)in);
}

/* ==== Fixed point and integer ==== */
static inline constexpr fixpt_t operator+(int a, fixpt_t b) {
    return fixpt_t(a) + fixpt_t(b);
}
static inline constexpr fixpt_t operator-(int a, fixpt_t b) {
    return fixpt_t(a) - fixpt_t(b);
}
static inline constexpr fixpt_t operator*(int a, fixpt_t b) {
    return fixpt_t(a) * fixpt_t(b);
}
static inline constexpr fixpt_t operator/(int a, fixpt_t b) {
    return fixpt_t(a) / fixpt_t(b);
}
static inline constexpr bool operator==(int a, fixpt_t b) {
    return fixpt_t(a) == fixpt_t(b);
}
static inline constexpr bool operator!=(int a, fixpt_t b) {
    return fixpt_t(a) != fixpt_t(b);
}
static inline constexpr bool operator<=(int a, fixpt_t b) {
    return fixpt_t(a) <= fixpt_t(b);
}
static inline constexpr bool operator>=(int a, fixpt_t b) {
    return fixpt_t(a) >= fixpt_t(b);
}
static inline constexpr bool operator<(int a, fixpt_t b) {
    return fixpt_t(a) < fixpt_t(b);
}
static inline constexpr bool operator>(int a, fixpt_t b) {
    return fixpt_t(a) > fixpt_t(b);
}

static inline constexpr fixpt_t operator+(fixpt_t a, int b) {
    return fixpt_t(a) + fixpt_t(b);
}
static inline constexpr fixpt_t operator-(fixpt_t a, int b) {
    return fixpt_t(a) - fixpt_t(b);
}
static inline constexpr fixpt_t operator*(fixpt_t a, int b) {
    return fixpt_t(a) * fixpt_t(b);
}
static inline constexpr fixpt_t operator/(fixpt_t a, int b) {
    return fixpt_t(a) / fixpt_t(b);
}
static inline constexpr bool operator==(fixpt_t a, int b) {
    return fixpt_t(a) == fixpt_t(b);
}
static inline constexpr bool operator!=(fixpt_t a, int b) {
    return fixpt_t(a) != fixpt_t(b);
}
static inline constexpr bool operator<=(fixpt_t a, int b) {
    return fixpt_t(a) <= fixpt_t(b);
}
static inline constexpr bool operator>=(fixpt_t a, int b) {
    return fixpt_t(a) >= fixpt_t(b);
}
static inline constexpr bool operator<(fixpt_t a, int b) {
    return fixpt_t(a) < fixpt_t(b);
}
static inline constexpr bool operator>(fixpt_t a, int b) {
    return fixpt_t(a) > fixpt_t(b);
}

/* ==== Fixed point and float ==== */
static inline constexpr fixpt_t operator+(float a, fixpt_t b) {
    return fixpt_t(a) + fixpt_t(b);
}
static inline constexpr fixpt_t operator-(float a, fixpt_t b) {
    return fixpt_t(a) - fixpt_t(b);
}
static inline constexpr fixpt_t operator*(float a, fixpt_t b) {
    return fixpt_t(a) * fixpt_t(b);
}
static inline constexpr fixpt_t operator/(float a, fixpt_t b) {
    return fixpt_t(a) / fixpt_t(b);
}
static inline constexpr bool operator==(float a, fixpt_t b) {
    return fixpt_t(a) == fixpt_t(b);
}
static inline constexpr bool operator!=(float a, fixpt_t b) {
    return fixpt_t(a) != fixpt_t(b);
}
static inline constexpr bool operator<=(float a, fixpt_t b) {
    return fixpt_t(a) <= fixpt_t(b);
}
static inline constexpr bool operator>=(float a, fixpt_t b) {
    return fixpt_t(a) >= fixpt_t(b);
}
static inline constexpr bool operator<(float a, fixpt_t b) {
    return fixpt_t(a) < fixpt_t(b);
}
static inline constexpr bool operator>(float a, fixpt_t b) {
    return fixpt_t(a) > fixpt_t(b);
}

static inline constexpr fixpt_t operator+(fixpt_t a, float b) {
    return fixpt_t(a) + fixpt_t(b);
}
static inline constexpr fixpt_t operator-(fixpt_t a, float b) {
    return fixpt_t(a) - fixpt_t(b);
}
static inline constexpr fixpt_t operator*(fixpt_t a, float b) {
    return fixpt_t(a) * fixpt_t(b);
}
static inline constexpr fixpt_t operator/(fixpt_t a, float b) {
    return fixpt_t(a) / fixpt_t(b);
}
static inline constexpr bool operator==(fixpt_t a, float b) {
    return fixpt_t(a) == fixpt_t(b);
}
static inline constexpr bool operator!=(fixpt_t a, float b) {
    return fixpt_t(a) != fixpt_t(b);
}
static inline constexpr bool operator<=(fixpt_t a, float b) {
    return fixpt_t(a) <= fixpt_t(b);
}
static inline constexpr bool operator>=(fixpt_t a, float b) {
    return fixpt_t(a) >= fixpt_t(b);
}
static inline constexpr bool operator<(fixpt_t a, float b) {
    return fixpt_t(a) < fixpt_t(b);
}
static inline constexpr bool operator>(fixpt_t a, float b) {
    return fixpt_t(a) > fixpt_t(b);
}

/* ==== Fixed point and float ==== */
static inline constexpr fixpt_t operator+(double a, fixpt_t b) {
    return fixpt_t(a) + fixpt_t(b);
}
static inline constexpr fixpt_t operator-(double a, fixpt_t b) {
    return fixpt_t(a) - fixpt_t(b);
}
static inline constexpr fixpt_t operator*(double a, fixpt_t b) {
    return fixpt_t(a) * fixpt_t(b);
}
static inline constexpr fixpt_t operator/(double a, fixpt_t b) {
    return fixpt_t(a) / fixpt_t(b);
}
static inline constexpr bool operator==(double a, fixpt_t b) {
    return fixpt_t(a) == fixpt_t(b);
}
static inline constexpr bool operator!=(double a, fixpt_t b) {
    return fixpt_t(a) != fixpt_t(b);
}
static inline constexpr bool operator<=(double a, fixpt_t b) {
    return fixpt_t(a) <= fixpt_t(b);
}
static inline constexpr bool operator>=(double a, fixpt_t b) {
    return fixpt_t(a) >= fixpt_t(b);
}
static inline constexpr bool operator<(double a, fixpt_t b) {
    return fixpt_t(a) < fixpt_t(b);
}
static inline constexpr bool operator>(double a, fixpt_t b) {
    return fixpt_t(a) > fixpt_t(b);
}

static inline constexpr fixpt_t operator+(fixpt_t a, double b) {
    return fixpt_t(a) + fixpt_t(b);
}
static inline constexpr fixpt_t operator-(fixpt_t a, double b) {
    return fixpt_t(a) - fixpt_t(b);
}
static inline constexpr fixpt_t operator*(fixpt_t a, double b) {
    return fixpt_t(a) * fixpt_t(b);
}
static inline constexpr fixpt_t operator/(fixpt_t a, double b) {
    return fixpt_t(a) / fixpt_t(b);
}
static inline constexpr bool operator==(fixpt_t a, double b) {
    return fixpt_t(a) == fixpt_t(b);
}
static inline constexpr bool operator!=(fixpt_t a, double b) {
    return fixpt_t(a) != fixpt_t(b);
}
static inline constexpr bool operator<=(fixpt_t a, double b) {
    return fixpt_t(a) <= fixpt_t(b);
}
static inline constexpr bool operator>=(fixpt_t a, double b) {
    return fixpt_t(a) >= fixpt_t(b);
}
static inline constexpr bool operator<(fixpt_t a, double b) {
    return fixpt_t(a) < fixpt_t(b);
}
static inline constexpr bool operator>(fixpt_t a, double b) {
    return fixpt_t(a) > fixpt_t(b);
}

#endif // !PAX_USE_FIXED_POINT
#endif // FIXPT_HPP
