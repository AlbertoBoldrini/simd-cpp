# Light SIMD library for modern C++

- [Design goals](#design-goals)
- [Integration](#integration)
- [Example](#example)
  - [Emitted instructions](#emitted-instructions)
- [License](#license)

## Design goals

Every [SIMD](https://it.wikipedia.org/wiki/Single_instruction_multiple_data) library out there is focused on some aspects, and each may even have its reason to exist. This class had these design goals:

- **Simplicity**. GCC and Clang compilers already produce good quality SIMD instructions using [vector extension](https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html). For most projects, there is no need to precise control emitted instructions. Instead, giving the compiler the freedom to choose the instructions you gain portability.
However, vector extensions are still complicated to use directly. This class simplifies their use with a simple wrapper. The `simd` class of this project 
behaves exactly like a normal `float` or `double`, replacement is often trivial. 
  

- **Trivial integration**. The whole code consists of a single header file containing a single class and some helper functions. You can simply copy `simd.hpp` in your project and include it. Many standard math functions are overloaded to work also with `simd` types. 

- **Easy customization**. The header file has a few hundreds of lines of code. If you have some experience with C++ templates you can understand the code just by reading it. If you need a specific functionality it should be easy to add it 
or you can just take a cue to write your class instead.

## Integration

[`simd.hpp`]() is the single required file. You need to add

```cpp
#include "simd.hpp"

// for convenience if you want
typedef simd<float, 8>  floatx8;
typedef simd<double, 8> doublex8;
...
```
and set the necessary switches to enable C++14 (e.g., `-std=c++14` for GCC and Clang). To obtain fast code you should enable optimization `-O3` or better `-Ofast` to speed up math expressions.
Don't forget to specify an architecture that supports simd with `-march` option, for example `-march=native`.


## Example
```cpp
#include <iostream>
#include <cstdint>
#include "../simd.hpp"

typedef simd<int32_t, 8> i32x8;
typedef simd<float, 8>   f32x8;

int main()
{
    f32x8 x; // Undefined values
    f32x8 y = 4; // All values are set to 4
    f32x8 z = {1,2,3,4,5,6,7,8}; // Initialized using a list of values

    // Read 8 values from the standard input
    std::cin >> x;

    // Some expressions
    f32x8 a = 3 * x + std::sqrt(x*x + y*y); // sqrt emits vsqrtps or vrsqrtps  
                                            // instructions depending depending 
                                            // on which optimizations have been enabled.

    f32x8 b = blend(a > 9, x, y); // Equivalent for each i to : a[i] = z[i] > 9 ? x[i] : y[i] 
    f32x8 c = blend(x < y, x, y); // Smaller values between x and y, same of std::min(x,y) 

    // Assigment operators
    x += 4 * a;
    x /= 3;

    // Access and set single elements
    y[3] = x[1];


    // Reduce operators
    if (any(x > 9))
        std::cout << "At least one value of x is greater than 9."  << std::endl;

    std::cout << "Sum of elements of z: " << sum(z) << std::endl;

    // Casts
    i32x8 g = x;
    f32x8 fz = z - i32x8(z);
    i32x8 indexes = {0,3,2,5,1,4,6,7};

    // Shuffle elements of z using indexes
    f32x8 z_shuffled = z[indexes];

    // Prints all values of z_shuffled
    std::cout << "z_shuffled: " << z_shuffled << std::endl;
    return 0;
}
```
### Emitted instructions
Compiling the example, the machine code produced by GCC 9.3 (only relevant parts) with `--std=c++14`, `-Ofast` and `-march=native` is the following
```assembly
...
  vmovaps -144(%rbp), %ymm2
  vxorps %xmm3, %xmm3, %xmm3
  vmovaps %ymm2, %ymm1
  vfmadd213ps .LC0(%rip), %ymm2, %ymm1
  vrsqrtps %ymm1, %ymm0 
  vcmpneqps %ymm1, %ymm3, %ymm3
  vandps %ymm3, %ymm0, %ymm0
  vmulps %ymm1, %ymm0, %ymm1
  vmulps %ymm0, %ymm1, %ymm0
  vmulps .LC2(%rip), %ymm1, %ymm1
  vaddps .LC1(%rip), %ymm0, %ymm0
  vmulps %ymm1, %ymm0, %ymm0
  vfmadd231ps .LC3(%rip), %ymm2, %ymm0
  vfmadd132ps .LC4(%rip), %ymm2, %ymm0
  vmulps .LC5(%rip), %ymm0, %ymm0
  vmovaps %ymm0, -144(%rbp)
  vcmpgtps .LC6(%rip), %ymm0, %ymm0
  vmovq %xmm0, %rax
  vmovdqa %ymm0, -112(%rbp)
...
  vmovaps -144(%rbp), %ymm5
  leaq _ZSt4cout(%rip), %rdi
  vextractf128 $0x1, %ymm5, %xmm1
  vaddps -144(%rbp), %xmm1, %xmm1
  vmovhlps %xmm1, %xmm1, %xmm0
  vaddps %xmm1, %xmm0, %xmm1
  vshufps $85, %xmm1, %xmm1, %xmm0
  vaddps %xmm1, %xmm0, %xmm0
  vcvtss2sd %xmm0, %xmm0, %xmm0
  vzeroupper
...
  vmovaps -144(%rbp), %ymm0
  vmovdqa .LC9(%rip), %ymm1
  movl $12, %edx
  vpermd %ymm0, %ymm1, %ymm1
  leaq .LC10(%rip), %rsi
  leaq _ZSt4cout(%rip), %rdi
  vmovaps %ymm0, -208(%rbp)
  vmovaps %ymm1, -112(%rbp)
  vzeroupper
...
  vmovaps -208(%rbp), %ymm0
  movq %rax, %rdi
  vcvtss2sd %xmm0, %xmm0, %xmm0
  vzeroupper
```

I encourage you to often look at the code produced by your compiler to make sure it is using the proper instructions.

## Data structure
To obtain big advantages from SIMD is often required to rewrite data structures of your application using simd as base types. For instance, if the application uses 3D vectors you should replace the base type inside the vector class with a simd type obtaining a "SIMD 3D vector" containing 8 independent 3D vectors.

```cpp
struct vec3Dx8 
{
    simd<float, 8> x, y, z;
    
    vec3Dx8 operator + (const vec3Dx8 &v){
        return {x + v.x, y + v.y, z + v.z};
    }

    simd<float, 8> operator * (const vec3Dx8 &v){
        return x * v.x + y * v.y + z * v.z;
    }

    ...
};
```

The replacement should be painless if the code makes little use of branches, in some cases you can use the `blend()` method to select between two values based on a condition (see [example](#example)).


## Licence
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so.


THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.