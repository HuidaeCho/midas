#include "global.h"

void accumulate(struct raster_map *dir_map, struct raster_map *accum_map,
                int use_lessmem)
{
    if (use_lessmem)
        accumulate_lessmem(dir_map, accum_map);
    else
        accumulate_moremem(dir_map, accum_map);
}
