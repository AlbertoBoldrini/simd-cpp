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

    f32x8 b = (a > 9).blend(x, y); // Equivalent for each i to : a[i] = z[i] > 9 ? x[i] : y[i] 
    f32x8 c = (x < y).blend(x, y); // Smaller values between x and y


    // Assigment operators
    x += 4 * a;
    x /= 3;

    // Access ans set single elements
    y[3] = x[1];


    // Reduce operators
    if (any(x > 9))
        std::cout << "At least one value of x is greater than 9."  << std::endl;

    std::cout << "Sum of elements of x: " << sum(x) << std::endl;

    // Casts
    i32x8 g = x;
    f32x8 fz = x - i32x8(x); // Fractional part of x
    i32x8 indexes = {0,3,2,5,1,4,6,7};

    // Shuffle elements of x using indexes
    f32x8 x_shuffled = x[indexes];

    // Prints all values of x_shuffled
    std::cout << "x_shuffled: " << x_shuffled << std::endl;
    return 0;
}
