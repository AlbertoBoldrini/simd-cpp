/* 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the “Software”), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of 
 * the Software, and to permit persons to whom the Software is 
 * furnished to do so.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _simd_hpp_
#define _simd_hpp_
#include <type_traits>
#include <cmath>

namespace simd {
 
    template<class T, unsigned int N>
        class pack
        {
        public:

        #if defined (__clang__)
            // Underlying vector type aligned
            typedef T aligned   __attribute__((ext_vector_type(N)));

            // Underlying vector type unaligned
            typedef T unaligned __attribute__((ext_vector_type(N), __aligned__(1)));
        #else
            // Underlying vector type aligned
            typedef T aligned   __attribute__ ((vector_size(N * sizeof(T))));

            // Underlying vector type unaligned
            typedef T unaligned __attribute__ ((vector_size(N * sizeof(T)), __aligned__(1)));
        #endif

            // Underlying vector containing data
            aligned r;

            constexpr pack() {}
            constexpr pack(const aligned &r) : r(r)   {}
            constexpr pack(const pack    &s) : r(s.r) {} 

            // Costruction from single or multiple scalar values
            template<class    V> constexpr pack (const V &   x) : r(T(x) - aligned{}) {}
            template<class... V> constexpr pack (const V &...x) : r{T(x)...}          {}

            // Construction from different pack type
            template<class V> constexpr pack (const pack<V,N> &x) : r(__builtin_convertvector(x.r, aligned)) {}

            // Store and load operations
            static pack load  (const T *p) { return *reinterpret_cast<const aligned   *>(p); } 
            static pack loadu (const T *p) { return *reinterpret_cast<const unaligned *>(p); }    

            // Assignment operators
            template<class V> pack & operator  =  (const V &x) { r  =  pack(x).r; return *this; }
            template<class V> pack & operator +=  (const V &x) { r +=  pack(x).r; return *this; }
            template<class V> pack & operator -=  (const V &x) { r -=  pack(x).r; return *this; }
            template<class V> pack & operator *=  (const V &x) { r *=  pack(x).r; return *this; }
            template<class V> pack & operator /=  (const V &x) { r /=  pack(x).r; return *this; }
            template<class V> pack & operator %=  (const V &x) { r %=  pack(x).r; return *this; }
            template<class V> pack & operator &=  (const V &x) { r &=  pack(x).r; return *this; }
            template<class V> pack & operator |=  (const V &x) { r |=  pack(x).r; return *this; }
            template<class V> pack & operator ^=  (const V &x) { r ^=  pack(x).r; return *this; }
            template<class V> pack & operator <<= (const V &x) { r <<= pack(x).r; return *this; }
            template<class V> pack & operator >>= (const V &x) { r >>= pack(x).r; return *this; }

            // Conversion operator
            constexpr operator aligned () const { return r; }

            // Unary operators
            constexpr pack operator + () const { return +r; }
            constexpr pack operator - () const { return -r; }
            constexpr pack operator ~ () const { return ~r; }

            // Access operators
                      T & operator [] (unsigned int i)       { return r[i]; }    
            constexpr T   operator [] (unsigned int i) const { return r[i]; } 

            // Shuffle operators
            template<class V, unsigned int M> constexpr pack<T,M> operator [] (const pack<V,M> &s) const
                { return __builtin_shuffle(r, r, s.r); }

            template<class V> constexpr pack<V,N> shuffle (const pack<V,N> &a, const pack<V,N> &b) const
                { return __builtin_shuffle(a.r, b.r, r); }
            
            // Integer type of the same length
            typedef pack<decltype((r < r)[0]), N> int_type;
        };

    template <class T>
        struct pack_properties {
            typedef T type;
            constexpr unsigned int size = 1;
        };

    template<class T, unsigned int N>
        struct pack_properties<pack<T,N>> {
            typedef T type;
            constexpr unsigned int size = N;
        };

    template <class T>
        using type = typename pack_properties<T>::type;

    template<class T>
        constexpr unsigned int size = pack_properties<T>::size;

}


namespace std 
{
    // std::common_type definitions for pack objects
    template<class T, class V, unsigned int N>
        struct common_type<simd::pack<T,N>, simd::pack<V,N> > { using type = pack<common_type_t<T,V>, N>; };

    template<class T, class V, unsigned int N>
        struct common_type<simd::pack<T,N>, V> { using type = pack<common_type_t<T,V>, N>; };    

    template<class T, class V, unsigned int N>
        struct common_type<T, simd::pack<V,N> > { using type = pack<common_type_t<T,V>, N>; }; 

#ifdef _GLIBCXX_OSTREAM
    template<class T, unsigned int N>
        ostream & operator << (ostream &o, const simd::pack<T,N> &s)
        {   
            o << '(' << s[0];

            for (int i = 1; i < N; i++)
                o << ',' << s[i];

            return o << ')';
        }
#endif

#ifdef _GLIBCXX_ISTREAM
    template<class T, unsigned int N>
        istream & operator >> (istream &is, simd::pack<T,N> &s)
        {   
            for (int i = 0; i < N; i++)
                is >> s[i];

            return is;
        }
#endif
}

template<class T>
    constexpr bool is_simd = false;

template<class T, unsigned int N>
    constexpr bool is_simd< pack<T,N> > = true;

template<class T>
    constexpr bool is_simd_or_scalar = std::is_arithmetic<T>::value || is_simd<T>;

// Binary operators
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator +  (const T &x, const V &y) { return R(x).r +  R(y).r; }  
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator -  (const T &x, const V &y) { return R(x).r -  R(y).r; }  
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator *  (const T &x, const V &y) { return R(x).r *  R(y).r; }  
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator /  (const T &x, const V &y) { return R(x).r /  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator %  (const T &x, const V &y) { return R(x).r %  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator && (const T &x, const V &y) { return R(x).r && R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator || (const T &x, const V &y) { return R(x).r || R(y).r; }   
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator &  (const T &x, const V &y) { return R(x).r &  R(y).r; } 
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator |  (const T &x, const V &y) { return R(x).r |  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator ^  (const T &x, const V &y) { return R(x).r ^  R(y).r; }   
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator << (const T &x, const V &y) { return R(x).r << R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>> constexpr R operator >> (const T &x, const V &y) { return R(x).r >> R(y).r; }

// Comparison operators
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator == (const T &x, const V &y) { return R(x).r == R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator != (const T &x, const V &y) { return R(x).r != R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator <  (const T &x, const V &y) { return R(x).r <  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator <= (const T &x, const V &y) { return R(x).r <= R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator >  (const T &x, const V &y) { return R(x).r >  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_scalar<T> && is_simd_or_scalar<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator >= (const T &x, const V &y) { return R(x).r >= R(y).r; }

template<class R, class T, unsigned int N>
    inline pack<R,N> map(R (*function)(T), const pack<T,N> &s)
    {
        pack<R,N> out;

        for (unsigned int i = 0; i < N; i++)
            out[i] = function(s[i]);

        return out;
    }

template<class T, unsigned int N, class F, class R = std::result_of_t<F(T)>>
    inline pack<R,N> map(const F &function, const pack<T,N> &s)
    {
        pack<R,N> out;

        for (unsigned int i = 0; i < N; i++)
            out[i] = function(s[i]);

        return out;
    }

template<class R, class T, unsigned int N>
    inline pack<R,N> map(R (*function)(T, T), const pack<T,N> &a, const pack<T,N> &b)
    {
        pack<R,N> out;

        for (unsigned int i = 0; i < N; i++)
            out[i] = function(a[i], b[i]);

        return out;
    }

template<class T, unsigned int N, class F, class R>
    inline R reduce(const F &function, const pack<T,N> &s, const R &initial)
    {
        R out = initial;

        for (unsigned int i = 0; i < N; i++)
            out = function(out, s[i]);

        return out;
    }

template<class T, class V, class W, class E = std::enable_if_t<is_simd<T>>, class R = pack<typename std::common_type_t<T,V,W>::type,T::size>>
    constexpr R blend (const T &test, const V &yes, const W &no)
        { return test.r ? R(yes).r : R(no).r; }

template<class T, unsigned int N> inline bool any (const pack<T,N> &s) { bool r = s[0]; for(unsigned int i = 1; i < N; i++) r = r || s[i]; return r; }
template<class T, unsigned int N> inline bool all (const pack<T,N> &s) { bool r = s[0]; for(unsigned int i = 1; i < N; i++) r = r && s[i]; return r; }
template<class T, unsigned int N> inline T    sum (const pack<T,N> &s) { T    r = s[0]; for(unsigned int i = 1; i < N; i++) r = r +  s[i]; return r; }
template<class T, unsigned int N> inline T    prod(const pack<T,N> &s) { T    r = s[0]; for(unsigned int i = 1; i < N; i++) r = r *  s[i]; return r; }
template<class T, unsigned int N> inline T    max (const pack<T,N> &s) { T    r = s[0]; for(unsigned int i = 1; i < N; i++) r = r > s[i] ? r : s[i]; return r; }
template<class T, unsigned int N> inline T    min (const pack<T,N> &s) { T    r = s[0]; for(unsigned int i = 1; i < N; i++) r = r < s[i] ? r : s[i]; return r; }

namespace std 
{
    template<class T, unsigned int N> inline pack<T,N> cos    (const pack<T,N> &s) { return map<T>(std::cos,    s); }
    template<class T, unsigned int N> inline pack<T,N> sin    (const pack<T,N> &s) { return map<T>(std::sin,    s); }
    template<class T, unsigned int N> inline pack<T,N> tan    (const pack<T,N> &s) { return map<T>(std::tan,    s); }
    template<class T, unsigned int N> inline pack<T,N> acos   (const pack<T,N> &s) { return map<T>(std::acos,   s); }
    template<class T, unsigned int N> inline pack<T,N> asin   (const pack<T,N> &s) { return map<T>(std::asin,   s); }
    template<class T, unsigned int N> inline pack<T,N> atan   (const pack<T,N> &s) { return map<T>(std::atan,   s); }

    template<class T, unsigned int N> inline pack<T,N> cosh   (const pack<T,N> &s) { return map<T>(std::cosh,   s); }
    template<class T, unsigned int N> inline pack<T,N> sinh   (const pack<T,N> &s) { return map<T>(std::sinh,   s); }
    template<class T, unsigned int N> inline pack<T,N> tanh   (const pack<T,N> &s) { return map<T>(std::tanh,   s); }
    template<class T, unsigned int N> inline pack<T,N> acosh  (const pack<T,N> &s) { return map<T>(std::acosh,  s); }
    template<class T, unsigned int N> inline pack<T,N> asinh  (const pack<T,N> &s) { return map<T>(std::asinh,  s); }
    template<class T, unsigned int N> inline pack<T,N> atanh  (const pack<T,N> &s) { return map<T>(std::atanh,  s); } 

    template<class T, unsigned int N> inline pack<T,N> exp    (const pack<T,N> &s) { return map<T>(std::exp,    s); }
    template<class T, unsigned int N> inline pack<T,N> log    (const pack<T,N> &s) { return map<T>(std::log,    s); }
    template<class T, unsigned int N> inline pack<T,N> log10  (const pack<T,N> &s) { return map<T>(std::log10,  s); }
    template<class T, unsigned int N> inline pack<T,N> exp2   (const pack<T,N> &s) { return map<T>(std::exp2,   s); }
    template<class T, unsigned int N> inline pack<T,N> log1p  (const pack<T,N> &s) { return map<T>(std::log1p,  s); }
    template<class T, unsigned int N> inline pack<T,N> log2   (const pack<T,N> &s) { return map<T>(std::log2,   s); }

    template<class T, unsigned int N> inline pack<T,N> sqrt   (const pack<T,N> &s) { return map<T>(std::sqrt,   s); }
    template<class T, unsigned int N> inline pack<T,N> cbrt   (const pack<T,N> &s) { return map<T>(std::cbrt,   s); }
    template<class T, unsigned int N> inline pack<T,N> hypot  (const pack<T,N> &s) { return map<T>(std::hypot,  s); }

    template<class T, unsigned int N> inline pack<T,N> erf    (const pack<T,N> &s) { return map<T>(std::erf,    s); }
    template<class T, unsigned int N> inline pack<T,N> erfc   (const pack<T,N> &s) { return map<T>(std::erfc,   s); }
    template<class T, unsigned int N> inline pack<T,N> tgamma (const pack<T,N> &s) { return map<T>(std::tgamma, s); }
    template<class T, unsigned int N> inline pack<T,N> lgamma (const pack<T,N> &s) { return map<T>(std::lgamma, s); }

    template<class T, unsigned int N> inline pack<T,N> ceil   (const pack<T,N> &s) { return map<T>(std::ceil,   s); }
    template<class T, unsigned int N> inline pack<T,N> floor  (const pack<T,N> &s) { return map<T>(std::floor,  s); }
    template<class T, unsigned int N> inline pack<T,N> trunc  (const pack<T,N> &s) { return map<T>(std::trunc,  s); }
    template<class T, unsigned int N> inline pack<T,N> round  (const pack<T,N> &s) { return map<T>(std::round,  s); }
    template<class T, unsigned int N> inline pack<T,N> rint   (const pack<T,N> &s) { return map<T>(std::rint,   s); }
    template<class T, unsigned int N> inline pack<T,N> abs    (const pack<T,N> &s) { return map<T>(std::abs,    s); }

    template<class T, unsigned int N> inline pack<T,N> atan2  (const pack<T,N> &a, const pack<T,N> &b) { return map<T>(std::atan2, a, b); }
    template<class T, unsigned int N> inline pack<T,N> pow    (const pack<T,N> &a, const pack<T,N> &b) { return map<T>(std::pow,   a, b); }

    template<class T, unsigned int N> inline pack<T,N> max    (const pack<T,N> &a, const pack<T,N> &b) { return a.r > b.r ? a.r : b.r; }
    template<class T, unsigned int N> inline pack<T,N> min    (const pack<T,N> &a, const pack<T,N> &b) { return a.r < b.r ? a.r : b.r; }
}

} // namespace simd
#endif