#include "global.h"

void lfp(struct raster_map *dir_map, struct outlet_list *outlet_l,
         int find_full, int use_lessmem
#ifdef LOOP_THEN_TASK
         , int tracing_stack_size
#endif
    )
{
    switch (use_lessmem) {
    case 0:
        lfp_moremem(dir_map, outlet_l, find_full
#ifdef LOOP_THEN_TASK
                    , tracing_stack_size
#endif
            );
        break;
    case 1:
        lfp_lessmem(dir_map, outlet_l, find_full, 0
#ifdef LOOP_THEN_TASK
                    , tracing_stack_size
#endif
            );
        break;
    case 2:
        lfp_lessmem(dir_map, outlet_l, find_full, 1
#ifdef LOOP_THEN_TASK
                    , tracing_stack_size
#endif
            );
        break;
    }
}
