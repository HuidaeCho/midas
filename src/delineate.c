#include "global.h"

void delineate(struct raster_map *dir_map, struct outlet_list *outlet_l,
               int use_lessmem
#ifdef LOOP_THEN_TASK
               , int tracing_stack_size
#endif
    )
{
    if (use_lessmem)
        delineate_lessmem(dir_map, outlet_l
#ifdef LOOP_THEN_TASK
                          , tracing_stack_size
#endif
            );
    else
        delineate_moremem(dir_map, outlet_l
#ifdef LOOP_THEN_TASK
                          , tracing_stack_size
#endif
            );
}
