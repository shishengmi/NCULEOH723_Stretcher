#include <myMath.h>
double calculateVariance(double data[], int size) {
    double sum = 0.0, mean, variance = 0.0;

    // Calculate the sum of all elements
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }

    // Calculate the mean
    mean = sum / size;

    // Calculate the variance
    for (int i = 0; i < size; ++i) {
        variance += pow(data[i] - mean, 2);
    }
    variance = variance / size;

    return variance;
}