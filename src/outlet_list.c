#include <stdio.h>
#include <stdlib.h>
#include "midas.h"

void init_outlet_list(struct outlet_list *ol)
{
    ol->nalloc = ol->n = 0;
    ol->row = ol->col = NULL;
    ol->id = NULL;

    /* MESHED only */
    ol->dir = NULL;

    /* MELFP only */
    ol->northo = ol->ndia = NULL;
    ol->lflen = NULL;
    ol->head_pl = NULL;
    ol->has_up = NULL;
    ol->down = NULL;
    ol->flen = NULL;
}

void reset_outlet_list(struct outlet_list *ol)
{
    ol->n = 0;
}

void free_outlet_list(struct outlet_list *ol)
{
    int i;

    if (ol->row)
        free(ol->row);
    if (ol->col)
        free(ol->col);
    if (ol->id)
        free(ol->id);

    /* MESHED only */
    if (ol->dir)
        free(ol->dir);

    /* MELFP only */
    if (ol->northo)
        free(ol->northo);
    if (ol->ndia)
        free(ol->ndia);
    if (ol->lflen)
        free(ol->lflen);
    if (ol->head_pl) {
        for (i = 0; i < ol->n; i++)
            free_point_list(&ol->head_pl[i]);
        free(ol->head_pl);
    }
    if (ol->has_up)
        free(ol->has_up);
    if (ol->down)
        free(ol->down);
    if (ol->flen)
        free(ol->flen);

    init_outlet_list(ol);
}

void add_outlet(struct outlet_list *ol, int row, int col, int id,
                unsigned char dir, int lfp_mode)
{
    if (ol->n == ol->nalloc) {
        ol->nalloc += REALLOC_INCREMENT;
        ol->row = realloc(ol->row, sizeof *ol->row * ol->nalloc);
        ol->col = realloc(ol->col, sizeof *ol->col * ol->nalloc);
        ol->id = realloc(ol->id, sizeof *ol->id * ol->nalloc);

        if (dir)
            /* MESHED only */
            ol->dir = realloc(ol->dir, sizeof *ol->dir * ol->nalloc);

        if (lfp_mode) {
            /* MELFP only */
            ol->northo = realloc(ol->northo, sizeof *ol->northo * ol->nalloc);
            ol->ndia = realloc(ol->ndia, sizeof *ol->ndia * ol->nalloc);
            ol->lflen = realloc(ol->lflen, sizeof *ol->lflen * ol->nalloc);
            ol->head_pl =
                realloc(ol->head_pl, sizeof *ol->head_pl * ol->nalloc);
            if (lfp_mode == 2) {
                ol->has_up =
                    realloc(ol->has_up, sizeof *ol->has_up * ol->nalloc);
                ol->down = realloc(ol->down, sizeof *ol->down * ol->nalloc);
                ol->flen = realloc(ol->flen, sizeof *ol->flen * ol->nalloc);
            }
        }

        if (!ol->row || !ol->col || !ol->id ||
            (dir && !ol->dir) ||
            (lfp_mode &&
             (!ol->lflen || !ol->head_pl ||
              (lfp_mode == 2 && (!ol->has_up || !ol->down || !ol->flen))))) {
            fprintf(stderr, "Unable to increase outlet list");
            exit(EXIT_FAILURE);
        }
    }
    ol->row[ol->n] = row;
    ol->col[ol->n] = col;
    ol->id[ol->n] = id;

    if (dir)
        /* MESHED only */
        ol->dir[ol->n] = dir;

    if (lfp_mode) {
        /* MELFP only */
        ol->northo[ol->n] = 0;
        ol->ndia[ol->n] = 0;
        ol->lflen[ol->n] = 0;
        init_point_list(&ol->head_pl[ol->n]);
        if (lfp_mode == 2) {
            ol->has_up[ol->n] = 0;
            ol->down[ol->n] = -1;
            ol->flen[ol->n] = 0;
        }
    }

    ol->n++;
}
