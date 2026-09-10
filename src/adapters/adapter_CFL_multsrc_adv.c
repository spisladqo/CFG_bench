#include "adapter_CFL_adv.h"
#include "GraphBLAS.h"
#include "LAGraph.h"
#include "../parser.h"
#include <ctype.h>

#define NTHREADS 1

#define TRY(GrB_method)                                                                                                \
    {                                                                                                                  \
        GrB_Info LG_GrB_Info = GrB_method;                                                                             \
        if (LG_GrB_Info < GrB_SUCCESS) {                                                                               \
            fprintf(stderr, "LAGraph failure (file %s, line %d): (%d, msg: %s) \n", __FILE__, __LINE__, LG_GrB_Info,     \
                    state.msg);                                                                                        \
            return (LG_GrB_Info);                                                                                      \
        }                                                                                                              \
    }

typedef struct {
    GrB_Matrix *matrices;
    size_t matrices_count;
} AdapterResult;

// inner state of the adapter
typedef struct {
    GrB_Matrix *adj_matrices;
    // GrB_Matrix *outputs;
    GrB_Matrix output;
    GrB_Index *src;
    size_t src_count;
    grammar_t grammar;
    char msg[LAGRAPH_MSG_LEN];
    size_t symbols_amount;
    int8_t optimizations;
} state_t;

static state_t state;

// helper structure to accumulate data for building matrices
typedef struct {
    size_t *rows;
    size_t *cols;
    size_t *indeces;
    size_t size;
    size_t capacity;
} SymbolData;

static void symbol_data_init(SymbolData *data) {
    data->rows = NULL;
    data->cols = NULL;
    data->indeces = NULL;
    data->size = 0;
    data->capacity = 0;
}

static void symbol_data_expand(SymbolData *data) {
    size_t new_capacity = data->capacity == 0 ? 10 : data->capacity * 2;
    data->rows = realloc(data->rows, new_capacity * sizeof(size_t));
    data->cols = realloc(data->cols, new_capacity * sizeof(size_t));
    data->indeces = realloc(data->indeces, new_capacity * sizeof(size_t));
    data->capacity = new_capacity;
}

static void symbol_data_free(SymbolData *data) {
    free(data->rows);
    free(data->cols);
    free(data->indeces);
}

static GrB_Matrix *get_matrices_from_graph(Graph graph, size_t *map_base_indecies, size_t symbols_amount) {
    SymbolData *symbol_datas = calloc(symbols_amount, sizeof(SymbolData));
    for (size_t i = 0; i < symbols_amount; i++) {
        symbol_data_init(&symbol_datas[i]);
    }

    for (size_t i = 0; i < graph.edge_count; i++) {
        GraphEdge edge = graph.edges[i];
        SymbolData *data = &symbol_datas[map_base_indecies[edge.term_index] + edge.index];

        if (data->size == data->capacity) {
            symbol_data_expand(data);
        }

        data->rows[data->size] = edge.u;
        data->cols[data->size] = edge.v;
        data->indeces[data->size] = edge.index;
        data->size++;
    }

    GrB_Matrix *matrices = malloc(sizeof(GrB_Matrix) * symbols_amount);
    for (size_t i = 0; i < symbols_amount; i++) {
        SymbolData data = symbol_datas[i];
        GrB_Index nrows = graph.node_count;

        char msg[LAGRAPH_MSG_LEN];
        MY_GRB_TRY(GrB_Matrix_new(&matrices[i], GrB_BOOL, nrows, graph.node_count));

        if (data.size == 0) {
            continue;
        }

        GrB_Scalar true_scalar;
        MY_GRB_TRY(GrB_Scalar_new(&true_scalar, GrB_BOOL));
        MY_GRB_TRY(GrB_Scalar_setElement_BOOL(true_scalar, true));
        MY_GRB_TRY(GxB_Matrix_build_Scalar(matrices[i], data.rows, data.cols, true_scalar, data.size));
        MY_GRB_TRY(GrB_free(&true_scalar));
    }

    for (size_t i = 0; i < symbols_amount; i++) {
        symbol_data_free(&symbol_datas[i]);
    }

    free(symbol_datas);

    return matrices;
}

// initialize LAGraph\GraphBLAS
static GrB_Info adapter_CFL_multsrc_adv_setup() {
    int nthreads = NTHREADS;

    TRY(LAGr_Init(GrB_NONBLOCKING, malloc, NULL, NULL, free, state.msg));

    // LAGraph_SetNumThreads(nthreads, nthreads, NULL); 
    // printf("LAGraph configured to utilize %d threads.\n", nthreads);

    return GrB_SUCCESS;
}

// given name and pointer to count, fills count and loads src, returning pointer
// to an array of src
static GrB_Index* load_src(const char *config, size_t *count) {
    char *src_path = NULL;
    if (strstr(config, "init.g") != NULL) {
        // printf("graph: init\n");
        src_path = "../CFPQ_PyAlgo/experiments/samples/init_smpl_5%_2.txt";
    }
    if (strstr(config, "mm.g") != NULL) {
        // printf("graph: mm\n");
        src_path = "../CFPQ_PyAlgo/experiments/samples/mm_smpl_5%_2.txt";
    }
    if (strstr(config, "ipc.g") != NULL) {
        // printf("graph: ipc\n");
        src_path = "../CFPQ_PyAlgo/experiments/samples/ipc_smpl_5%_2.txt";
    }
    if (strstr(config, "lib.g") != NULL) {
        // printf("graph: lib\n");
        src_path = "../CFPQ_PyAlgo/experiments/samples/lib_smpl_5%_2.txt";
    }
    if (strstr(config, "block.g") != NULL) {
        // printf("graph: block\n");
        src_path = "../CFPQ_PyAlgo/experiments/samples/block_smpl_5%_2.txt";
    }
    if (strstr(config, "arch.g") != NULL) {
        // printf("graph: arch\n");
        src_path = "../CFPQ_PyAlgo/experiments/samples/arch_smpl_5%_2.txt";
    }
    if (strstr(config, "crypto.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/crypto_smpl_5%_2.txt";
    }
    if (strstr(config, "security.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/security_smpl_5%_2.txt";
    }
    if (strstr(config, "sound.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/sound_smpl_5%_2.txt";
    }
    if (strstr(config, "fs.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/fs_smpl_5%_2.txt";
    }
    if (strstr(config, "net.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/net_smpl_5%_2.txt";
    }
    if (strstr(config, "drivers.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/drivers_smpl_5%_2.txt";
    }
    if (strstr(config, "go_hierarchy.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/go_hierarchy_smpl_5%_2.txt";
    }
    if (strstr(config, "eclass.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/eclass_smpl_5%_2.txt";
    }
    if (strstr(config, "go.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/go_smpl_5%_2.txt";
    }
    if (strstr(config, "taxonomy.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/taxonomy_smpl_5%_2.txt";
    }
    if (strstr(config, "taxonomy_hierarchy.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/taxonomy_hierarchy_smpl_5%_2.txt";
    }
    if (strstr(config, "avrora.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/avrora_smpl_5%_2.txt";
    }
    if (strstr(config, "batik.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/batik_smpl_5%_2.txt";
    }
    if (strstr(config, "commons_io.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/commons_io_smpl_5%_2.txt";
    }
    if (strstr(config, "commons_lang3.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/commons_lang3_smpl_5%_2.txt";
    }
    if (strstr(config, "eclipse.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/eclipse_smpl_5%_2.txt";
    }
    if (strstr(config, "fop.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/fop_smpl_5%_2.txt";
    }
    if (strstr(config, "gson.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/gson_smpl_5%_2.txt";
    }
    if (strstr(config, "h2.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/h2_smpl_5%_2.txt";
    }
    if (strstr(config, "luindex.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/luindex_smpl_5%_2.txt";
    }
    if (strstr(config, "mockito.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/mockito_smpl_5%_2.txt";
    }
    if (strstr(config, "pmd.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/pmd_smpl_5%_2.txt";
    }
    if (strstr(config, "sunflow.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/sunflow_smpl_5%_2.txt";
    }
    if (strstr(config, "tomcat.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/tomcat_smpl_5%_2.txt";
    }
    if (strstr(config, "tradebeans.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/tradebeans_smpl_5%_2.txt";
    }
    if (strstr(config, "tradesoap.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/tradesoap_smpl_5%_2.txt";
    }
    if (strstr(config, "xalan.g") != NULL) {
        src_path = "../CFPQ_PyAlgo/experiments/samples/xalan_smpl_5%_2.txt";
    }
    printf("src path: %s\n", src_path);
    FILE *file = fopen(src_path, "r");
    if (file == NULL) {
        return NULL;
    }

    // First pass: count numbers by streaming
    *count = 0;
    int ch;
    int in_number = 0;
    
    while ((ch = fgetc(file)) != EOF) {
        if (isdigit(ch)) {
            if (!in_number) {
                (*count)++;
                in_number = 1;
            }
        } else {
            in_number = 0;
        }
    }

    if (*count == 0) {
        fclose(file);
        return NULL;
    }

    // Allocate array
    GrB_Index *src = malloc(*count * sizeof(GrB_Index));
    if (src == NULL) {
        fclose(file);
        return NULL;
    }

    // Second pass: read numbers properly
    rewind(file);

    int index = 0;
    in_number = 0;
    char number_buffer[32];
    int buf_index = 0;
    
    while ((ch = fgetc(file)) != EOF && index < *count) {
        if (isdigit(ch)) {
            if (buf_index < sizeof(number_buffer) - 1) {
                number_buffer[buf_index++] = (char)ch;
            }
            in_number = 1;
        } else {
            if (in_number) {
                number_buffer[buf_index] = '\0';
                src[index++] = atoi(number_buffer);
                buf_index = 0;
                in_number = 0;
            }
        }
    }

    // Handle last number if file ends with digit
    if (in_number && index < *count) {
        number_buffer[buf_index] = '\0';
        src[index++] = atoi(number_buffer);
    }

    fclose(file);

    // This should now equal the actual count
    if (index != *count) {
        free(src);
        *count = 0;
        return NULL;
    }

    return src;
}

// inner structure to pass data to prepare function
typedef CFL_adv_PrepareData PrepareData;

// prepare the adapter for use with the given parser result
// this may include converting the grammar and graph into a format suitable for the algorithm
//
// this modify ther inner state of the adapter
//
// adapter_CFL_adv_prepare should be called just once for each config
static GrB_Info adapter_CFL_multsrc_adv_prepare(ParserResult parser_result, void *prepare_data) {
    PrepareData *data = (PrepareData *)prepare_data;
    int8_t optimizations = data->optimizations;
    Grammar grammar = parser_result.grammar;
    Graph graph = parser_result.graph;
    SymbolList list = parser_result.symbols;
    size_t src_count = 0;
    // const char *sources_path = ((CFL_adv_PrepareData*)prepare_data)->sources_path;
    GrB_Index *src = load_src(graph.path, &src_count);
    printf("src: %p\n", src);
    printf("src_count: %lu\n", src_count);

    // indexed symbols must be each enumerate
    size_t *map = calloc(list.count * graph.block_count, sizeof(size_t));
    size_t offset = 0;
    for (size_t i = 0; i < list.count; i++) {
        Symbol sym = list.symbols[i];
        if (!sym.is_indexed) {
            map[i] = i + offset;
            continue;
        }

        map[i] = i + offset;
        offset += graph.block_count - 1;
    }
    state.symbols_amount = list.count + offset;

    GrB_Matrix *matrices = get_matrices_from_graph(graph, map, state.symbols_amount);


    LAGraph_rule_EWCNF *rules_EWCNF = calloc(grammar.rules_count, sizeof(LAGraph_rule_EWCNF));
    for (size_t i = 0; i < grammar.rules_count; i++) {
        Rule rule = grammar.rules[i];

        rules_EWCNF[i] = (LAGraph_rule_EWCNF){.nonterm = rule.first == -1 ? -1 : (int32_t)map[rule.first],
                                              .prod_A = rule.second == -1 ? -1 : (int32_t)map[rule.second],
                                              .prod_B = rule.third == -1 ? -1 : (int32_t)map[rule.third],
                                              .indexed = 0,
                                              .indexed_count = 0};
        if (rule.first != -1 && list.symbols[rule.first].is_indexed) {
            rules_EWCNF[i].indexed |= LAGraph_EWNCF_INDEX_NONTERM;
        }
        if (rule.second != -1 && list.symbols[rule.second].is_indexed)
            rules_EWCNF[i].indexed |= LAGraph_EWNCF_INDEX_PROD_A;
        if (rule.third != -1 && list.symbols[rule.third].is_indexed)
            rules_EWCNF[i].indexed |= LAGraph_EWNCF_INDEX_PROD_B;
        if (rules_EWCNF[i].indexed != 0) {
            rules_EWCNF[i].indexed_count = graph.block_count;
        }
    }
    free(map);

    int32_t nonterms_count = 0;
    for (size_t i = 0; i < list.count; i++) {
        if (list.symbols[i].is_nonterm)
            nonterms_count++;
    }

    state.adj_matrices = matrices;
    state.grammar =
        (grammar_t){.nonterms_count = nonterms_count, .rules_count = grammar.rules_count, .rules = rules_EWCNF};
    state.optimizations = optimizations;
    state.src = src;
    state.src_count = src_count;

    free(graph.edges);
    free(grammar.rules);
    for (size_t i = 0; i < list.count; i++) {
        free(list.symbols[i].label);
    }
    free(list.symbols);
    free(graph.path);

    return GrB_SUCCESS;
}

// initialize output matrices
//
// this should be called before each run of the algorithm
static GrB_Info adapter_CFL_multsrc_adv_init_outputs() {
    // TRY(LAGraph_Calloc((void **)&state.outputs, 1, sizeof(GrB_Matrix), state.msg));
    return GrB_SUCCESS;
}

#include <LAGraphX.h>

void print_grammar(grammar_t grammar) {
    for (int i = 0; i < grammar.rules_count; i++) {
        LAGraph_rule_EWCNF rule = grammar.rules[i];
        printf("rule %d: %d -> %d %d (%u, %u)\n", i, rule.nonterm, rule.prod_A, rule.prod_B, rule.indexed, rule.indexed_count);
    }
}

int count = 0;
// run the algorithm
//
// this should be called after adapter_CFL_adv_init_outputs
static GrB_Info adapter_CFL_multsrc_adv_run() {
    if (count++ == 0) {
        printf("src_count:       %d\n", state.src_count);
        printf("output:          %p\n", state.output);
        printf("adj_matrices:    %p\n", state.adj_matrices);
        printf("nonterms_count:  %d\n", state.grammar.nonterms_count);
        printf("rules:           %p\n", state.grammar.rules);
        printf("rules_count:     %d\n", state.grammar.rules_count);
        printf("symbols_amount:  %d\n", state.symbols_amount);
    }

    // print_grammar(state.grammar);

    TRY(LAGraph_CFL_reachability_multsrc_adv(&state.output, state.adj_matrices,
            state.src, state.src_count, state.symbols_amount, state.grammar.rules,
            state.grammar.rules_count, state.msg, state.optimizations));

    return GrB_SUCCESS;
}

// check if the result of the algorithm is valid
//
// TODO: now check only count of reachibility pairs, make this more generic for other adapters
static bool adapter_CFL_multsrc_adv_is_result_valid(size_t valid_result) {
    GrB_Index nnz = 0;
    // TRY(GrB_Matrix_nvals(&nnz, state.outputs[0]));
    TRY(GrB_Matrix_nvals(&nnz, state.output));
    return nnz == valid_result;
}

// TODO: now this is the same as adapter_CFL_adv_is_result_valid, make this more generic for other adapters
static size_t adapter_CFL_multsrc_adv_get_result() {
    // GrB_Index nnz = 0;
    GrB_Index nvals = 0;
    TRY(GrB_Matrix_nvals(&nvals, state.output));
    return nvals;
}

// free output matrices
//
// this should be called after each run of the algorithm
static GrB_Info adapter_CFL_multsrc_adv_free_outputs() {
    // if (state.outputs == NULL) {
    //     return GrB_SUCCESS;
    // }

    // if (state.outputs[0] != NULL) {
    //     TRY(GrB_free(&state.outputs[0]));
    // }
    if (state.output != NULL) {
        TRY(GrB_free(&state.output));
    }
    // TRY(LAGraph_Free((void **)&state.outputs, state.msg));

    return GrB_SUCCESS;
}

// free all resources allocated by the adapter except outputs
//
// this should be called after all runs of the algorithm for the given config
static GrB_Info adapter_CFL_multsrc_adv_cleanup() {
    for (size_t i = 0; i < state.symbols_amount; i++) {
        if (state.adj_matrices[i] == NULL)
            continue;

        TRY(GrB_free(&state.adj_matrices[i]));
    }
    free(state.adj_matrices);
    free(state.src);
    free(state.grammar.rules);

    return GrB_SUCCESS;
}

// free LAGraph\GraphBLAS resources
static GrB_Info adapter_CFL_multsrc_adv_teardown() {
    TRY(LAGraph_Finalize(state.msg));
    return GrB_SUCCESS;
}

// get the methods of the adapter
AdapterMethods adapter_CFL_multsrc_adv_get_methods() {
    AdapterMethods methods = {.setup = adapter_CFL_multsrc_adv_setup,
                              .teardown = adapter_CFL_multsrc_adv_teardown,
                              .init_outputs = adapter_CFL_multsrc_adv_init_outputs,
                              .free_outputs = adapter_CFL_multsrc_adv_free_outputs,
                              .run = adapter_CFL_multsrc_adv_run,
                              .prepare = adapter_CFL_multsrc_adv_prepare,
                              .cleanup = adapter_CFL_multsrc_adv_cleanup,
                              .is_result_valid = adapter_CFL_multsrc_adv_is_result_valid,
                              .get_result = adapter_CFL_multsrc_adv_get_result};
    return methods;
}