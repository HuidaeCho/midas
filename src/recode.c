#include <math.h>
#include "global.h"

double recode_encoding(double value, void *data)
{
    int *encoding = data;
    int internal_encoding[8] = { E, SE, S, SW, W, NW, N, NE };
    int i;

    for (i = 0; i < 8 && value != encoding[i]; i++) ;
    if (i < 8)
        value = internal_encoding[i];

    return value;
}

double recode_degree(double value, void *data)
{
    return pow(2, 8 - (int)((value + 22.5) / 45));
}
