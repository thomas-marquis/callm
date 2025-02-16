#ifndef MATHS_H
#define MATHS_H

float square(float x);

float mean(const float *x, int n);

void softmax(float *x, int n);

float relu(float x);

float Q_rsqrt(float number);

#endif  // !#ifndef MATHS_H
