#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"

int main(int argc, char *argv[])
{
    int i;
    int print_usage = 1, save_outlets = 0, find_full = 0, use_lessmem = 2;
    double (*recode)(double, void *) = NULL;
    int *recode_data = NULL, encoding[8];
    char *dir_path = NULL, *dir_opts = NULL,
        *outlets_path = NULL, *outlets_layer = NULL, *outlets_opts = NULL,
        *id_col = NULL,
        *output_path = NULL, *oid_col = NULL,
        *lfp_name = NULL, *heads_name = NULL, *coors_path = NULL;
    int num_threads = 0;
    struct raster_map *dir_map;
    struct outlet_list *outlet_l;
    size_t num_cells;
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
        else if (lfp_name && use_lessmem == 1) {
            fprintf(stderr,
                    "Forced to preserve input data for vector routing; Ignoring -P\n");
            use_lessmem = 2;
        }
    }

    if (print_usage) {
        if (print_usage == 2)
            printf("\n");
        printf("Usage: melfp OPTIONS dir outlets id_col output oid_col\n");
        printf("\n");
        printf
            ("  dir\t\tInput flow direction raster (e.g., gpkg:file.gpkg:layer)\n");
        printf("  outlets\tInput outlets vector\n");
        printf("  id_col\tInput column for outlet IDs\n");
        printf("  output\tOutput GeoPackage or output text file with -W\n");
        printf("  oid_col\tOutput column for outlet IDs\n");
        printf("  -W\t\tWrite outlet rows and columns, and exit\n");
        printf("  -f\t\tFind full longest flow paths\n");
        printf("  -m\t\tUse more memory\n");
        printf("  -P\t\tDo not preserve input data (faster without -l)\n");
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
        printf("  -l lfp\tLayer name for output longest flow paths\n");
        printf("  -h heads\tLayer name for output longest flow path heads\n");
        printf
            ("  -c coors.csv\tOutput longest flow path head coordinates CSV\n");
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
              read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_BYTE, 0, recode,
                          recode_data))) {
            fprintf(stderr, "%s: Failed to read flow direction raster\n",
                    dir_path);
            exit(EXIT_FAILURE);
        }
    }
    else if (!(dir_map =
               read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_BYTE, 0, NULL,
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
                       dir_map, 0, 1 + find_full))) {
        fprintf(stderr, "%s: Failed to read outlets\n", outlets_path);
        exit(EXIT_FAILURE);
    }
    gettimeofday(&end_time, NULL);
    printf("Input time for outlets: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));

    num_cells = (size_t)dir_map->nrows * dir_map->ncols;
    printf("Number of cells: %zu\n", num_cells);
    printf("Number of outlets: %d\n", outlet_l->n);

    if (save_outlets) {
        printf("Writing outlets...\n");
        gettimeofday(&start_time, NULL);
        if (write_outlets(output_path, outlet_l) > 0) {
            fprintf(stderr, "%s: Failed to write outlets file\n",
                    output_path);
            free_raster(dir_map);
            free_outlet_list(outlet_l);
            exit(EXIT_FAILURE);
        }
        gettimeofday(&end_time, NULL);
        printf("Output time for outlets: %lld microsec\n",
               timeval_diff(NULL, &end_time, &start_time));
    }
    else {
        int append_layer = 0;
        int num_lfp = 0;

#ifdef LOOP_THEN_TASK
        guess_tracing_stack_size(dir_map, num_threads);
#endif

        printf("Finding longest flow paths...\n");
        gettimeofday(&start_time, NULL);
        lfp(dir_map, outlet_l, find_full, use_lessmem);
        gettimeofday(&end_time, NULL);
        printf
            ("Computation time for longest flow paths: %lld microsec\n",
             timeval_diff(NULL, &end_time, &start_time));

        for (i = 0; i < outlet_l->n; i++)
            num_lfp += outlet_l->head_pl[i].n;
        printf("Number of longest flow paths found: %d\n", num_lfp);

        if (lfp_name) {
            printf("Writing longest flow path lines <%s>...\n", output_path);
            gettimeofday(&start_time, NULL);
            if (write_lfp
                (output_path, lfp_name, oid_col, outlet_l, dir_map,
                 append_layer) > 0) {
                fprintf(stderr,
                        "%s: Failed to write longest flow path lines\n",
                        output_path);
                free_raster(dir_map);
                free_outlet_list(outlet_l);
                exit(EXIT_FAILURE);
            }
            gettimeofday(&end_time, NULL);
            printf("Output time for longest flow path lines: %lld microsec\n",
                   timeval_diff(NULL, &end_time, &start_time));
            append_layer = 1;
        }

        if (heads_name) {
            printf("Writing longest flow path heads <%s>...\n", output_path);
            gettimeofday(&start_time, NULL);
            if (write_lfp_heads
                (output_path, heads_name, oid_col, outlet_l, dir_map,
                 append_layer) > 0) {
                fprintf(stderr,
                        "%s: Failed to write longest flow path heads\n",
                        output_path);
                free_raster(dir_map);
                free_outlet_list(outlet_l);
                exit(EXIT_FAILURE);
            }
            gettimeofday(&end_time, NULL);
            printf("Output time for longest flow path heads: %lld microsec\n",
                   timeval_diff(NULL, &end_time, &start_time));
            append_layer = 1;
        }

        if (coors_path) {
            printf
                ("Writing longest flow path head coordinates <%s>...\n",
                 coors_path);
            gettimeofday(&start_time, NULL);
            if (write_lfp_head_coors(coors_path, oid_col, outlet_l, dir_map) >
                0) {
                fprintf(stderr,
                        "%s: Failed to write longest flow path head coordinates\n",
                        coors_path);
                free_raster(dir_map);
                free_outlet_list(outlet_l);
                exit(EXIT_FAILURE);
            }
            gettimeofday(&end_time, NULL);
            printf
                ("Output time for longest flow path head coordinates: %lld microsec\n",
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
