#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"

int main(int argc, char *argv[])
{
    int i;
    int print_usage = 1, use_lessmem = 0, compress_output = 0, save_outlets =
        0;
    double (*recode)(double, void *) = NULL;
    int *recode_data = NULL, encoding[8];
    char *dir_path = NULL, *dir_opts = NULL,
        *outlets_path = NULL, *outlets_layer = NULL, *outlets_opts = NULL,
        *id_col = NULL, *wsheds_path = NULL, *hier_path = NULL;
    int num_threads = 0;
    struct raster_map *dir_map;
    struct outlet_list *outlet_l;
    struct timeval first_time, start_time, end_time;

    gettimeofday(&first_time, NULL);

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
                    if (strcmp(argv[++i], "power2") == 0) {
                        recode = NULL;
                        recode_data = NULL;
                        break;
                    }
                    else if (strcmp(argv[i], "taudem") == 0) {
                        int k;

                        for (k = 1; k < 9; k++)
                            encoding[k % 8] = 9 - k;
                    }
                    else if (strcmp(argv[i], "45degree") == 0) {
                        int k;

                        for (k = 0; k < 8; k++)
                            encoding[k] = 8 - k;
                    }
                    else if (strcmp(argv[i], "degree") == 0) {
                        recode = recode_degree;
                        recode_data = NULL;
                        break;
                    }
                    else if (sscanf
                             (argv[i], "%d,%d,%d,%d,%d,%d,%d,%d",
                              &encoding[0], &encoding[1], &encoding[2],
                              &encoding[3], &encoding[4], &encoding[5],
                              &encoding[6], &encoding[7]) != 8) {
                        fprintf(stderr, "%s: Invalid encoding\n", argv[i]);
                        print_usage = 2;
                        break;
                    }
                    recode = recode_encoding;
                    recode_data = encoding;
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

    printf("Reading flow direction raster <%s>...\n", dir_path);
    gettimeofday(&start_time, NULL);
    if (recode) {
        printf("Converting flow direction encoding...\n");
        if (!(dir_map =
              read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_INT32, 0,
                          recode, recode_data))) {
            fprintf(stderr, "%s: Failed to read flow direction raster\n",
                    dir_path);
            exit(EXIT_FAILURE);
        }
    }
    else if (!(dir_map =
               read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_INT32, 0, NULL,
                           NULL))) {
        fprintf(stderr, "%s: Failed to read flow direction raster\n",
                dir_path);
        exit(EXIT_FAILURE);
    }
    gettimeofday(&end_time, NULL);
    printf("Input time for flow direction: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));

    printf("Reading outlets <%s>...\n", outlets_path);
    gettimeofday(&start_time, NULL);
    if (!(outlet_l =
          read_outlets(outlets_path, outlets_layer, outlets_opts, id_col,
                       dir_map, hier_path != NULL, 0))) {
        fprintf(stderr, "%s: Failed to read outlets\n", outlets_path);
        exit(EXIT_FAILURE);
    }
    gettimeofday(&end_time, NULL);
    printf("Input time for outlets: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));

    if (save_outlets) {
        printf("Writing outlets...\n");
        gettimeofday(&start_time, NULL);
        if (write_outlets(wsheds_path, outlet_l) > 0) {
            fprintf(stderr, "%s: Failed to write outlets file\n",
                    wsheds_path);
            free_raster(dir_map);
            free_outlet_list(outlet_l);
            exit(EXIT_FAILURE);
        }
        gettimeofday(&end_time, NULL);
        printf("Output time for outlets: %lld microsec\n",
               timeval_diff(NULL, &end_time, &start_time));
    }
    else {
#ifdef LOOP_THEN_TASK
        guess_tracing_stack_size(dir_map, num_threads);
#endif

        printf("Delineating subwatersheds...\n");
        gettimeofday(&start_time, NULL);
        delineate(dir_map, outlet_l, use_lessmem);
        gettimeofday(&end_time, NULL);
        printf
            ("Computation time for subwatershed delineation: %lld microsec\n",
             timeval_diff(NULL, &end_time, &start_time));

        dir_map->compress = compress_output;
        printf("Writing subwatersheds raster <%s>...\n", wsheds_path);
        gettimeofday(&start_time, NULL);
        if (write_raster(wsheds_path, dir_map, RASTER_MAP_TYPE_AUTO) > 0) {
            fprintf(stderr, "%s: Failed to write subwatersheds raster\n",
                    wsheds_path);
            free_raster(dir_map);
            free_outlet_list(outlet_l);
            exit(EXIT_FAILURE);
        }
        gettimeofday(&end_time, NULL);
        printf("Output time for subwatersheds: %lld microsec\n",
               timeval_diff(NULL, &end_time, &start_time));

        if (hier_path) {
            struct shed_hier *hier;

            printf("Analyzing subwatershed hierarchy...\n");
            gettimeofday(&start_time, NULL);
            hier = analyze_shed_hier(dir_map, outlet_l);
            gettimeofday(&end_time, NULL);
            printf
                ("Analysis time for subwatershed hierarchy: %lld microsec\n",
                 timeval_diff(NULL, &end_time, &start_time));

            if (write_shed_hier(hier_path, hier, id_col) > 0) {
                fprintf(stderr,
                        "%s: Failed to write subwatershed hierarchy file\n",
                        hier_path);
                free_raster(dir_map);
                free_outlet_list(outlet_l);
                exit(EXIT_FAILURE);
            }
            gettimeofday(&end_time, NULL);
            printf("Output time for subwatershed hierarchy: %lld microsec\n",
                   timeval_diff(NULL, &end_time, &start_time));
        }
    }

    free_raster(dir_map);
    free_outlet_list(outlet_l);

    gettimeofday(&end_time, NULL);
    printf("Total elapsed time: %lld microsec\n",
           timeval_diff(NULL, &end_time, &first_time));

    exit(EXIT_SUCCESS);
}
