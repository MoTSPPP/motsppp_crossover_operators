#pragma once

#include <cstdlib>

struct Instance;
struct Solution;
struct BoundedParetoSet;

enum MutationMode {
    MUTATION_CURRENT = 0,
    MUTATION_SEQUENTIAL = 1,
    MUTATION_PARAMETERIZED = 2,
    MUTATION_NONE = 3,
    // Funcoes de mutacao isoladas (fase 2 / fase 4 do estudo de operadores)
    MUTATION_ONLY_SWAP = 4,
    MUTATION_ONLY_REMOVE = 5,
    MUTATION_ONLY_ADD = 6,
    MUTATION_ONLY_BONUS = 7
};

struct MutationConfig {
    MutationMode mode = MUTATION_CURRENT;
    int prob_swap = 50;
    int prob_remove = 50;
    int prob_add = 50;
    int prob_bonus = 50;
};

// Contadores de viabilidade dos operadores de variacao (por teste / por cenario).
// Crossover, random_init e mutation sao contagens agregadas. Os 4 atomicos
// (swap/remove/add/bonus) so sao incrementados quando o modo aplica uma chamada
// individual (SEQUENTIAL, PARAMETERIZED, ONLY_*). No modo CURRENT a viabilidade
// fica apenas no agregado `mutation_*` porque uma unica avaliacao cobre
// routes+bonus juntos.
struct OperatorStats {
    int crossover_calls = 0,   crossover_feasible = 0;
    int random_init_calls = 0, random_init_feasible = 0;
    int mutation_calls = 0,    mutation_feasible = 0;
    int swap_calls = 0,   swap_feasible = 0;
    int remove_calls = 0, remove_feasible = 0;
    int add_calls = 0,    add_feasible = 0;
    int bonus_calls = 0,  bonus_feasible = 0;

    void add(const OperatorStats& o) {
        crossover_calls    += o.crossover_calls;    crossover_feasible    += o.crossover_feasible;
        random_init_calls  += o.random_init_calls;  random_init_feasible  += o.random_init_feasible;
        mutation_calls     += o.mutation_calls;     mutation_feasible     += o.mutation_feasible;
        swap_calls   += o.swap_calls;   swap_feasible   += o.swap_feasible;
        remove_calls += o.remove_calls; remove_feasible += o.remove_feasible;
        add_calls    += o.add_calls;    add_feasible    += o.add_feasible;
        bonus_calls  += o.bonus_calls;  bonus_feasible  += o.bonus_feasible;
    }
};

// Tipos atomicos de mutacao (passados para apply_mutation_strategy para
// rastrear a viabilidade por tipo). -1 = nao rastrear (modo CURRENT).
enum MutationAtomicType {
    MUT_ATOMIC_NONE = -1,
    MUT_ATOMIC_SWAP = 0,
    MUT_ATOMIC_REMOVE = 1,
    MUT_ATOMIC_ADD = 2,
    MUT_ATOMIC_BONUS = 3
};

// Applies the configured mutation strategy to a solution.
// Each mutation step is followed by able_passengers + update_objectives + archive check.
// Returns the number of valuations consumed. Stats opcional (nullptr para nao contar).
int apply_mutation_strategy(
    Instance& instance,
    Solution& solution,
    BoundedParetoSet* EP,
    const MutationConfig& config,
    OperatorStats* stats = nullptr
);
