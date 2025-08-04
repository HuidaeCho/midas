#define _MIDAS_C_

#include <omp.h>
#include <gdal.h>
#include <math.h>
#include "midas.h"

static int initialized = 0;
static void init_midas(int *);
static void set_tracing_stack_size(struct raster_map *, int, int *);

static void init_midas(int *num_threads)
{
    if (*num_threads == 0)
        *num_threads = omp_get_max_threads();
    else {
        if (*num_threads < 0) {
            *num_threads += omp_get_num_procs();
            if (*num_threads < 1)
                *num_threads = 1;
        }
        omp_set_num_threads(*num_threads);
    }
    printf("Using %d threads...\n", *num_threads);

    GDALAllRegister();

    initialized = 1;
}

#ifdef LOOP_THEN_TASK
static void set_tracing_stack_size(struct raster_map *dir_map,
                                   int num_threads, int *tracing_stack_size)
{
    if (*tracing_stack_size == 0) {
        char *p;

        if ((p = getenv("MIDAS_TRACING_STACK_SIZE")))
            *tracing_stack_size = atoi(p);
        else
            *tracing_stack_size = 1024 * 3;
    }
    else if (*tracing_stack_size < 0) {
        printf
            ("Guessing tracing stack size using sqrt(nrows * ncols) / num_threads...\n");
        *tracing_stack_size =
            sqrt((size_t)dir_map->nrows * dir_map->ncols) / num_threads;
    }

    printf("Tracing stack size for loop-then-task: %d\n",
           *tracing_stack_size);
}
#endif

int mefa(const char *dir_path, const char *dir_opts, const char *encoding,
         const char *accum_path, int use_lessmem, int compress_output,
         int num_threads)
{
    double (*recode)(double, void *);
    int *enc;
    struct raster_map *dir_map, *accum_map;
    struct timeval first_time, start_time, end_time;

    gettimeofday(&first_time, NULL);

    if (!initialized)
        init_midas(&num_threads);

    printf("Reading flow direction raster <%s>...\n", dir_path);
    gettimeofday(&start_time, NULL);
    if (read_encoding(encoding, &recode, &enc))
        fprintf(stderr, "%s: Invalid encoding\n", encoding);
    if (recode) {
        printf("Converting flow direction encoding...\n");
        if (!(dir_map =
              read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_BYTE, 0, recode,
                          enc))) {
            fprintf(stderr, "%s: Failed to read flow direction raster\n",
                    dir_path);
            return 1;
        }
    }
    else if (!(dir_map =
               read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_BYTE, 0, NULL,
                           NULL))) {
        fprintf(stderr, "%s: Failed to read flow direction raster\n",
                dir_path);
        return 1;
    }
    free_encoding(enc);
    gettimeofday(&end_time, NULL);
    printf("Input time for flow direction: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));

    accum_map = init_raster(dir_map->nrows, dir_map->ncols,
                            RASTER_MAP_TYPE_UINT32);
    copy_raster_metadata(accum_map, dir_map);

    printf("Accumulating flows...\n");
    gettimeofday(&start_time, NULL);
    accumulate(dir_map, accum_map, use_lessmem);
    gettimeofday(&end_time, NULL);
    printf("Computation time for flow accumulation: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));
    free_raster(dir_map);

    accum_map->compress = compress_output;
    printf("Writing flow accumulation raster <%s>...\n", accum_path);
    gettimeofday(&start_time, NULL);
    if (write_raster(accum_path, accum_map, RASTER_MAP_TYPE_AUTO) > 0) {
        fprintf(stderr, "%s: Failed to write flow accumulation raster\n",
                accum_path);
        free_raster(accum_map);
        return 1;
    }
    gettimeofday(&end_time, NULL);
    printf("Output time for flow accumulation: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));

    free_raster(accum_map);

    gettimeofday(&end_time, NULL);
    printf("Total elapsed time: %lld microsec\n",
           timeval_diff(NULL, &end_time, &first_time));

    return 0;
}

int meshed(const char *dir_path, const char *dir_opts, const char *encoding,
           const char *outlets_path, const char *outlets_layer,
           const char *outlets_opts, const char *id_col,
           const char *wsheds_path, const char *hier_path,
           int use_lessmem, int compress_output, int save_outlets,
           int num_threads
#ifdef LOOP_THEN_TASK
           , int tracing_stack_size
#endif
    )
{
    double (*recode)(double, void *);
    int *enc;
    struct raster_map *dir_map;
    struct outlet_list *outlet_l;
    struct timeval first_time, start_time, end_time;

    gettimeofday(&first_time, NULL);

    if (!initialized)
        init_midas(&num_threads);

    printf("Reading flow direction raster <%s>...\n", dir_path);
    gettimeofday(&start_time, NULL);
    if (read_encoding(encoding, &recode, &enc))
        fprintf(stderr, "%s: Invalid encoding\n", encoding);
    if (recode) {
        printf("Converting flow direction encoding...\n");
        if (!(dir_map =
              read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_INT32, 0,
                          recode, enc))) {
            fprintf(stderr, "%s: Failed to read flow direction raster\n",
                    dir_path);
            return 1;
        }
    }
    else if (!(dir_map =
               read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_INT32, 0, NULL,
                           NULL))) {
        fprintf(stderr, "%s: Failed to read flow direction raster\n",
                dir_path);
        return 1;
    }
    free_encoding(enc);
    gettimeofday(&end_time, NULL);
    printf("Input time for flow direction: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));

    printf("Reading outlets <%s>...\n", outlets_path);
    gettimeofday(&start_time, NULL);
    if (!(outlet_l =
          read_outlets(outlets_path, outlets_layer, outlets_opts, id_col,
                       dir_map, hier_path != NULL, 0))) {
        fprintf(stderr, "%s: Failed to read outlets\n", outlets_path);
        return 1;
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
            return 1;
        }
        gettimeofday(&end_time, NULL);
        printf("Output time for outlets: %lld microsec\n",
               timeval_diff(NULL, &end_time, &start_time));
    }
    else {
#ifdef LOOP_THEN_TASK
        set_tracing_stack_size(dir_map, num_threads, &tracing_stack_size);
#endif

        printf("Delineating subwatersheds...\n");
        gettimeofday(&start_time, NULL);
        delineate(dir_map, outlet_l, use_lessmem
#ifdef LOOP_THEN_TASK
                  , tracing_stack_size
#endif
            );
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
            return 1;
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
                return 1;
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

    return 0;
}

int melfp(const char *dir_path, const char *dir_opts, const char *encoding,
          const char *outlets_path, const char *outlets_layer,
          const char *outlets_opts, const char *id_col,
          const char *output_path, const char *oid_col, const char *lfp_name,
          const char *heads_name, const char *coors_path, int find_full,
          int use_lessmem, int save_outlets, int num_threads
#ifdef LOOP_THEN_TASK
          , int tracing_stack_size
#endif
    )
{
    double (*recode)(double, void *);
    int *enc;
    struct raster_map *dir_map;
    struct outlet_list *outlet_l;
    size_t num_cells;
    struct timeval first_time, start_time, end_time;

    gettimeofday(&first_time, NULL);

    if (!initialized)
        init_midas(&num_threads);

    if (lfp_name && use_lessmem == 1) {
        fprintf(stderr,
                "Forced to preserve input data for vector routing; Using use_lessmem=2\n");
        use_lessmem = 2;
    }

    printf("Reading flow direction raster <%s>...\n", dir_path);
    gettimeofday(&start_time, NULL);
    if (read_encoding(encoding, &recode, &enc))
        fprintf(stderr, "%s: Invalid encoding\n", encoding);
    if (recode) {
        printf("Converting flow direction encoding...\n");
        if (!(dir_map =
              read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_BYTE, 0, recode,
                          enc))) {
            fprintf(stderr, "%s: Failed to read flow direction raster\n",
                    dir_path);
            return 1;
        }
    }
    else if (!(dir_map =
               read_raster(dir_path, dir_opts, RASTER_MAP_TYPE_BYTE, 0, NULL,
                           NULL))) {
        fprintf(stderr, "%s: Failed to read flow direction raster\n",
                dir_path);
        return 1;
    }
    free_encoding(enc);
    gettimeofday(&end_time, NULL);
    printf("Input time for flow direction: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));

    printf("Reading outlets <%s>...\n", outlets_path);
    gettimeofday(&start_time, NULL);
    if (!(outlet_l =
          read_outlets(outlets_path, outlets_layer, outlets_opts, id_col,
                       dir_map, 0, 1 + find_full))) {
        fprintf(stderr, "%s: Failed to read outlets\n", outlets_path);
        return 1;
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
            return 1;
        }
        gettimeofday(&end_time, NULL);
        printf("Output time for outlets: %lld microsec\n",
               timeval_diff(NULL, &end_time, &start_time));
    }
    else {
        int i;
        int append_layer = 0;
        int num_lfp = 0;

#ifdef LOOP_THEN_TASK
        set_tracing_stack_size(dir_map, num_threads, &tracing_stack_size);
#endif

        printf("Finding longest flow paths...\n");
        gettimeofday(&start_time, NULL);
        lfp(dir_map, outlet_l, find_full, use_lessmem
#ifdef LOOP_THEN_TASK
            , tracing_stack_size
#endif
            );
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
                return 1;
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
                return 1;
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
                return 1;
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

    return 0;
}
