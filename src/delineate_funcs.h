#include <stdio.h>
#include <stdlib.h>
#include "global.h"

#define INDEX(row, col) ((size_t)(row) * ncols + (col))
#define DIR_NULL dir_map->null_value
#define DIR(row, col) dir_map->cells.int32[INDEX(row, col)]
#define SHED(row, col) DIR(row, col)

#ifdef USE_LESS_MEMORY
#define DELINEATE delineate_lessmem
#define GET_DIR(row, col) (DIR(row, col) & 0x7fffffff)
#define SET_NOTDONE(row, col) do { DIR(row, col) |= 0x80000000; } while(0)
#define SET_DONE(row, col) do { DIR(row, col) &= 0x7fffffff; } while(0)
#define IS_NOTDONE(row, col) (DIR(row, col) & 0x80000000)
#define IS_DONE(row, col) !IS_NOTDONE(row, col)
#else
#define DELINEATE delineate_moremem
#define GET_DIR(row, col) DIR(row, col)
#define DONE(row, col) done[INDEX(row, col)]
#define SET_DONE(row, col) do { DONE(row, col) = 1; } while(0)
#define IS_NOTDONE(row, col) !DONE(row, col)
#define IS_DONE(row, col) DONE(row, col)
static char *done;
#endif

/* relative row, col, and direction to the center in order of
 * E S W N SE SW NW NE */
static int nbr_rcd[8][3] = {
    {0, 1, W},
    {1, 0, N},
    {0, -1, E},
    {-1, 0, S},
    {1, 1, NW},
    {1, -1, NE},
    {-1, -1, SE},
    {-1, 1, SW}
};

struct cell
{
    int row;
    int col;
};

struct cell_stack
{
    struct cell *cells;
    int n;
    int nalloc;
};

static int nrows, ncols;

#ifdef LOOP_THEN_TASK
#define TRACE_UP_RETURN int
#else
#define TRACE_UP_RETURN void
#endif

static TRACE_UP_RETURN trace_up(struct raster_map *, int, int, int,
                                struct cell_stack *);
static void init_up_stack(struct cell_stack *);
static void free_up_stack(struct cell_stack *);
static void push_up(struct cell_stack *, struct cell *);
static void pop_up(struct cell_stack *, struct cell *);

void DELINEATE(struct raster_map *dir_map, struct outlet_list *outlet_l)
{
    int i, j;

    nrows = dir_map->nrows;
    ncols = dir_map->ncols;

#ifdef USE_LESS_MEMORY
#pragma omp parallel for schedule(dynamic) private(j)
    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++)
            SET_NOTDONE(i, j);
    }
#else
    done = calloc((size_t)nrows * ncols, sizeof *done);
#endif

#pragma omp parallel for schedule(dynamic)
    for (i = 0; i < outlet_l->n; i++) {
#ifndef USE_LESS_MEMORY
        SET_DONE(outlet_l->row[i], outlet_l->col[i]);
#endif
        SHED(outlet_l->row[i], outlet_l->col[i]) = outlet_l->id[i];
    }

    /* loop through all outlets and delineate the subwatershed for each */
#pragma omp parallel for schedule(dynamic)
    for (i = 0; i < outlet_l->n; i++) {
        struct cell_stack *up_stack = malloc(sizeof *up_stack);

        init_up_stack(up_stack);

        /* trace up flow directions */
#ifdef LOOP_THEN_TASK
        if (
#endif
               trace_up(dir_map, outlet_l->row[i], outlet_l->col[i],
                        outlet_l->id[i], up_stack)
#ifdef LOOP_THEN_TASK
            ) {
#ifdef _MSC_VER
            /* XXX: MSVC needs it! even with the above private(i), i inside
             * tasks is not the same as i from the creator thread
             * (undocumented?); GCC works without this line */
            int I = i;
#else
#define I i
#endif

            do {
                /* trace up from branching cells in the stack */
                while (up_stack->n) {
                    struct cell up;

#pragma omp critical
                    pop_up(up_stack, &up);

#pragma omp task
                    {
                        int row = up.row, col = up.col;
                        struct cell_stack *task_up_stack =
                            malloc(sizeof *task_up_stack);

                        init_up_stack(task_up_stack);

                        /* trace up from a branching node */
                        if (trace_up
                            (dir_map, row, col, outlet_l->id[I],
                             task_up_stack)) {
                            while (task_up_stack->n) {
                                struct cell task_up;

                                pop_up(task_up_stack, &task_up);
#pragma omp critical
                                push_up(up_stack, &task_up);
                            }
                        }

                        free_up_stack(task_up_stack);
                    }
                }
#pragma omp taskwait
            } while (up_stack->n);
        }
#endif
        /* XXX: work around an indent bug
         * #else
         *     ;
         * #endif
         * doesn't work */
        ;

        free_up_stack(up_stack);
    }

    dir_map->null_value = SHED_NULL;
#pragma omp parallel for schedule(dynamic) private(j)
    for (i = 0; i < nrows; i++) {
        for (j = 0; j < ncols; j++)
            if (IS_NOTDONE(i, j))
                SHED(i, j) = SHED_NULL;
    }

#ifndef USE_LESS_MEMORY
    free(done);
#endif
}

static TRACE_UP_RETURN trace_up(struct raster_map *dir_map, int row, int col,
                                int id, struct cell_stack *up_stack)
{
#ifdef DONT_USE_TCO
    do {
#endif
        int i;
        int nup = 0;
        int next_row = -1, next_col = -1;
        struct cell up;

        for (i = 0; i < 8 && nup <= 1; i++) {
            int nbr_row = row + nbr_rcd[i][0];
            int nbr_col = col + nbr_rcd[i][1];

            /* skip edge cells */
            if (nbr_row < 0 || nbr_row >= nrows || nbr_col < 0 ||
                nbr_col >= ncols)
                continue;

            /* if a neighbor cell flows into the current cell, trace up
             * further; we need to check if that neighbor cell has already been
             * processed because we don't want to misinterpret a subwatershed
             * ID as a direction; remember we're overwriting dir_map so it can
             * have both directions and subwatershed IDs */
            if (GET_DIR(nbr_row, nbr_col) == nbr_rcd[i][2] &&
                IS_NOTDONE(nbr_row, nbr_col) && ++nup == 1) {
                /* climb up only to this cell at this time */
                next_row = nbr_row;
                next_col = nbr_col;
#ifndef USE_LESS_MEMORY
                SET_DONE(next_row, next_col);
#endif
                SHED(next_row, next_col) = id;
            }
        }

        if (!nup) {
            /* reached a ridge cell; if there were any up cells to visit, let's go
             * back or simply complete tracing */
            if (!up_stack->n)
                return
#ifdef LOOP_THEN_TASK
                    0
#endif
                    ;

#ifdef LOOP_THEN_TASK
            /* next cell is not popped yet;
             * if current stack size >= tracing stack size */
            if (up_stack->n >= tracing_stack_size)
                return 1;
#endif

            pop_up(up_stack, &up);
            next_row = up.row;
            next_col = up.col;
        }
        else if (nup > 1) {
            /* if there are more up cells to visit, let's come back later */
            up.row = row;
            up.col = col;
            push_up(up_stack, &up);
        }

#ifdef LOOP_THEN_TASK
        /* next cell is not in the stack;
         * if current stack size + next cell >= tracing stack size */
        if (up_stack->n + 1 >= tracing_stack_size) {
            up.row = next_row;
            up.col = next_col;
            push_up(up_stack, &up);

            return 1;
        }
#endif

#ifdef DONT_USE_TCO
        row = next_row;
        col = next_col;
    } while (1);
    /* XXX: work around an indent bug
     * #else
     * doesn't work */
#endif
#ifndef DONT_USE_TCO
    /* use gcc -O2 or -O3 flags for tail-call optimization
     * (-foptimize-sibling-calls) */
#ifdef LOOP_THEN_TASK
    return
#endif
        trace_up(dir_map, next_row, next_col, id, up_stack);
#endif
}

static void init_up_stack(struct cell_stack *up_stack)
{
    up_stack->nalloc = up_stack->n = 0;
    up_stack->cells = NULL;
}

static void free_up_stack(struct cell_stack *up_stack)
{
    if (up_stack->cells)
        free(up_stack->cells);
    init_up_stack(up_stack);
}

static void push_up(struct cell_stack *up_stack, struct cell *up)
{
    if (up_stack->n == up_stack->nalloc) {
        up_stack->nalloc += REALLOC_INCREMENT;
        up_stack->cells =
            realloc(up_stack->cells,
                    sizeof *up_stack->cells * up_stack->nalloc);
    }
    up_stack->cells[up_stack->n++] = *up;
}

static void pop_up(struct cell_stack *up_stack, struct cell *up)
{
    if (up_stack->n > 0) {
        *up = up_stack->cells[--up_stack->n];
        if (up_stack->n == 0)
            free_up_stack(up_stack);
        else if (up_stack->n == up_stack->nalloc - REALLOC_INCREMENT) {
            up_stack->nalloc -= REALLOC_INCREMENT;
            up_stack->cells =
                realloc(up_stack->cells,
                        sizeof *up_stack->cells * up_stack->nalloc);
        }
    }
}
