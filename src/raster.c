#include <math.h>
#include <gdal.h>
#include <cpl_string.h>
#include <omp.h>
#include "raster.h"

void print_raster(const char *path, const char *opts, const char *null_str,
                  const char *fmt)
{
    struct raster_map *rast_map;
    int width;
    char format[128], *type_format;
    int row, col;

    if (!(rast_map =
          read_raster(path, opts, RASTER_MAP_TYPE_AUTO, 1, NULL, NULL)))
        return;

    switch (rast_map->type) {
    case RASTER_MAP_TYPE_FLOAT64:
        type_format = "lf";
        break;
    case RASTER_MAP_TYPE_FLOAT32:
        type_format = "f";
        break;
    case RASTER_MAP_TYPE_UINT32:
    case RASTER_MAP_TYPE_UINT16:
        type_format = "u";
        break;
    default:
        type_format = "d";
        break;
    }

    width = rast_map->max > 0 ? (int)log10(rast_map->max) + 1 : 1;

    if (null_str) {
        int w = strlen(null_str);

        if (w > width)
            width = w;
    }

    if (fmt) {
        char *p;

        /* find a numeric format */
        for (p = (char *)fmt; *p && *p != '%'; p++) ;
        if (*p) {
            char *prefix = (char *)fmt;
            char *precision = "";

            /* put 0 at the end of prefix */
            *p = 0;

            /* find width */
            if (*(p + 1) >= '0' && *(p + 1) <= '9') {
                int w = atoi(++p);

                if (w > width)
                    width = w;
                /* skip the width */
                for (p++; *p >= '0' && *p <= '9'; p++) ;
                /* remember where the dot is if any */
                if (*p == '.')
                    /* skip precision */
                    for (precision = p++; *p >= '0' && *p <= '9'; p++) ;
            }
            else
                /* increase the width by the length of prefix */
                width += p++ - fmt;
            /* skip the type format */
            if (*p == 'l' && *(p + 1) == 'f') {
                /* put 0 at the end of precision */
                *p = 0;
                p += 2;
            }
            else {
                /* put 0 at the end of precision */
                *p = 0;
                p++;
            }
            sprintf(format, "%s%%%d%s%s%s%%s", prefix, width, precision,
                    type_format, p);
        }
        else {
            int w = strlen(fmt);

            if (w > width)
                width = w;

            sprintf(format, "%*s%%s", width, fmt);
        }
    }
    else {
        switch (rast_map->type) {
        case RASTER_MAP_TYPE_FLOAT64:
        case RASTER_MAP_TYPE_FLOAT32:
            sprintf(format, "%%%d.%d%s%%s", width + 4, 3, type_format);
            break;
        default:
            sprintf(format, "%%%d%s%%s", width, type_format);
            break;
        }
    }

    for (row = 0; row < rast_map->nrows; row++) {
        for (col = 0; col < rast_map->ncols; col++) {
            size_t idx = (size_t)row * rast_map->ncols + col;
            char *sep = col < rast_map->ncols - 1 ? " " : "";

            if (null_str && is_null(rast_map, row, col))
                printf("%*s%s", width, null_str, sep);
            else {
                switch (rast_map->type) {
                case RASTER_MAP_TYPE_FLOAT64:
                    printf(format, rast_map->cells.float64[idx], sep);
                    break;
                case RASTER_MAP_TYPE_FLOAT32:
                    printf(format, rast_map->cells.float32[idx], sep);
                    break;
                case RASTER_MAP_TYPE_UINT32:
                    printf(format, rast_map->cells.uint32[idx], sep);
                    break;
                case RASTER_MAP_TYPE_INT32:
                    printf(format, rast_map->cells.int32[idx], sep);
                    break;
                case RASTER_MAP_TYPE_UINT16:
                    printf(format, rast_map->cells.uint16[idx], sep);
                    break;
                case RASTER_MAP_TYPE_INT16:
                    printf(format, rast_map->cells.int16[idx], sep);
                    break;
                default:
                    printf(format, rast_map->cells.byte[idx], sep);
                    break;
                }
            }
        }
        printf("\n");
    }

    free_raster(rast_map);
}

int is_null(struct raster_map *rast_map, int row, int col)
{
    int ret;
    size_t idx = (size_t)row * rast_map->ncols + col;

    switch (rast_map->type) {
    case RASTER_MAP_TYPE_FLOAT64:
        ret = rast_map->cells.float64[idx] == rast_map->null_value ||
            isnan(rast_map->cells.float64[idx]);
        break;
    case RASTER_MAP_TYPE_FLOAT32:
        ret = rast_map->cells.float32[idx] == rast_map->null_value ||
            isnan(rast_map->cells.float32[idx]);
        break;
    case RASTER_MAP_TYPE_UINT32:
        ret = rast_map->cells.uint32[idx] == rast_map->null_value;
        break;
    case RASTER_MAP_TYPE_INT32:
        ret = rast_map->cells.int32[idx] == rast_map->null_value;
        break;
    case RASTER_MAP_TYPE_UINT16:
        ret = rast_map->cells.uint16[idx] == rast_map->null_value;
        break;
    case RASTER_MAP_TYPE_INT16:
        ret = rast_map->cells.int16[idx] == rast_map->null_value;
        break;
    default:
        ret = rast_map->cells.byte[idx] == rast_map->null_value;
        break;
    }
    return ret;
}

void set_null(struct raster_map *rast_map, int row, int col)
{
    size_t idx = (size_t)row * rast_map->ncols + col;

    switch (rast_map->type) {
    case RASTER_MAP_TYPE_FLOAT64:
        rast_map->cells.float64[idx] = rast_map->null_value;
        break;
    case RASTER_MAP_TYPE_FLOAT32:
        rast_map->cells.float32[idx] = rast_map->null_value;
        break;
    case RASTER_MAP_TYPE_UINT32:
        rast_map->cells.uint32[idx] = rast_map->null_value;
        break;
    case RASTER_MAP_TYPE_INT32:
        rast_map->cells.int32[idx] = rast_map->null_value;
        break;
    case RASTER_MAP_TYPE_UINT16:
        rast_map->cells.uint16[idx] = rast_map->null_value;
        break;
    case RASTER_MAP_TYPE_INT16:
        rast_map->cells.int16[idx] = rast_map->null_value;
        break;
    default:
        rast_map->cells.byte[idx] = rast_map->null_value;
        break;
    }
}

void reset_null(struct raster_map *rast_map, double null_value)
{
    int row, col;

#pragma omp parallel for schedule(dynamic) private(col)
    for (row = 0; row < rast_map->nrows; row++)
        for (col = 0; col < rast_map->ncols; col++) {
            size_t idx = (size_t)row * rast_map->ncols + col;

            switch (rast_map->type) {
            case RASTER_MAP_TYPE_FLOAT64:
                if (rast_map->cells.float64[idx] == rast_map->null_value ||
                    isnan(rast_map->cells.float64[idx]))
                    rast_map->cells.float64[idx] = null_value;
                break;
            case RASTER_MAP_TYPE_FLOAT32:
                if (rast_map->cells.float32[idx] == rast_map->null_value ||
                    isnan(rast_map->cells.float32[idx]))
                    rast_map->cells.float32[idx] = null_value;
                break;
            case RASTER_MAP_TYPE_UINT32:
                if (rast_map->cells.uint32[idx] == rast_map->null_value)
                    rast_map->cells.uint32[idx] = null_value;
                break;
            case RASTER_MAP_TYPE_INT32:
                if (rast_map->cells.int32[idx] == rast_map->null_value)
                    rast_map->cells.int32[idx] = null_value;
                break;
            case RASTER_MAP_TYPE_UINT16:
                if (rast_map->cells.uint16[idx] == rast_map->null_value)
                    rast_map->cells.uint16[idx] = null_value;
                break;
            case RASTER_MAP_TYPE_INT16:
                if (rast_map->cells.int16[idx] == rast_map->null_value)
                    rast_map->cells.int16[idx] = null_value;
                break;
            default:
                if (rast_map->cells.byte[idx] == rast_map->null_value)
                    rast_map->cells.byte[idx] = null_value;
                break;
            }
        }

    rast_map->null_value = null_value;
}

struct raster_map *init_raster(int nrows, int ncols, int type)
{
    struct raster_map *rast_map;
    size_t row_size;
    int i;

    rast_map = malloc(sizeof *rast_map);
    rast_map->nrows = nrows;
    rast_map->ncols = row_size = ncols;
    rast_map->type = type;

    switch (type) {
    case RASTER_MAP_TYPE_FLOAT64:
        row_size *= sizeof(double);
        break;
    case RASTER_MAP_TYPE_FLOAT32:
        row_size *= sizeof(float);
        break;
    case RASTER_MAP_TYPE_UINT32:
        row_size *= sizeof(unsigned int);
        break;
    case RASTER_MAP_TYPE_INT32:
        row_size *= sizeof(int);
        break;
    case RASTER_MAP_TYPE_UINT16:
        row_size *= sizeof(unsigned short);
        break;
    case RASTER_MAP_TYPE_INT16:
        row_size *= sizeof(short);
        break;
    }

    rast_map->cells.v = calloc(nrows, row_size);

    rast_map->null_value = 0;
    rast_map->projection = NULL;
    for (i = 0; i < 6; i++)
        rast_map->geotransform[i] = 0;
    rast_map->dx = rast_map->dy = 1;
    rast_map->compress = 0;

    return rast_map;
}

void free_raster(struct raster_map *rast_map)
{
    free(rast_map->cells.v);
    free(rast_map->projection);
}

void copy_raster_metadata(struct raster_map *dest_map,
                          const struct raster_map *src_map)
{
    int i;

    dest_map->projection = strdup(src_map->projection);
    for (i = 0; i < 6; i++)
        dest_map->geotransform[i] = src_map->geotransform[i];
    dest_map->dx = src_map->dx;
    dest_map->dy = src_map->dy;
}

struct raster_map *read_raster(const char *path, const char *opts, int type,
                               int get_stats, double (*recode)(double,
                                                               void *),
                               void *recode_data)
{
    struct raster_map *rast_map;
    const char **ds_opts = NULL;
    GDALDatasetH dataset;
    GDALRasterBandH band;
    GDALDataType gdt_type;
    size_t row_size;
    int row;
    int error = 0;

    if (opts) {
        char *o, *p;
        int n;

        strcpy((o = malloc(strlen(opts) + 1)), opts);
        for (p = o, n = 2; *p; p++)
            if (*p == ',')
                n++;
        ds_opts = malloc(sizeof *ds_opts * n);

        for (p = o, n = 0; *p; p++) {
            if (p == o || *(p - 1) == ',') {
                ds_opts[n] = p;
                if (p > o)
                    *(p - 1) = 0;
            }
        }
    }

    if (!(dataset =
          GDALOpenEx(path, GDAL_OF_RASTER | GDAL_OF_THREAD_SAFE, NULL,
                     ds_opts, NULL)))
        return NULL;

    rast_map = calloc(1, sizeof *rast_map);
    rast_map->nrows = GDALGetRasterYSize(dataset);
    rast_map->ncols = row_size = GDALGetRasterXSize(dataset);
    rast_map->projection = strdup(GDALGetProjectionRef(dataset));
    GDALGetGeoTransform(dataset, rast_map->geotransform);
    rast_map->dx = rast_map->geotransform[1];
    rast_map->dy = -rast_map->geotransform[5];

    band = GDALGetRasterBand(dataset, 1);

    if (get_stats) {
        rast_map->has_stats = 1;
        GDALGetRasterStatistics(band, 0, 1, &rast_map->min, &rast_map->max,
                                &rast_map->mean, &rast_map->sd);
    }

    rast_map->null_value = GDALGetRasterNoDataValue(band, NULL);
    rast_map->compress = 0;

    if (recode) {
        size_t gdt_row_size;
        GDALDataType rast_type;

        gdt_type = GDALGetRasterDataType(band);
        gdt_row_size = row_size * GDALGetDataTypeSizeBytes(gdt_type);

        switch (type) {
        case RASTER_MAP_TYPE_FLOAT64:
            rast_type = GDT_Float64;
            break;
        case RASTER_MAP_TYPE_FLOAT32:
            rast_type = GDT_Float32;
            break;
        case RASTER_MAP_TYPE_UINT32:
            rast_type = GDT_UInt32;
            break;
        case RASTER_MAP_TYPE_INT32:
            rast_type = GDT_Int32;
            break;
        case RASTER_MAP_TYPE_UINT16:
            rast_type = GDT_UInt16;
            break;
        case RASTER_MAP_TYPE_INT16:
            rast_type = GDT_Int16;
            break;
        case RASTER_MAP_TYPE_BYTE:
            rast_type = GDT_Byte;
            break;
        default:
            switch (gdt_type) {
            case GDT_Float64:
                rast_type = RASTER_MAP_TYPE_FLOAT64;
                break;
            case GDT_Float32:
                rast_type = RASTER_MAP_TYPE_FLOAT32;
                break;
            case GDT_UInt32:
                rast_type = RASTER_MAP_TYPE_UINT32;
                break;
            case GDT_Int32:
                rast_type = RASTER_MAP_TYPE_INT32;
                break;
            case GDT_UInt16:
                rast_type = RASTER_MAP_TYPE_UINT16;
                break;
            case GDT_Int16:
                rast_type = RASTER_MAP_TYPE_INT16;
                break;
            case GDT_Byte:
                rast_type = RASTER_MAP_TYPE_BYTE;
                break;
            default:
                fprintf(stderr, "Unsupported GDAL type\n");
                exit(EXIT_FAILURE);
                break;
            }
            break;
        }
        row_size *= GDALGetDataTypeSizeBytes(rast_type);

        rast_map->type = rast_type;
        rast_map->cells.v = malloc(rast_map->nrows * row_size);

        if (rast_type == gdt_type) {
#pragma omp parallel for schedule(dynamic)
            for (row = 0; row < rast_map->nrows; row++) {
                if (GDALRasterIO
                    (band, GF_Read, 0, row,
                     rast_map->ncols, 1,
                     (char *)rast_map->cells.v + row * row_size,
                     rast_map->ncols, 1, gdt_type, 0, 0) == CE_None) {
                    int col;

                    for (col = 0; col < rast_map->ncols; col++) {
                        size_t i = (size_t)row * rast_map->ncols + col;
                        double v;

                        switch (gdt_type) {
                        case GDT_Float64:
                            v = rast_map->cells.float64[i];
                            break;
                        case GDT_Float32:
                            v = rast_map->cells.float32[i];
                            break;
                        case GDT_UInt32:
                            v = rast_map->cells.uint32[i];
                            break;
                        case GDT_Int32:
                            v = rast_map->cells.int32[i];
                            break;
                        case GDT_UInt16:
                            v = rast_map->cells.uint16[i];
                            break;
                        case GDT_Int16:
                            v = rast_map->cells.int16[i];
                            break;
                        default:
                            v = rast_map->cells.byte[i];
                            break;
                        }

                        if (v == rast_map->null_value || isnan(v))
                            continue;

                        switch (gdt_type) {
                        case GDT_Float64:
                            rast_map->cells.float64[i] =
                                recode(v, recode_data);
                            break;
                        case GDT_Float32:
                            rast_map->cells.float32[i] =
                                recode(v, recode_data);
                            break;
                        case GDT_UInt32:
                            rast_map->cells.uint32[i] =
                                recode(v, recode_data);
                            break;
                        case GDT_Int32:
                            rast_map->cells.int32[i] = recode(v, recode_data);
                            break;
                        case GDT_UInt16:
                            rast_map->cells.uint16[i] =
                                recode(v, recode_data);
                            break;
                        case GDT_Int16:
                            rast_map->cells.int16[i] = recode(v, recode_data);
                            break;
                        default:
                            rast_map->cells.byte[i] = recode(v, recode_data);
                            break;
                        }
                    }
                }
                else
                    error = 1;
            }
        }
        else {
            union
            {
                void *v;
                unsigned char *byte;
                short *int16;
                unsigned short *uint16;
                int *int32;
                unsigned int *uint32;
                float *float32;
                double *float64;
            } *cells;

#pragma omp parallel
            {
#pragma omp single
                cells = malloc(sizeof *cells * omp_get_num_threads());
                cells[omp_get_thread_num()].v = malloc(gdt_row_size);
            }

            if (rast_type == GDT_Float64 || rast_type == GDT_Float32)
                rast_map->null_value = NAN;
            else if (rast_type == GDT_UInt32)
                rast_map->null_value = UINT_MAX;
            else if (rast_type == GDT_Int32)
                rast_map->null_value = INT_MAX;
            else if (rast_type == GDT_UInt16)
                rast_map->null_value = USHRT_MAX;
            else if (rast_type == GDT_Int16)
                rast_map->null_value = SHRT_MAX;
            else
                rast_map->null_value = UCHAR_MAX;

#pragma omp parallel for schedule(dynamic)
            for (row = 0; row < rast_map->nrows; row++) {
                int thread_num = omp_get_thread_num();

                if (GDALRasterIO
                    (band, GF_Read, 0, row,
                     rast_map->ncols, 1, cells[thread_num].v, rast_map->ncols,
                     1, gdt_type, 0, 0) == CE_None) {
                    int col;

                    for (col = 0; col < rast_map->ncols; col++) {
                        size_t i = (size_t)row * rast_map->ncols + col;
                        double v;

                        switch (gdt_type) {
                        case GDT_Float64:
                            v = cells[thread_num].float64[col];
                            break;
                        case GDT_Float32:
                            v = cells[thread_num].float32[col];
                            break;
                        case GDT_UInt32:
                            v = cells[thread_num].uint32[col];
                            break;
                        case GDT_Int32:
                            v = cells[thread_num].int32[col];
                            break;
                        case GDT_UInt16:
                            v = cells[thread_num].uint16[col];
                            break;
                        case GDT_Int16:
                            v = cells[thread_num].int16[col];
                            break;
                        default:
                            v = cells[thread_num].byte[col];
                            break;
                        }

                        if (v == rast_map->null_value || isnan(v)) {
                            switch (rast_type) {
                            case GDT_Float64:
                                rast_map->cells.float64[i] =
                                    rast_map->null_value;
                                break;
                            case GDT_Float32:
                                rast_map->cells.float32[i] =
                                    rast_map->null_value;
                                break;
                            case GDT_UInt32:
                                rast_map->cells.uint32[i] =
                                    rast_map->null_value;
                                break;
                            case GDT_Int32:
                                rast_map->cells.int32[i] =
                                    rast_map->null_value;
                                break;
                            case GDT_UInt16:
                                rast_map->cells.uint16[i] =
                                    rast_map->null_value;
                                break;
                            case GDT_Int16:
                                rast_map->cells.int16[i] =
                                    rast_map->null_value;
                                break;
                            default:
                                rast_map->cells.byte[i] =
                                    rast_map->null_value;
                                break;
                            }
                            continue;
                        }

                        switch (rast_type) {
                        case GDT_Float64:
                            rast_map->cells.float64[i] =
                                recode(v, recode_data);
                            break;
                        case GDT_Float32:
                            rast_map->cells.float32[i] =
                                recode(v, recode_data);
                            break;
                        case GDT_UInt32:
                            rast_map->cells.uint32[i] =
                                recode(v, recode_data);
                            break;
                        case GDT_Int32:
                            rast_map->cells.int32[i] = recode(v, recode_data);
                            break;
                        case GDT_UInt16:
                            rast_map->cells.uint16[i] =
                                recode(v, recode_data);
                            break;
                        case GDT_Int16:
                            rast_map->cells.int16[i] = recode(v, recode_data);
                            break;
                        default:
                            rast_map->cells.byte[i] = recode(v, recode_data);
                            break;
                        }
                    }
                }
                else
                    error = 1;
            }

#pragma omp parallel
            free(cells[omp_get_thread_num()].v);
            free(cells);
        }
    }
    else {
        switch (type) {
        case RASTER_MAP_TYPE_FLOAT64:
            gdt_type = GDT_Float64;
            break;
        case RASTER_MAP_TYPE_FLOAT32:
            gdt_type = GDT_Float32;
            break;
        case RASTER_MAP_TYPE_UINT32:
            gdt_type = GDT_UInt32;
            break;
        case RASTER_MAP_TYPE_INT32:
            gdt_type = GDT_Int32;
            break;
        case RASTER_MAP_TYPE_UINT16:
            gdt_type = GDT_UInt16;
            break;
        case RASTER_MAP_TYPE_INT16:
            gdt_type = GDT_Int16;
            break;
        case RASTER_MAP_TYPE_BYTE:
            gdt_type = GDT_Byte;
            break;
        default:
            switch ((gdt_type = GDALGetRasterDataType(band))) {
            case GDT_Float64:
                type = RASTER_MAP_TYPE_FLOAT64;
                break;
            case GDT_Float32:
                type = RASTER_MAP_TYPE_FLOAT32;
                break;
            case GDT_UInt32:
                type = RASTER_MAP_TYPE_UINT32;
                break;
            case GDT_Int32:
                type = RASTER_MAP_TYPE_INT32;
                break;
            case GDT_UInt16:
                type = RASTER_MAP_TYPE_UINT16;
                break;
            case GDT_Int16:
                type = RASTER_MAP_TYPE_INT16;
                break;
            case GDT_Byte:
                type = RASTER_MAP_TYPE_BYTE;
                break;
            default:
                fprintf(stderr, "Unsupported GDAL type\n");
                exit(EXIT_FAILURE);
                break;
            }
            break;
        }
        row_size *= GDALGetDataTypeSizeBytes(gdt_type);

        rast_map->type = type;
        rast_map->cells.v = malloc(rast_map->nrows * row_size);

#pragma omp parallel for schedule(dynamic)
        for (row = 0; row < rast_map->nrows; row++) {
            if (GDALRasterIO
                (band, GF_Read, 0, row,
                 rast_map->ncols, 1,
                 (char *)rast_map->cells.v + row * row_size, rast_map->ncols,
                 1, gdt_type, 0, 0) != CE_None)
                error = 1;
        }
    }

    GDALClose(dataset);

    if (error)
        return NULL;

    return rast_map;
}

int write_raster(const char *path, struct raster_map *rast_map, int type)
{
    GDALDriverH driver = GDALGetDriverByName("GTiff");
    char **metadata, **options = NULL;
    GDALDatasetH dataset;
    GDALRasterBandH band;
    GDALDataType data_type, gdt_type;
    size_t row_size;

    if (!driver)
        return 1;

    metadata = GDALGetMetadata(driver, NULL);
    if (!CSLFetchBoolean(metadata, GDAL_DCAP_CREATE, FALSE))
        return 2;

    if (rast_map->compress)
        options = CSLSetNameValue(options, "COMPRESS", "ZSTD");

    row_size = rast_map->ncols;

    /* actual data size */
    switch (rast_map->type) {
    case RASTER_MAP_TYPE_FLOAT64:
        data_type = GDT_Float64;
        break;
    case RASTER_MAP_TYPE_FLOAT32:
        data_type = GDT_Float32;
        break;
    case RASTER_MAP_TYPE_UINT32:
        data_type = GDT_UInt32;
        break;
    case RASTER_MAP_TYPE_INT32:
        data_type = GDT_Int32;
        break;
    case RASTER_MAP_TYPE_UINT16:
        data_type = GDT_UInt16;
        break;
    case RASTER_MAP_TYPE_INT16:
        data_type = GDT_Int16;
        break;
    default:
        data_type = GDT_Byte;
        break;
    }
    row_size *= GDALGetDataTypeSizeBytes(data_type);

    /* requested data type */
    gdt_type = data_type;
    if (type != RASTER_MAP_TYPE_AUTO && type != rast_map->type)
        switch (type) {
        case RASTER_MAP_TYPE_FLOAT64:
            gdt_type = GDT_Float64;
            break;
        case RASTER_MAP_TYPE_FLOAT32:
            gdt_type = GDT_Float32;
            break;
        case RASTER_MAP_TYPE_UINT32:
            gdt_type = GDT_UInt32;
            break;
        case RASTER_MAP_TYPE_INT32:
            gdt_type = GDT_Int32;
            break;
        case RASTER_MAP_TYPE_UINT16:
            gdt_type = GDT_UInt16;
            break;
        case RASTER_MAP_TYPE_INT16:
            gdt_type = GDT_Int16;
            break;
        default:
            gdt_type = GDT_Byte;
            break;
        }

    if (!(dataset =
          GDALCreate(driver, path, rast_map->ncols, rast_map->nrows, 1,
                     gdt_type, options)))
        return 3;

    GDALSetProjection(dataset, rast_map->projection);
    GDALSetGeoTransform(dataset, rast_map->geotransform);

    band = GDALGetRasterBand(dataset, 1);
    GDALSetRasterNoDataValue(band, rast_map->null_value);

    if (GDALRasterIO
        (band, GF_Write, 0, 0, rast_map->ncols, rast_map->nrows,
         (char *)rast_map->cells.v, rast_map->ncols, rast_map->nrows,
         data_type, 0, 0) != CE_None)
        return 4;

    GDALClose(dataset);

    return 0;
}

void calc_row_col(struct raster_map *rast_map, double x, double y,
                  int *row, int *col)
{
    *row = (rast_map->geotransform[4] * x - rast_map->geotransform[1] * y
            - rast_map->geotransform[4] * rast_map->geotransform[0]
            + rast_map->geotransform[1] * rast_map->geotransform[3]) /
        (rast_map->geotransform[4] * rast_map->geotransform[2]
         - rast_map->geotransform[1] * rast_map->geotransform[5]);
    *col = (rast_map->geotransform[5] * x - rast_map->geotransform[2] * y
            - rast_map->geotransform[5] * rast_map->geotransform[0]
            + rast_map->geotransform[2] * rast_map->geotransform[3]) /
        (rast_map->geotransform[5] * rast_map->geotransform[1]
         - rast_map->geotransform[2] * rast_map->geotransform[4]);
}

void calc_coors(struct raster_map *rast_map, int row, int col, double *x,
                double *y)
{
    *x = rast_map->geotransform[0] + rast_map->geotransform[1] * (col + 0.5) +
        rast_map->geotransform[2] * (row + 0.5);
    *y = rast_map->geotransform[3] + rast_map->geotransform[4] * (col + 0.5) +
        rast_map->geotransform[5] * (row + 0.5);
}
