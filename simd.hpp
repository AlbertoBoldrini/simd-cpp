#ifndef _simd_hpp_
#define _simd_hpp_
#include <type_traits>
#include <cmath>

template<class T, unsigned int N>
    class simd
    {
    public:
        // Number of elements contained in this object
        static constexpr unsigned int size = N;

        // Type of the elements contained in this object
        typedef T type;

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

        constexpr simd() {}
        constexpr simd(const aligned &r) : r(r)   {}
        constexpr simd(const simd &s)    : r(s.r) {} 

        // Costruction from single or multiple scalar values
        template<class V>    constexpr simd (const V &   x) : r(T(x) - aligned{}) {}
        template<class... V> constexpr simd (const V &...x) : r{T(x)...}          {}

        // Construction from different simd type
        template<class V> constexpr simd (const simd<V,N> &x) : r(__builtin_convertvector(x.r, aligned)) {}

        // Store and load operations
        static simd load  (const T *p) { return *reinterpret_cast<const aligned   *>(p); } 
        static simd loadu (const T *p) { return *reinterpret_cast<const unaligned *>(p); }    

        // Assignment operators
        template<class V> simd & operator  =  (const V &x) { r  =  simd(x).r; return *this; }
        template<class V> simd & operator +=  (const V &x) { r +=  simd(x).r; return *this; }
        template<class V> simd & operator -=  (const V &x) { r -=  simd(x).r; return *this; }
        template<class V> simd & operator *=  (const V &x) { r *=  simd(x).r; return *this; }
        template<class V> simd & operator /=  (const V &x) { r /=  simd(x).r; return *this; }
        template<class V> simd & operator %=  (const V &x) { r %=  simd(x).r; return *this; }
        template<class V> simd & operator &=  (const V &x) { r &=  simd(x).r; return *this; }
        template<class V> simd & operator |=  (const V &x) { r |=  simd(x).r; return *this; }
        template<class V> simd & operator ^=  (const V &x) { r ^=  simd(x).r; return *this; }
        template<class V> simd & operator <<= (const V &x) { r <<= simd(x).r; return *this; }
        template<class V> simd & operator >>= (const V &x) { r >>= simd(x).r; return *this; }

        // Conversion operator
        constexpr operator aligned () const { return r; }

        // Unary operators
        constexpr simd operator + () const { return +r; }
        constexpr simd operator - () const { return -r; }
        constexpr simd operator ~ () const { return ~r; }

        // Access operators
                  T & operator [] (unsigned int i)       { return r[i]; }    
        constexpr T   operator [] (unsigned int i) const { return r[i]; } 

        // Shuffle operators
        template<class V, unsigned int M> constexpr simd<T,M> operator [] (const simd<V,M> &s) const
            { return __builtin_shufflevector(r, r, s.r); }

        template<class V> constexpr simd<V,N> shuffle (const simd<V,N> &a, const simd<V,N> &b) const
            { return __builtin_shufflevector(a.r, b.r, r); }

        template<class V> constexpr simd<V,N> blend (const simd<V,N> &a, const simd<V,N> &b) const
            { return r ? a.r : b.r; }
        
        // Integer type of the same length
        typedef simd<decltype((r < r)[0]), N> int_type;
    };

namespace std 
{
    // std::common_type definitions for simd objects
    template<class T, class V, unsigned int N>
        struct common_type<simd<T,N>, simd<V,N> > { using type = simd<common_type_t<T,V>, N>; };

    template<class T, class V, unsigned int N>
        struct common_type<simd<T,N>, V> { using type = simd<common_type_t<T,V>, N>; };    

    template<class T, class V, unsigned int N>
        struct common_type<T, simd<V,N> > { using type = simd<common_type_t<T,V>, N>; }; 

#ifdef _GLIBCXX_OSTREAM
    template<class T, unsigned int N>
        ostream & operator << (ostream &o, const simd<T,N> &s)
        {   
            o << '(' << s[0];

            for (int i = 1; i < N; i++)
                o << ',' << s[i];

            return o << ')';
        }
#endif

#ifdef _GLIBCXX_ISTREAM
    template<class T, unsigned int N>
        istream & operator >> (istream &is, simd<T,N> &s)
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
    constexpr bool is_simd< simd<T,N> > = true;

template<class T>
    constexpr bool is_simd_or_arithmetic = std::is_arithmetic<T>::value || is_simd<T>;

// Binary operators
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator +  (const T &x, const V &y) { return R(x).r +  R(y).r; }  
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator -  (const T &x, const V &y) { return R(x).r -  R(y).r; }  
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator *  (const T &x, const V &y) { return R(x).r *  R(y).r; }  
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator /  (const T &x, const V &y) { return R(x).r /  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator %  (const T &x, const V &y) { return R(x).r %  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator && (const T &x, const V &y) { return R(x).r && R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator || (const T &x, const V &y) { return R(x).r || R(y).r; }   
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator &  (const T &x, const V &y) { return R(x).r &  R(y).r; } 
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator |  (const T &x, const V &y) { return R(x).r |  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator ^  (const T &x, const V &y) { return R(x).r ^  R(y).r; }   
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator << (const T &x, const V &y) { return R(x).r << R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>> constexpr R operator >> (const T &x, const V &y) { return R(x).r >> R(y).r; }

// Comparison operators
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator == (const T &x, const V &y) { return R(x).r == R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator != (const T &x, const V &y) { return R(x).r != R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator <  (const T &x, const V &y) { return R(x).r <  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator <= (const T &x, const V &y) { return R(x).r <= R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator >  (const T &x, const V &y) { return R(x).r >  R(y).r; }
template<class T, class V, class E = std::enable_if_t<is_simd_or_arithmetic<T> && is_simd_or_arithmetic<V>>, class R = std::common_type_t<T,V>, class M = typename R::int_type> constexpr M operator >= (const T &x, const V &y) { return R(x).r >= R(y).r; }

template<class T, unsigned int N>
    bool any(const simd<T,N> &s)
    {
        bool out = false;

        for (unsigned int i = 0; i < N; i++)
            out = out || s[i];

        return out;
    }

template<class T, unsigned int N>
    bool all(const simd<T,N> &s)
    {
        bool out = true;

        for (unsigned int i = 0; i < N; i++)
            out = out && s[i];

        return out;
    }

template<class R, class T, unsigned int N>
    simd<R,N> map(R (*function)(T), const simd<T,N> &s)
    {
        simd<R,N> out;

        for (unsigned int i = 0; i < N; i++)
            out[i] = function(s[i]);

        return out;
    }

template<class T, unsigned int N, class F, class R = std::result_of_t<F(T)>>
    simd<R,N> map(const F &function, const simd<T,N> &s)
    {
        simd<R,N> out;

        for (unsigned int i = 0; i < N; i++)
            out[i] = function(s[i]);

        return out;
    }

template<class T, unsigned int N, class F, class R>
    R reduce(const F &function, const simd<T,N> &s, const R &initial)
    {
        R out = initial;

        for (unsigned int i = 0; i < N; i++)
            out = function(out, s[i]);

        return out;
    }

namespace std 
{
    template<class T, unsigned int N> inline simd<T,N> round(const simd<T,N> &s) { return map<T>(std::round, s); }
    template<class T, unsigned int N> inline simd<T,N> sqrt (const simd<T,N> &s) { return map<T>(std::sqrt,  s); }
    template<class T, unsigned int N> inline simd<T,N> exp  (const simd<T,N> &s) { return map<T>(std::exp,   s); }
    template<class T, unsigned int N> inline simd<T,N> log  (const simd<T,N> &s) { return map<T>(std::log,   s); }
    template<class T, unsigned int N> inline simd<T,N> cos  (const simd<T,N> &s) { return map<T>(std::cos,   s); }
    template<class T, unsigned int N> inline simd<T,N> sin  (const simd<T,N> &s) { return map<T>(std::sin,   s); }
}

#endif