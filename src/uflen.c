#include "midas.h"

void uflen(struct raster_map *dir_map, struct raster_map *flen_map,
           int use_lessmem, int from_one)
{
    switch (use_lessmem) {
    case 0:
        uflen_moremem(dir_map, flen_map, from_one);
        break;
    case 1:
        uflen_lessmem(dir_map, flen_map, from_one);
        break;
    case 2:
        uflen_leastmem(dir_map, from_one);
        break;
    }
}
