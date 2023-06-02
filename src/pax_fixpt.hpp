/*
	MIT License

	Copyright (c) 2021-2023 Julian Scheffers

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#ifndef FIXPT_HPP
#define FIXPT_HPP

#include "pax_config.h"

#if !PAX_USE_FIXED_POINT
typedef float fixpt_t;
#else

#define PAX_FIXPT_INT_BITS  12
#define PAX_FIXPT_FRAC_BITS 20
#define PAX_FIXPT_MUL       0x00100000

class fixpt_t {
	public:
		int32_t raw_value;
		
	private:
		static constexpr int32_t _div(int32_t a, int32_t b) {
			return b ? (((int64_t) a << PAX_FIXPT_FRAC_BITS) / (int64_t) b) : (a > 0 ? INT32_MAX : INT32_MIN);
		}
		static constexpr int32_t _mul(int32_t a, int32_t b) {
			return ((int64_t) a * (int64_t) b) >> PAX_FIXPT_FRAC_BITS;
		}
		template<typename T>
		static constexpr int32_t _from(T in) {
			return in * PAX_FIXPT_MUL;
		}
		template<typename T>
		static constexpr T _to(int32_t in) {
			return (T) (in / (T) PAX_FIXPT_MUL);
		}
		constexpr fixpt_t(int32_t val, bool dummy): raw_value(val) {}
		
	public:
		// Create from raw value (instead of conversion).
		static constexpr fixpt_t from_raw(int32_t raw) {
			return fixpt_t(raw, false);
		}
		
		// Default (0).
		constexpr fixpt_t(): raw_value(0) {}
		// Keep default copy constructor.
		constexpr fixpt_t(const fixpt_t &in) = default;
		// Keep default move constructor.
		constexpr fixpt_t(fixpt_t &&in) = default;
		// Create from integer.
		constexpr fixpt_t(int in): raw_value(_from(in)) {}
		// Create from float.
		constexpr fixpt_t(float in): raw_value(_from(in)) {}
		// Create from float.
		constexpr fixpt_t(double in): raw_value(_from(in)) {}
		
		// Keep default assignment operator.
		fixpt_t &operator=(const fixpt_t &) = default;
		// Keep default move assignment operator.
		fixpt_t &operator=(fixpt_t &&) = default;
		// Set from integer.
		fixpt_t &operator=(int in) { raw_value = _from(in); return *this; }
		// Set from float.
		fixpt_t &operator=(float in) { raw_value = _from(in); return *this; }
		// Set from float.
		fixpt_t &operator=(double in) { raw_value = _from(in); return *this; }
		
		/* ==== Implicit conversion operators ==== */
		constexpr operator bool() const { return raw_value; }
		constexpr operator char() const { return _to<int>(raw_value); }
		constexpr operator short() const { return _to<int>(raw_value); }
		constexpr operator int() const { return _to<int>(raw_value); }
		constexpr operator long() const { return _to<long>(raw_value); }
		constexpr operator long long() const { return _to<long long>(raw_value); }
		constexpr operator float() const { return _to<float>(raw_value); }
		constexpr operator double() const { return _to<double>(raw_value); }
		constexpr operator long double() const { return _to<long double>(raw_value); }
		
		/* ==== Comparison operators ==== */
		template<typename T>
		constexpr bool operator==(T other) const { return raw_value == fixpt_t(other).raw_value; }
		template<typename T>
		constexpr bool operator!=(T other) const { return raw_value != fixpt_t(other).raw_value; }
		template<typename T>
		constexpr bool operator>=(T other) const { return raw_value >= fixpt_t(other).raw_value; }
		template<typename T>
		constexpr bool operator<=(T other) const { return raw_value <= fixpt_t(other).raw_value; }
		template<typename T>
		constexpr bool operator>(T other) const { return raw_value > fixpt_t(other).raw_value; }
		template<typename T>
		constexpr bool operator<(T other) const { return raw_value < fixpt_t(other).raw_value; }
		
		/* ==== Binary math operators ==== */
		constexpr fixpt_t operator+(fixpt_t other) const { return from_raw(raw_value + other.raw_value); }
		constexpr fixpt_t operator-(fixpt_t other) const { return from_raw(raw_value - other.raw_value); }
		constexpr fixpt_t operator*(fixpt_t other) const { return from_raw(_mul(raw_value, other.raw_value)); }
		constexpr fixpt_t operator/(fixpt_t other) const { return from_raw(_div(raw_value, other.raw_value)); }
		constexpr fixpt_t operator<<(int other) const { return from_raw(raw_value << other); }
		constexpr fixpt_t operator>>(int other) const { return from_raw(raw_value >> other); }
		
		/* ==== Unary math operators ==== */
		constexpr bool operator!() const { return !raw_value; }
		constexpr fixpt_t operator+() const { return *this; }
		constexpr fixpt_t operator-() const { return from_raw(-raw_value); }
		fixpt_t &operator++() { raw_value += PAX_FIXPT_MUL; return *this; }
		fixpt_t &operator--() { raw_value -= PAX_FIXPT_MUL; return *this; }
		fixpt_t operator++(int) { auto pre = *this; raw_value += PAX_FIXPT_MUL; return pre; }
		fixpt_t operator--(int) { auto pre = *this; raw_value -= PAX_FIXPT_MUL; return pre; }
		
		/* ==== Assignment math operators ==== */
		fixpt_t &operator+=(fixpt_t other) { raw_value = raw_value + other.raw_value; return *this; }
		fixpt_t &operator-=(fixpt_t other) { raw_value = raw_value - other.raw_value; return *this; }
		fixpt_t &operator*=(fixpt_t other) { raw_value = _mul(raw_value, other.raw_value); return *this; }
		fixpt_t &operator/=(fixpt_t other) { raw_value = _div(raw_value, other.raw_value); return *this; }
		fixpt_t &operator<<=(int other) { raw_value = raw_value << other; return *this; }
		fixpt_t &operator>>=(int other) { raw_value = raw_value >> other; return *this; }
};

/* ==== Literal operator ==== */
static inline constexpr fixpt_t operator ""_fix(unsigned long long in) { return fixpt_t((int) in); }
static inline constexpr fixpt_t operator ""_fix(long double in) { return fixpt_t((double) in); }

/* ==== Fixed point and integer ==== */
static inline constexpr fixpt_t operator+(int a, fixpt_t b) { return fixpt_t(a) + fixpt_t(b); }
static inline constexpr fixpt_t operator-(int a, fixpt_t b) { return fixpt_t(a) - fixpt_t(b); }
static inline constexpr fixpt_t operator*(int a, fixpt_t b) { return fixpt_t(a) * fixpt_t(b); }
static inline constexpr fixpt_t operator/(int a, fixpt_t b) { return fixpt_t(a) / fixpt_t(b); }
static inline constexpr fixpt_t operator==(int a, fixpt_t b) { return fixpt_t(a) == fixpt_t(b); }
static inline constexpr fixpt_t operator!=(int a, fixpt_t b) { return fixpt_t(a) != fixpt_t(b); }
static inline constexpr fixpt_t operator<=(int a, fixpt_t b) { return fixpt_t(a) <= fixpt_t(b); }
static inline constexpr fixpt_t operator>=(int a, fixpt_t b) { return fixpt_t(a) >= fixpt_t(b); }
static inline constexpr fixpt_t operator<(int a, fixpt_t b) { return fixpt_t(a) < fixpt_t(b); }
static inline constexpr fixpt_t operator>(int a, fixpt_t b) { return fixpt_t(a) > fixpt_t(b); }

static inline constexpr fixpt_t operator+(fixpt_t a, int b) { return fixpt_t(a) + fixpt_t(b); }
static inline constexpr fixpt_t operator-(fixpt_t a, int b) { return fixpt_t(a) - fixpt_t(b); }
static inline constexpr fixpt_t operator*(fixpt_t a, int b) { return fixpt_t(a) * fixpt_t(b); }
static inline constexpr fixpt_t operator/(fixpt_t a, int b) { return fixpt_t(a) / fixpt_t(b); }
static inline constexpr fixpt_t operator==(fixpt_t a, int b) { return fixpt_t(a) == fixpt_t(b); }
static inline constexpr fixpt_t operator!=(fixpt_t a, int b) { return fixpt_t(a) != fixpt_t(b); }
static inline constexpr fixpt_t operator<=(fixpt_t a, int b) { return fixpt_t(a) <= fixpt_t(b); }
static inline constexpr fixpt_t operator>=(fixpt_t a, int b) { return fixpt_t(a) >= fixpt_t(b); }
static inline constexpr fixpt_t operator<(fixpt_t a, int b) { return fixpt_t(a) < fixpt_t(b); }
static inline constexpr fixpt_t operator>(fixpt_t a, int b) { return fixpt_t(a) > fixpt_t(b); }

/* ==== Fixed point and float ==== */
static inline constexpr fixpt_t operator+(float a, fixpt_t b) { return fixpt_t(a) + fixpt_t(b); }
static inline constexpr fixpt_t operator-(float a, fixpt_t b) { return fixpt_t(a) - fixpt_t(b); }
static inline constexpr fixpt_t operator*(float a, fixpt_t b) { return fixpt_t(a) * fixpt_t(b); }
static inline constexpr fixpt_t operator/(float a, fixpt_t b) { return fixpt_t(a) / fixpt_t(b); }
static inline constexpr fixpt_t operator==(float a, fixpt_t b) { return fixpt_t(a) == fixpt_t(b); }
static inline constexpr fixpt_t operator!=(float a, fixpt_t b) { return fixpt_t(a) != fixpt_t(b); }
static inline constexpr fixpt_t operator<=(float a, fixpt_t b) { return fixpt_t(a) <= fixpt_t(b); }
static inline constexpr fixpt_t operator>=(float a, fixpt_t b) { return fixpt_t(a) >= fixpt_t(b); }
static inline constexpr fixpt_t operator<(float a, fixpt_t b) { return fixpt_t(a) < fixpt_t(b); }
static inline constexpr fixpt_t operator>(float a, fixpt_t b) { return fixpt_t(a) > fixpt_t(b); }

static inline constexpr fixpt_t operator+(fixpt_t a, float b) { return fixpt_t(a) + fixpt_t(b); }
static inline constexpr fixpt_t operator-(fixpt_t a, float b) { return fixpt_t(a) - fixpt_t(b); }
static inline constexpr fixpt_t operator*(fixpt_t a, float b) { return fixpt_t(a) * fixpt_t(b); }
static inline constexpr fixpt_t operator/(fixpt_t a, float b) { return fixpt_t(a) / fixpt_t(b); }
static inline constexpr fixpt_t operator==(fixpt_t a, float b) { return fixpt_t(a) == fixpt_t(b); }
static inline constexpr fixpt_t operator!=(fixpt_t a, float b) { return fixpt_t(a) != fixpt_t(b); }
static inline constexpr fixpt_t operator<=(fixpt_t a, float b) { return fixpt_t(a) <= fixpt_t(b); }
static inline constexpr fixpt_t operator>=(fixpt_t a, float b) { return fixpt_t(a) >= fixpt_t(b); }
static inline constexpr fixpt_t operator<(fixpt_t a, float b) { return fixpt_t(a) < fixpt_t(b); }
static inline constexpr fixpt_t operator>(fixpt_t a, float b) { return fixpt_t(a) > fixpt_t(b); }

/* ==== Fixed point and float ==== */
static inline constexpr fixpt_t operator+(double a, fixpt_t b) { return fixpt_t(a) + fixpt_t(b); }
static inline constexpr fixpt_t operator-(double a, fixpt_t b) { return fixpt_t(a) - fixpt_t(b); }
static inline constexpr fixpt_t operator*(double a, fixpt_t b) { return fixpt_t(a) * fixpt_t(b); }
static inline constexpr fixpt_t operator/(double a, fixpt_t b) { return fixpt_t(a) / fixpt_t(b); }
static inline constexpr fixpt_t operator==(double a, fixpt_t b) { return fixpt_t(a) == fixpt_t(b); }
static inline constexpr fixpt_t operator!=(double a, fixpt_t b) { return fixpt_t(a) != fixpt_t(b); }
static inline constexpr fixpt_t operator<=(double a, fixpt_t b) { return fixpt_t(a) <= fixpt_t(b); }
static inline constexpr fixpt_t operator>=(double a, fixpt_t b) { return fixpt_t(a) >= fixpt_t(b); }
static inline constexpr fixpt_t operator<(double a, fixpt_t b) { return fixpt_t(a) < fixpt_t(b); }
static inline constexpr fixpt_t operator>(double a, fixpt_t b) { return fixpt_t(a) > fixpt_t(b); }

static inline constexpr fixpt_t operator+(fixpt_t a, double b) { return fixpt_t(a) + fixpt_t(b); }
static inline constexpr fixpt_t operator-(fixpt_t a, double b) { return fixpt_t(a) - fixpt_t(b); }
static inline constexpr fixpt_t operator*(fixpt_t a, double b) { return fixpt_t(a) * fixpt_t(b); }
static inline constexpr fixpt_t operator/(fixpt_t a, double b) { return fixpt_t(a) / fixpt_t(b); }
static inline constexpr fixpt_t operator==(fixpt_t a, double b) { return fixpt_t(a) == fixpt_t(b); }
static inline constexpr fixpt_t operator!=(fixpt_t a, double b) { return fixpt_t(a) != fixpt_t(b); }
static inline constexpr fixpt_t operator<=(fixpt_t a, double b) { return fixpt_t(a) <= fixpt_t(b); }
static inline constexpr fixpt_t operator>=(fixpt_t a, double b) { return fixpt_t(a) >= fixpt_t(b); }
static inline constexpr fixpt_t operator<(fixpt_t a, double b) { return fixpt_t(a) < fixpt_t(b); }
static inline constexpr fixpt_t operator>(fixpt_t a, double b) { return fixpt_t(a) > fixpt_t(b); }

#endif // !PAX_USE_FIXED_POINT
#endif // FIXPT_HPP
