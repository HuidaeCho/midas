#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "midas.h"

int main(int argc, char *argv[])
{
    int i;
    int print_usage = 1, find_full = 0, use_lessmem = 2, save_outlets = 0;
    char *dir_path = NULL, *dir_opts = NULL, *encoding = NULL,
        *outlets_path = NULL, *outlets_layer = NULL, *outlets_opts = NULL,
        *id_col = NULL,
        *output_path = NULL, *oid_col = NULL,
        *lfp_name = NULL, *heads_name = NULL, *coors_path = NULL;
    int num_threads = 0;

#ifdef LOOP_THEN_TASK
    int tracing_stack_size = 0;
#endif

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            int j, n = strlen(argv[i]);
            int unknown = 0;

            for (j = 1; j < n && !unknown; j++) {
                switch (argv[i][j]) {
                case 'W':
                    save_outlets = 1;
                    break;
                case 'f':
                    find_full = 1;
                    break;
                case 'm':
                    use_lessmem = 0;
                    break;
                case 'P':
                    if (use_lessmem)
                        use_lessmem = 1;
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
                case 'O':
                    if (i == argc - 1) {
                        fprintf(stderr,
                                "-%c: Missing GDAL options for input outlets\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    outlets_opts = argv[++i];
                    break;
                case 'o':
                    if (i == argc - 1) {
                        fprintf(stderr,
                                "-%c: Missing layer name for input outlets vector\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    outlets_layer = argv[++i];
                    break;
                case 'l':
                    if (i == argc - 1) {
                        fprintf(stderr,
                                "-%c: Missing layer name for output longest flow paths\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    lfp_name = argv[++i];
                    break;
                case 'h':
                    if (i == argc - 1) {
                        fprintf(stderr,
                                "-%c: Missing layer name for output longest flow path heads\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    heads_name = argv[++i];
                    break;
                case 'c':
                    if (i == argc - 1) {
                        fprintf(stderr,
                                "-%c: Missing output coordinates path\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    coors_path = argv[++i];
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
#ifdef LOOP_THEN_TASK
                case 's':
                    if (i == argc - 1) {
                        fprintf(stderr, "-%c: Missing tracing stack size\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    tracing_stack_size = atoi(argv[++i]);
                    break;
#endif
                default:
                    unknown = 1;
                    break;
                }
            }
            if (unknown) {
                fprintf(stderr, "-%c: Unknown flag\n", argv[i][--j]);
                print_usage = 2;
                break;
            }
        }
        else if (!dir_path)
            dir_path = argv[i];
        else if (!outlets_path)
            outlets_path = argv[i];
        else if (!id_col)
            id_col = argv[i];
        else if (!output_path)
            output_path = argv[i];
        else if (!oid_col) {
            oid_col = argv[i];
            print_usage = 0;
        }
        else {
            fprintf(stderr, "%s: Unable to process extra arguments\n",
                    argv[i]);
            print_usage = 2;
            break;
        }
    }

    if (!print_usage) {
        if (!lfp_name && !heads_name && !coors_path) {
            fprintf(stderr, "Missing output layers or file\n");
            print_usage = 2;
        }
        if (!lfp_name && !heads_name && !save_outlets)
            printf("No output vector layers specified; Not creating %s\n",
                   output_path);
    }

    if (print_usage) {
        if (print_usage == 2)
            printf("\n");
        printf("Usage: melfp OPTIONS dir outlets id_col output oid_col\n\n"
               "  dir\t\tInput flow direction raster (e.g., gpkg:file.gpkg:layer)\n"
               "  outlets\tInput outlets vector\n"
               "  id_col\tInput column for outlet IDs\n"
               "  output\tOutput GeoPackage or output text file with -W\n"
               "  oid_col\tOutput column for outlet IDs\n"
               "  -W\t\tWrite outlet rows and columns, and exit\n"
               "  -f\t\tFind full longest flow paths\n"
               "  -m\t\tUse more memory\n"
               "  -P\t\tDo not preserve input data (faster without -l)\n"
               "  -e encoding\tInput flow direction encoding\n"
               "\t\tpower2 (default): 2^0-7 CW from E (e.g., r.terraflow, ArcGIS)\n"
               "\t\ttaudem: 1-8 (E-SE CCW) (e.g., d8flowdir)\n"
               "\t\t45degree: 1-8 (NE-E CCW) (e.g., r.watershed)\n"
               "\t\tdegree: (0,360] (E-E CCW)\n"
               "\t\tE,SE,S,SW,W,NW,N,NE: custom (e.g., 1,8,7,6,5,4,3,2 for taudem)\n"
               "  -D opts\tComma-separated list of GDAL options for dir\n"
               "  -O opts\tComma-separated list of GDAL options for outlets\n"
               "  -o layer\tLayer name of input outlets vector, if necessary (e.g., gpkg)\n"
               "  -l lfp\tLayer name for output longest flow paths\n"
               "  -h heads\tLayer name for output longest flow path heads\n"
               "  -c coors.csv\tOutput longest flow path head coordinates CSV\n"
               "  -t threads\tNumber of threads (default OMP_NUM_THREADS)\n"
#ifdef LOOP_THEN_TASK
               "  -s size\tTracing stack size (default %d, 0 for guessing)\n",
               tracing_stack_size
#endif
            );
        exit(print_usage == 1 ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    if (melfp
        (dir_path, dir_opts, encoding, outlets_path, outlets_layer,
         outlets_opts, id_col, output_path, oid_col, lfp_name, heads_name,
         coors_path, find_full, use_lessmem, save_outlets, num_threads
#ifdef LOOP_THEN_TASK
         , tracing_stack_size
#endif
        ))
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}
