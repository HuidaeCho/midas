#include <stdio.h>
#include <stdlib.h>
#include "global.h"

#define SHED(row, col) shed_map->cells.int32[(size_t)(row) * ncols + (col)]

struct shed_hier *analyze_shed_hier(struct raster_map *shed_map,
                                    struct outlet_list *outlet_l)
{
    struct shed_hier *hier = malloc(sizeof *hier);
    int nrows = shed_map->nrows, ncols = shed_map->ncols;
    int i;

    hier->n = outlet_l->n;
    hier->self = malloc(sizeof *hier->self * hier->n);
    hier->up = malloc(sizeof *hier->up * hier->n);
    hier->down = malloc(sizeof *hier->down * hier->n);

#pragma omp parallel for schedule(dynamic)
    for (i = 0; i < outlet_l->n; i++) {
        int row = outlet_l->row[i];
        int col = outlet_l->col[i];

        hier->self[i] = outlet_l->id[i];
        switch (outlet_l->dir[i]) {
        case NW:
            row--;
            col--;
            break;
        case N:
            row--;
            break;
        case NE:
            row--;
            col++;
            break;
        case W:
            col--;
            break;
        case E:
            col++;
            break;
        case SW:
            row++;
            col--;
            break;
        case S:
            row++;
            break;
        case SE:
            row++;
            col++;
            break;
        }

        hier->down[i] = row < 0 || row >= nrows || col < 0 || col >= ncols ||
            SHED(row, col) == SHED_NULL ? SHED_HIER_NULL : SHED(row, col);
    }

#pragma omp parallel for schedule(dynamic)
    for (i = 0; i < hier->n; i++) {
        int self = hier->self[i];
        int j;

        for (j = 0; j < hier->n && hier->down[j] != self; j++) ;
        if (j == hier->n)
            hier->up[i] = SHED_HIER_NULL;
        else
            hier->up[i] = hier->self[j];
    }

    return hier;
}

int write_shed_hier(const char *hier_path, struct shed_hier *hier,
                    const char *id_col)
{
    FILE *fp;
    int i;

    if (!(fp = fopen(hier_path, "w")))
        return 1;

    fprintf(fp, "%s,%s_up,%s_down\n", id_col, id_col, id_col);

    for (i = 0; i < hier->n; i++)
        fprintf(fp, "%d,%d,%d\n", hier->self[i], hier->up[i], hier->down[i]);

    fclose(fp);

    return 0;
}
