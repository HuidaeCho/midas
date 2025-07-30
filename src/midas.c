#define _MIDAS_C_

#include <omp.h>
#include <gdal.h>
#include <math.h>
#include "global.h"

void init_midas(int *num_threads)
{
#ifdef LOOP_THEN_TASK
    char *p;

    if ((p = getenv("MELFP_TRACING_STACK_SIZE")))
        tracing_stack_size = atoi(p);
    else
        tracing_stack_size = 1024 * 3;
#endif

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
}

#ifdef LOOP_THEN_TASK
void guess_tracing_stack_size(struct raster_map *dir_map, int num_threads)
{
    if (tracing_stack_size <= 0) {
        printf
            ("Guessing tracing stack size using sqrt(nrows * ncols) / num_threads...\n");
        tracing_stack_size =
            sqrt((size_t)dir_map->nrows * dir_map->ncols) / num_threads;
    }

    printf("Tracing stack size for loop-then-task: %d\n", tracing_stack_size);
}
#endif

int mefa(const char *dir_path, const char *dir_opts, const char *encoding,
         const char *accum_path, int use_lessmem, int compress_output,
         int num_threads)
{
    double (*recode)(double, void *) = NULL;
    int *recode_data = NULL;
    struct raster_map *dir_map, *accum_map;
    struct timeval first_time, start_time, end_time;

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
    gettimeofday(&end_time, NULL);
    printf("Input time for flow direction: %lld microsec\n",
           timeval_diff(NULL, &end_time, &start_time));

    accum_map =
        init_raster(dir_map->nrows, dir_map->ncols, RASTER_MAP_TYPE_UINT32);
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
