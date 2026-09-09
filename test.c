#include <GraphBLAS.h>
#include <LAGraph.h>
#include <LAGraphX.h>
#include <parser.h>
#include <time.h>
#include <ctype.h>

// #define run_algorithm()                                                        \
//     LAGraph_CFL_reachability(outputs, adj_matrices, grammar.terms_count,       \
//                              grammar.nonterms_count, grammar.rules,            \
//                              grammar.rules_count, msg)

#define run_algorithm()                                                        \
    LAGraph_CFL_reachability_multsrc(outputs, adj_matrices, src, src_count, grammar.terms_count,       \
                             grammar.nonterms_count, grammar.rules,            \
                             grammar.rules_count, msg)

// #define run_algorithm()                                                        \
//     LAGraph_CFL_reachability_multsrc_fast(outputs, adj_matrices, src, src_count, grammar.terms_count,       \
//                              grammar.nonterms_count, grammar.rules,            \
//                              grammar.rules_count, msg, 1 )


#define check_error(error)                                                     \
    {                                                                          \
        retval = run_algorithm();                                              \
        TEST_CHECK(retval == error);                                           \
        TEST_MSG("retval = %d (%s)", retval, msg);                             \
    }

#define check_result(result)                                                   \
    {                                                                          \
        char *expected = output_to_str(0);                                     \
        TEST_CHECK(strcmp(result, expected) == 0);                             \
        TEST_MSG("Wrong result. Actual: %s", expected);                        \
    }

GrB_Matrix *adj_matrices = NULL;
GrB_Matrix *outputs = NULL;
grammar_t grammar = {0, 0, 0, NULL};
char msg[LAGRAPH_MSG_LEN];

void setup() { LAGr_Init(GrB_NONBLOCKING, malloc, NULL, NULL, free, msg); }

void teardown(void) { LAGraph_Finalize(msg); }

void init_outputs() {
    outputs = calloc(grammar.nonterms_count, sizeof(GrB_Matrix));
}

void free_outputs() {
    for (size_t i = 0; i < grammar.nonterms_count; i++) {
        if (outputs == NULL)
            break;

        if (outputs[i] == NULL)
            continue;

        GrB_free(&outputs[i]);
    }
    free(outputs);
    outputs = NULL;
}

void free_workspace() {

    for (size_t i = 0; i < grammar.terms_count; i++) {
        if (adj_matrices == NULL)
            break;

        if (adj_matrices[i] == NULL)
            continue;

        GrB_free(&adj_matrices[i]);
    }
    free(adj_matrices);
    adj_matrices = NULL;

    free_outputs();

    free(grammar.rules);
    grammar = (grammar_t){0, 0, 0, NULL};
}

char *configs_rdf[] = {
                       "data/graphs/rdf/go_hierarchy.g,old_data/grammars/"
                       "nested_parentheses_subClassOf_type.cnf",
                    //    "data/graphs/rdf/taxonomy.g,old_data/grammars/"
                    //    "nested_parentheses_subClassOf_type.cnf",
                    //    "data/graphs/rdf/eclass.g,old_data/grammars/"
                    //    "nested_parentheses_subClassOf_type.cnf",
                       "data/graphs/rdf/go.g,old_data/grammars/"
                       "nested_parentheses_subClassOf_type.cnf",
                       "data/graphs/rdf/taxonomy_hierarchy.g,old_data/grammars/"
                       "nested_parentheses_subClassOf_type.cnf",
                       NULL};

char *configs_java[] = {
    "data/graphs/java/avrora.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/batik.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/commons_io.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/commons_lang3.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/eclipse.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/fop.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/gson.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/h2.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/luindex.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/mockito.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/pmd.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/sunflow.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/xalan.g,old_data/grammars/java_points_to.cnf",
    "data/graphs/java/tomcat.g,old_data/grammars/java_points_to.cnf",
    NULL};

char *configs_c_alias[] = {
    "data/graphs/c_alias/init.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/arch.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/block.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/crypto.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/drivers.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/mm.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/ipc.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/lib.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/security.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/sound.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/fs.g,old_data/grammars/c_alias.cnf",
    "data/graphs/c_alias/net.g,old_data/grammars/c_alias.cnf",
    NULL};

char *configs_vf[] = {"data/graphs/vf/xz.g,old_data/grammars/vf.cnf",
                      "data/graphs/vf/nab.g,old_data/grammars/vf.cnf",
                      "data/graphs/vf/leela.g,old_data/grammars/vf.cnf", NULL};

char *configs_my[] = {"data/graphs/vf/xz.g,old_data/grammars/vf.cnf", NULL};

char *configs_all[] = {
    "data/graphs/rdf/go_hierarchy.g,old_data/grammars/"
    "nested_parentheses_subClassOf_type.cnf",
    "data/graphs/rdf/go.g,old_data/grammars/"
    "nested_parentheses_subClassOf_type.cnf",
    "data/graphs/rdf/taxonomy_hierarchy.g,old_data/grammars/"
    "nested_parentheses_subClassOf_type.cnf",

    // "data/graphs/java/avrora.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/batik.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/commons_io.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/commons_lang3.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/eclipse.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/fop.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/gson.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/h2.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/luindex.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/mockito.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/pmd.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/sunflow.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/xalan.g,old_data/grammars/java_points_to.cnf",
    // "data/graphs/java/tomcat.g,old_data/grammars/java_points_to.cnf",

    // "data/graphs/c_alias/init.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/arch.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/block.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/crypto.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/drivers.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/mm.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/ipc.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/lib.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/security.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/sound.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/fs.g,old_data/grammars/c_alias.cnf",
    // "data/graphs/c_alias/net.g,old_data/grammars/c_alias.cnf",
    NULL
};

// Number of benchmark runs on a single graph
#define COUNT 20
// If true, the first run is done without measuring time (warm-up)
#define HOT false
// Use your custom configuration for the benchmark (default is the xz.g graph
// and vf.cnf grammar)
#define configs configs_all

GrB_Index* load_array_stream(const char *filename, int *count) {
    FILE *file = fopen(filename, "r");
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
    GrB_Index *array = malloc(*count * sizeof(GrB_Index));
    if (array == NULL) {
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
                array[index++] = atoi(number_buffer);
                buf_index = 0;
                in_number = 0;
            }
        }
    }

    // Handle last number if file ends with digit
    if (in_number && index < *count) {
        number_buffer[buf_index] = '\0';
        array[index++] = atoi(number_buffer);
    }

    fclose(file);
    
    // This should now equal the actual count
    if (index != *count) {
        free(array);
        *count = 0;
        return NULL;
    }
    
    return array;
}

void print_grammar(grammar_t grammar) {
    for (int i = 0; i < grammar.rules_count; i++) {
        LAGraph_rule_WCNF rule = grammar.rules[i];
        printf("rule %d: %d -> %d %d (%d)\n", i, rule.nonterm, rule.prod_A, rule.prod_B, rule.index);
    }
}

int main(int argc, char **argv) {
    setup();
    GrB_Info retval;

    // char *config = argv[1];
    printf("Start bench\n");
    fflush(stdout);
    int config_index = 0;
    char *config = configs[config_index];
    while (config) {
        printf("CONFIG: %s\n", config);
        fflush(stdout);
        parser(config, &grammar, &adj_matrices);

        double start[COUNT];
        double end[COUNT];

        // if (HOT) {
        //     run_algorithm();
        // }

        int src_count;
        char *src_path;
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


        GrB_Index* src = load_array_stream(src_path, &src_count);
        printf("src path:        %s\n", src_path);
        printf("src_count:       %d\n", src_count);
        printf("outpus:          %p\n", outputs);
        printf("adj_matrices:    %p\n", adj_matrices);
        printf("terms_count:     %d\n", grammar.terms_count);
        printf("nonterms_count:  %d\n", grammar.nonterms_count);
        printf("rules:           %p\n", grammar.rules);
        printf("rules_count:     %d\n", grammar.rules_count);
        // print_grammar(grammar);

        GrB_Index nnz = 0;
        for (size_t i = 0; i < COUNT; i++) {
            init_outputs();

            start[i] = LAGraph_WallClockTime();
#ifndef CI
            retval = run_algorithm();
#endif
            end[i] = LAGraph_WallClockTime();

            GrB_Matrix_nvals(&nnz, outputs[0]);
            printf("\t%.3fs", end[i] - start[i]);
            fflush(stdout);
            free_outputs();
        }
        printf("\n");

        // printf("retval = %d (%s)\n", retval, msg);
        double sum = 0;
        for (size_t i = 0; i < COUNT; i++) {
            sum += end[i] - start[i];
        }

        printf("\tTime elapsed (avg): %.6f seconds. Result: %ld (return code "
               "%d) (%s)\n\n",
               sum / COUNT, nnz, retval, msg);
        // GxB_print(outputs[0], 1);
        free_workspace();
        config = configs[++config_index];
        fflush(stdout);
    }

    teardown();
}
