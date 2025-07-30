#include <stdio.h>
#include <stdlib.h>
#include "global.h"

void init_point_list(struct point_list *pl)
{
    pl->nalloc = pl->n = 0;
    pl->row = pl->col = NULL;
}

void reset_point_list(struct point_list *pl)
{
    pl->n = 0;
}

void free_point_list(struct point_list *pl)
{
    if (pl->row)
        free(pl->row);
    if (pl->col)
        free(pl->col);
    init_point_list(pl);
}

/* adapted from r.path */
void add_point(struct point_list *pl, int row, int col)
{
    if (pl->n == pl->nalloc) {
        pl->nalloc += REALLOC_INCREMENT;
        pl->row = realloc(pl->row, sizeof *pl->row * pl->nalloc);
        pl->col = realloc(pl->col, sizeof *pl->col * pl->nalloc);
        if (!pl->row || !pl->col) {
            fprintf(stderr, "Unable to increase point list");
            exit(EXIT_FAILURE);
        }
    }
    pl->row[pl->n] = row;
    pl->col[pl->n] = col;
    pl->n++;
}
