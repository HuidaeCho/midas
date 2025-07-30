#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>

#ifdef _MSC_VER
#include <winsock2.h>
/* gettimeofday.c */
int gettimeofday(struct timeval *, struct timezone *);
#else
#include <sys/time.h>
#endif
#include "raster.h"

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

/* midas.c */
void init_midas(int *);

#ifdef LOOP_THEN_TASK
void read_tracing_stack_size(int *);
void guess_tracing_stack_size(struct raster_map *, int, int *);
#endif
int mefa(const char *, const char *, const char *, const char *, int, int,
         int);
int meshed(const char *, const char *, const char *,
           const char *, const char *, const char *, const char *,
           const char *, const char *, int, int, int, int
#ifdef LOOP_THEN_TASK
           , int
#endif
    );
int melfp(const char *, const char *, const char *, const char *,
          const char *, const char *, const char *, const char *,
          const char *, const char *, const char *, const char *, int, int,
          int, int
#ifdef LOOP_THEN_TASK
          , int
#endif
    );

/* timeval_diff.c */
long long timeval_diff(struct timeval *, struct timeval *, struct timeval *);

/* recode.c */
int read_encoding(const char *, double (**)(double, void *), int **);
void free_encoding(int *);
double recode_encoding(double, void *);
double recode_degree(double, void *);

/*******************************************************************************
 * MEFA only
 ******************************************************************************/
/* accumulate.c */
void accumulate(struct raster_map *, struct raster_map *, int);

/* accumulate_lessmem.c */
void accumulate_lessmem(struct raster_map *, struct raster_map *);

/* accumulate_moremem.c */
void accumulate_moremem(struct raster_map *, struct raster_map *);

/*******************************************************************************
 * MESHED and MELFP
 ******************************************************************************/
/* outlet_list.c */
void init_outlet_list(struct outlet_list *);
void reset_outlet_list(struct outlet_list *);
void free_outlet_list(struct outlet_list *);
void add_outlet(struct outlet_list *, int, int, int, unsigned char, int);

/* outlets.c */
struct outlet_list *read_outlets(const char *, const char *, const char *,
                                 const char *, struct raster_map *, int, int);
int write_outlets(const char *, struct outlet_list *);

/*******************************************************************************
 * MESHED only
 ******************************************************************************/
/* delineate.c */
void delineate(struct raster_map *, struct outlet_list *, int
#ifdef LOOP_THEN_TASK
               , int
#endif
    );

/* delineate_lessmem.c */
void delineate_lessmem(struct raster_map *, struct outlet_list *
#ifdef LOOP_THEN_TASK
                       , int
#endif
    );

/* delineate_moremem.c */
void delineate_moremem(struct raster_map *, struct outlet_list *
#ifdef LOOP_THEN_TASK
                       , int
#endif
    );

/* shed_hier.c */
struct shed_hier *analyze_shed_hier(struct raster_map *,
                                    struct outlet_list *);
int write_shed_hier(const char *, struct shed_hier *, const char *);

/*******************************************************************************
 * MELFP only
 ******************************************************************************/
/* point_list.c */
void init_point_list(struct point_list *);
void reset_point_list(struct point_list *);
void free_point_list(struct point_list *);
void add_point(struct point_list *, int, int);

void lfp(struct raster_map *, struct outlet_list *, int, int
#ifdef LOOP_THEN_TASK
         , int
#endif
    );

/* lfp_lessmem.c */
void lfp_lessmem(struct raster_map *, struct outlet_list *, int, int
#ifdef LOOP_THEN_TASK
                 , int
#endif
    );

/* lfp_moremem.c */
void lfp_moremem(struct raster_map *, struct outlet_list *, int
#ifdef LOOP_THEN_TASK
                 , int
#endif
    );

/* write_lfp.c */
int write_lfp(const char *, const char *, const char *, struct outlet_list *,
              struct raster_map *, int);
int write_lfp_heads(const char *, const char *, const char *,
                    struct outlet_list *, struct raster_map *, int);
int write_lfp_head_coors(const char *, const char *, struct outlet_list *,
                         struct raster_map *);

#endif
