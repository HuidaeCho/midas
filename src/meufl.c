#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "midas.h"

int main(int argc, char *argv[])
{
    int i;
    int print_usage = 1, calc_md5 = 0, from_one = 0, use_lessmem =
        0, compress_output = 0;
    char *dir_path = NULL, *dir_opts = NULL, *encoding = NULL, *flen_path =
        NULL;
    int num_threads = 0;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            int j, n = strlen(argv[i]);
            int unknown = 0;

            for (j = 1; j < n && !unknown; j++) {
                switch (argv[i][j]) {
                case 'o':
                    if (i == argc - 1) {
                        fprintf(stderr, "-%c: Missing output filename\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    flen_path = argv[++i];
                    break;
                case 'm':
                    calc_md5 = 1;
                    break;
                case '1':
                    from_one = 1;
                    break;
                case 'l':
                    use_lessmem = 1;
                    break;
                case 'L':
                    use_lessmem = 2;
                    break;
                case 'z':
                    compress_output = 1;
                    break;
                case 'e':
                    if (i == argc - 1) {
                        fprintf(stderr, "-%c: Missing encoding\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    encoding = argv[++i];
                    break;
                case 'D':
                    if (i == argc - 1) {
                        fprintf(stderr,
                                "-%c: Missing GDAL options for input direction\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    dir_opts = argv[++i];
                    break;
                case 't':
                    if (i == argc - 1) {
                        fprintf(stderr, "-%c: Missing number of threads\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    num_threads = atoi(argv[++i]);
                    break;
                default:
                    unknown = 1;
                    j--;
                    break;
                }
            }
            if (unknown) {
                fprintf(stderr, "%c: Unknown flag\n", argv[i][j]);
                print_usage = 2;
                break;
            }
        }
        else if (!dir_path) {
            dir_path = argv[i];
            print_usage = 0;
        }
        else {
            fprintf(stderr, "%s: Unable to process extra arguments\n",
                    argv[i]);
            print_usage = 2;
            break;
        }
    }

    if (!flen_path && !calc_md5) {
        fprintf(stderr, "Either -o or -m is required\n");
        print_usage = 2;
    }

    if (print_usage) {
        if (print_usage == 2)
            printf("\n");
        printf("Usage: meufl OPTIONS dir\n\n"
               "  dir\t\tInput flow direction raster (e.g., gpkg:file.gpkg:layer)\n"
               "  -o uflen\tOutput upstream flow length GeoTIFF (or -m required)\n"
               "  -m\t\tCalculate MD5 (or -o required)\n"
               "  -1\t\tCount from 1 (default: 0)\n"
               "  -l\t\tUse less memory\n"
               "  -L\t\tUse least memory\n"
               "  -z\t\tCompress output GeoTIFF\n"
               "  -e encoding\tInput flow direction encoding\n"
               "\t\tpower2 (default): 2^0-7 (E-NE CW) (e.g., r.terraflow, ArcGIS)\n"
               "\t\ttaudem: 1-8 (E-SE CCW) (e.g., d8flowdir)\n"
               "\t\t45degree: 1-8 (NE-E CCW) (e.g., r.watershed)\n"
               "\t\tdegree: (0,360] (E-E CCW)\n"
               "\t\tE,SE,S,SW,W,NW,N,NE: custom (e.g., 1,8,7,6,5,4,3,2 for taudem)\n"
               "  -D opts\tComma-separated list of GDAL options for dir\n"
               "  -t threads\tNumber of threads (default: OMP_NUM_THREADS)\n");
        exit(print_usage == 1 ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    if (meufl
        (dir_path, dir_opts, encoding, flen_path, from_one, use_lessmem,
         compress_output, num_threads))
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}
