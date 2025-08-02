#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "midas.h"

int main(int argc, char *argv[])
{
    int i;
    int print_usage = 1, use_lessmem = 0, compress_output = 0,
        save_outlets = 0;
    char *dir_path = NULL, *dir_opts = NULL, *format = NULL,
        *outlets_path = NULL, *outlets_layer = NULL, *outlets_opts = NULL,
        *id_col = NULL, *wsheds_path = NULL, *hier_path = NULL;
    int num_threads = 0;

#ifdef LOOP_THEN_TASK
    int tracing_stack_size;

    read_tracing_stack_size(&tracing_stack_size);
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
                case 'm':
                    use_lessmem = 1;
                    break;
                case 'c':
                    compress_output = 1;
                    break;
                case 'e':
                    if (i == argc - 1) {
                        fprintf(stderr, "-%c: Missing encoding\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    format = argv[++i];
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
                case 'h':
                    if (i == argc - 1) {
                        fprintf(stderr,
                                "-%c: Missing output hierarchy path\n",
                                argv[i][j]);
                        print_usage = 2;
                        break;
                    }
                    hier_path = argv[++i];
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
                fprintf(stderr, "%c: Unknown flag\n", argv[i][--j]);
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
        else if (!wsheds_path) {
            wsheds_path = argv[i];
            print_usage = 0;
        }
        else {
            fprintf(stderr, "%s: Unable to process extra arguments\n",
                    argv[i]);
            print_usage = 2;
            break;
        }
    }

    if (save_outlets && (compress_output || hier_path)) {
        if (compress_output && hier_path)
            fprintf(stderr, "Unable to process -W, -c, and -h at once\n");
        else if (compress_output)
            fprintf(stderr, "Unable to process both -W and -c\n");
        else
            fprintf(stderr, "Unable to process both -W and -h\n");
        print_usage = 2;
    }

    if (print_usage) {
        if (print_usage == 2)
            printf("\n");
        printf("Usage: meshed OPTIONS dir outlets id_col output\n");
        printf("\n");
        printf
            ("  dir\t\tInput flow direction raster (e.g., gpkg:file.gpkg:layer)\n");
        printf("  outlets\tInput outlets vector\n");
        printf("  id_col\tInput column for outlet IDs\n");
        printf("  output\tOutput GeoTIFF or output text file with -W\n");
        printf("  -W\t\tWrite outlet rows and columns, and exit\n");
        printf("  -m\t\tUse less memory\n");
        printf("  -c\t\tCompress output GeoTIFF\n");
        printf("  -e encoding\tInput flow direction encoding\n");
        printf
            ("\t\tpower2 (default): 2^0-7 CW from E (e.g., r.terraflow, ArcGIS)\n");
        printf("\t\ttaudem: 1-8 (E-SE CCW) (e.g., d8flowdir)\n");
        printf("\t\t45degree: 1-8 (NE-E CCW) (e.g., r.watershed)\n");
        printf("\t\tdegree: (0,360] (E-E CCW)\n");
        printf
            ("\t\tE,SE,S,SW,W,NW,N,NE: custom (e.g., 1,8,7,6,5,4,3,2 for taudem)\n");
        printf("  -D opts\tComma-separated list of GDAL options for dir\n");
        printf
            ("  -O opts\tComma-separated list of GDAL options for outlets\n");
        printf
            ("  -o layer\tLayer name of input outlets vector, if necessary (e.g., gpkg)\n");
        printf("  -h hier.csv\tOutput subwatershed hierarchy CSV\n");
        printf("  -t threads\tNumber of threads (default OMP_NUM_THREADS)\n");
#ifdef LOOP_THEN_TASK
        printf("  -s size\tTracing stack size (default %d, 0 for guessing)\n",
               tracing_stack_size);
#endif
        exit(print_usage == 1 ? EXIT_SUCCESS : EXIT_FAILURE);
    }

    init_midas(&num_threads);

    if (meshed(dir_path, dir_opts, format,
               outlets_path, outlets_layer, outlets_opts, id_col,
               wsheds_path, hier_path, compress_output, save_outlets,
               use_lessmem, num_threads
#ifdef LOOP_THEN_TASK
               , tracing_stack_size
#endif
        ))
        exit(EXIT_FAILURE);

    exit(EXIT_SUCCESS);
}
