#ifndef _MIDAS_H_
#define _MIDAS_H_

#ifdef _WIN32
#ifdef EXPORTS
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif
#else
#define EXPORT
#endif

#include <stdint.h>

#ifdef _MSC_VER
#include <winsock2.h>
/* gettimeofday.c */
EXPORT int gettimeofday(struct timeval *, struct timezone *);
#else
#include <sys/time.h>
#endif

#define LOOP_THEN_TASK

#define REALLOC_INCREMENT 1024

#define NE 128
#define N 64
#define NW 32
#define W 16
#define SW 8
#define S 4
#define SE 2
#define E 1

#define SHED_NULL INT32_MIN
#define SHED_HIER_NULL -1

struct outlet_list
{
    int nalloc, n;
    int *row, *col;
    int *id;

    /* MESHED only */
    unsigned char *dir;

    /* MELFP only */
    int *northo, *ndia;
    double *lflen;
    struct point_list *head_pl;
    /* for full lfp */
    char *has_up;
    int *down;
    double *flen;
};

struct point_list
{
    int nalloc, n;
    int *row, *col;
};

struct shed_hier
{
    int n;
    int *self, *up, *down;
};

/* raster.c */
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

EXPORT void print_raster(const char *, const char *, const char *,
                         const char *);
EXPORT int is_null(struct raster_map *, int, int);
EXPORT void set_null(struct raster_map *, int, int);
EXPORT void reset_null(struct raster_map *, double);
EXPORT struct raster_map *init_raster(int, int, int);
EXPORT void free_raster(struct raster_map *);
EXPORT void copy_raster_metadata(struct raster_map *,
                                 const struct raster_map *);
EXPORT struct raster_map *read_raster(const char *, const char *, int, int,
                                      double (*)(double, void *), void *);
EXPORT int write_raster(const char *, struct raster_map *, int);
EXPORT void calc_row_col(struct raster_map *, double, double, int *, int *);
EXPORT void calc_coors(struct raster_map *, int, int, double *, double *);

/* midas.c */
EXPORT int mefa(const char *, const char *, const char *, const char *,
                int, int, int);
EXPORT int meshed(const char *, const char *, const char *, const char *,
                  const char *, const char *, const char *, const char *,
                  const char *, int, int, int, int
#ifdef LOOP_THEN_TASK
                  , int
#endif
    );
EXPORT int melfp(const char *, const char *, const char *, const char *,
                 const char *, const char *, const char *, const char *,
                 const char *, const char *, const char *, const char *,
                 int, int, int, int
#ifdef LOOP_THEN_TASK
                 , int
#endif
    );

/* timeval_diff.c */
EXPORT long long timeval_diff(struct timeval *, struct timeval *,
                              struct timeval *);

/* recode.c */
EXPORT int read_encoding(const char *, double (**)(double, void *), int **);
EXPORT void free_encoding(int *);
EXPORT double recode_encoding(double, void *);
EXPORT double recode_degree(double, void *);

/*******************************************************************************
 * MEFA only
 ******************************************************************************/
/* accumulate.c */
EXPORT void accumulate(struct raster_map *, struct raster_map *, int);

/* accumulate_lessmem.c */
EXPORT void accumulate_lessmem(struct raster_map *, struct raster_map *);

/* accumulate_moremem.c */
EXPORT void accumulate_moremem(struct raster_map *, struct raster_map *);

/*******************************************************************************
 * MESHED and MELFP
 ******************************************************************************/
/* outlet_list.c */
EXPORT void init_outlet_list(struct outlet_list *);
EXPORT void reset_outlet_list(struct outlet_list *);
EXPORT void free_outlet_list(struct outlet_list *);
EXPORT void add_outlet(struct outlet_list *, int, int, int, unsigned char,
                       int);

/* outlets.c */
EXPORT struct outlet_list *read_outlets(const char *, const char *,
                                        const char *, const char *,
                                        struct raster_map *, int, int);
EXPORT int write_outlets(const char *, struct outlet_list *);

/*******************************************************************************
 * MESHED only
 ******************************************************************************/
/* delineate.c */
EXPORT void delineate(struct raster_map *, struct outlet_list *, int
#ifdef LOOP_THEN_TASK
                      , int
#endif
    );

/* delineate_lessmem.c */
EXPORT void delineate_lessmem(struct raster_map *, struct outlet_list *
#ifdef LOOP_THEN_TASK
                              , int
#endif
    );

/* delineate_moremem.c */
EXPORT void delineate_moremem(struct raster_map *, struct outlet_list *
#ifdef LOOP_THEN_TASK
                              , int
#endif
    );

/* shed_hier.c */
EXPORT struct shed_hier *analyze_shed_hier(struct raster_map *,
                                           struct outlet_list *);
EXPORT int write_shed_hier(const char *, struct shed_hier *, const char *);

/*******************************************************************************
 * MELFP only
 ******************************************************************************/
/* point_list.c */
EXPORT void init_point_list(struct point_list *);
EXPORT void reset_point_list(struct point_list *);
EXPORT void free_point_list(struct point_list *);
EXPORT void add_point(struct point_list *, int, int);

EXPORT void lfp(struct raster_map *, struct outlet_list *, int, int
#ifdef LOOP_THEN_TASK
                , int
#endif
    );

/* lfp_lessmem.c */
EXPORT void lfp_lessmem(struct raster_map *, struct outlet_list *, int, int
#ifdef LOOP_THEN_TASK
                        , int
#endif
    );

/* lfp_moremem.c */
EXPORT void lfp_moremem(struct raster_map *, struct outlet_list *, int
#ifdef LOOP_THEN_TASK
                        , int
#endif
    );

/* write_lfp.c */
EXPORT int write_lfp(const char *, const char *, const char *,
                     struct outlet_list *, struct raster_map *, int);
EXPORT int write_lfp_heads(const char *, const char *, const char *,
                           struct outlet_list *, struct raster_map *, int);
EXPORT int write_lfp_head_coors(const char *, const char *,
                                struct outlet_list *, struct raster_map *);

#endif
