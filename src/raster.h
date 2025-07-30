#ifndef _RASTER_H_
#define _RASTER_H_

#define RASTER_MAP_TYPE_AUTO 0
#define RASTER_MAP_TYPE_BYTE 1
#define RASTER_MAP_TYPE_INT16 2
#define RASTER_MAP_TYPE_UINT16 3
#define RASTER_MAP_TYPE_INT32 4
#define RASTER_MAP_TYPE_UINT32 5
#define RASTER_MAP_TYPE_FLOAT32 6
#define RASTER_MAP_TYPE_FLOAT64 7

struct raster_map
{
    int type;
    int nrows, ncols;
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
    } cells;
    double null_value;
    char *projection;
    double geotransform[6];
    double dx, dy;
    int compress;
    int has_stats;
    double min;
    double max;
    double mean;
    double sd;
};

/* raster.c */
void print_raster(const char *, const char *, const char *, const char *);
int is_null(struct raster_map *, int, int);
void set_null(struct raster_map *, int, int);
void reset_null(struct raster_map *, double);
struct raster_map *init_raster(int, int, int);
void free_raster(struct raster_map *);
void copy_raster_metadata(struct raster_map *, const struct raster_map *);
struct raster_map *read_raster(const char *, const char *, int, int,
                               double (*)(double, void *), void *);
int write_raster(const char *, struct raster_map *, int);
void calc_row_col(struct raster_map *, double, double, int *, int *);
void calc_coors(struct raster_map *, int, int, double *, double *);

#endif
