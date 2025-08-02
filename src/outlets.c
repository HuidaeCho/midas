#include <stdio.h>
#include <stdlib.h>
#include <gdal.h>
#include "midas.h"

#define DIR(row, col) \
        dir_map->cells.int32[(size_t)(row) * dir_map->ncols + (col)]

struct outlet_list *read_outlets(const char *outlets_path,
                                 const char *layer_name, const char *opts,
                                 const char *id_col,
                                 struct raster_map *dir_map,
                                 int store_dir, int lfp_mode)
{
    struct outlet_list *outlet_l = malloc(sizeof *outlet_l);
    const char **ds_opts = NULL;
    GDALDatasetH dataset;
    OGRLayerH layer;
    OGRFeatureDefnH feature_def;
    int id_field_idx;
    OGRFeatureH feature;

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
          GDALOpenEx(outlets_path, GDAL_OF_VECTOR, NULL, ds_opts, NULL)))
        return NULL;

    if (!GDALDatasetGetLayerCount(dataset))
        return NULL;

    if (!(layer =
          layer_name ? GDALDatasetGetLayerByName(dataset, layer_name) :
          GDALDatasetGetLayer(dataset, 0)))
        return NULL;

    init_outlet_list(outlet_l);

    feature_def = OGR_L_GetLayerDefn(layer);
    if (strcmp(id_col, OGR_L_GetFIDColumn(layer)) == 0)
        id_field_idx = -1;
    else {
        OGRFieldDefnH id_field_def;

        if ((id_field_idx = OGR_FD_GetFieldIndex(feature_def, id_col)) < 0)
            return NULL;

        id_field_def = OGR_FD_GetFieldDefn(feature_def, id_field_idx);
        if (OGR_Fld_GetType(id_field_def) != OFTInteger)
            return NULL;
    }

    while ((feature = OGR_L_GetNextFeature(layer))) {
        OGRGeometryH geometry;
        int id;
        double x, y;

        geometry = OGR_F_GetGeometryRef(feature);
        if (geometry &&
            wkbFlatten(OGR_G_GetGeometryType(geometry)) == wkbPoint) {
            int row, col;

            x = OGR_G_GetX(geometry, 0);
            y = OGR_G_GetY(geometry, 0);
            id = id_field_idx >= 0 ?
                OGR_F_GetFieldAsInteger(feature, id_field_idx) :
                OGR_F_GetFID(feature);

            calc_row_col(dir_map, x, y, &row, &col);

            /* if the outlet is outside the computational region, skip */
            if (row >= 0 && row < dir_map->nrows && col >= 0 &&
                col < dir_map->ncols)
                add_outlet(outlet_l, row, col, id,
                           store_dir ? DIR(row, col) : 0, lfp_mode);
            else
                printf
                    ("Skip outlet %d at (%d, %d) outside the current region\n",
                     id, row, col);
        }

        OGR_F_Destroy(feature);
    }

    GDALClose(dataset);

    return outlet_l;
}

int write_outlets(const char *outlets_path, struct outlet_list *outlet_l)
{
    FILE *fp;
    int i;

    if (!(fp = fopen(outlets_path, "w")))
        return 1;

    for (i = 0; i < outlet_l->n; i++)
        fprintf(fp, "%d %d %d\n", outlet_l->row[i], outlet_l->col[i],
                outlet_l->id[i]);

    fclose(fp);

    return 0;
}
