#include "maths.h"
#include <math.h>

float
square(float x)
{
    return x * x;
}

float
mean(const float *x, int n)
{
    float sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += x[i];
    }
    return sum / n;
}

void
softmax(float *x, int n)
{
    float max = 0;
    for (int i = 0; i < n; i++)
    {
        if (x[i] > max)
        {
            max = x[i];
        }
    }
    float sum = 0;
    for (int i = 0; i < n; i++)
    {
        x[i] = exp(x[i] - max);
        sum += x[i];
    }
    for (int i = 0; i < n; i++)
    {
        x[i] /= sum;
    }
}

float
relu(float x)
{
    return x > 0 ? x : 0;
}

float
Q_rsqrt(float number)
{
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(long *) &y;           // evil floating point bit level hacking
    i = 0x5f3759df - (i >> 1);  // what the fuck?
    y = *(float *) &i;
    y = y * (threehalfs - (x2 * y * y));  // 1st iteration
    //	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

    return y;
}
