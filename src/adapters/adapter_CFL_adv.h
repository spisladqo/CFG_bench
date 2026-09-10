#pragma once

#include "adapter.h"
#include <GraphBLAS.h>
#include <LAGraph.h>

typedef struct {
    int8_t optimizations;
    char *sources_path;
} CFL_adv_PrepareData;

AdapterMethods adapter_CFL_adv_get_methods(void);
AdapterMethods adapter_CFL_multsrc_adv_get_methods(void);