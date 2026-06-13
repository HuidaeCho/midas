#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "midas.h"

int read_encoding(const char *encoding_type,
                  double (**recode)(double, void *), int **encoding)
{
    int enc[8];

    if (!encoding_type || strcmp(encoding_type, "power2") == 0) {
        *recode = NULL;
        *encoding = NULL;
        return 0;
    }
    else if (strcmp(encoding_type, "taudem") == 0) {
        int i;

        for (i = 1; i < 9; i++)
            enc[i % 8] = 9 - i;
    }
    else if (strcmp(encoding_type, "45degree") == 0) {
        int i;

        for (i = 0; i < 8; i++)
            enc[i] = 8 - i;
    }
    else if (strcmp(encoding_type, "degree") == 0) {
        *recode = recode_degree;
        *encoding = NULL;
        return 0;
    }
    else if (sscanf
             (encoding_type, "%d,%d,%d,%d,%d,%d,%d,%d", &enc[0], &enc[1],
              &enc[2], &enc[3], &enc[4], &enc[5], &enc[6], &enc[7]) != 8) {
        fprintf(stderr, "%s: Invalid encoding\n", encoding_type);
        return 1;
    }

    *recode = recode_encoding;
    *encoding = malloc(sizeof enc);
    memcpy(*encoding, enc, sizeof enc);

    return 0;
}

void free_encoding(int *encoding)
{
    if (encoding)
        free(encoding);
}

double recode_encoding(double value, void *encoding)
{
    int *enc = encoding;
    int internal_encoding[8] = { E, SE, S, SW, W, NW, N, NE };
    int i;

    for (i = 0; i < 8 && value != enc[i]; i++) ;
    if (i < 8)
        value = internal_encoding[i];

    return value;
}

double recode_degree(double value, void *encoding)
{
    return pow(2, 8 - (int)((value + 22.5) / 45));
}
