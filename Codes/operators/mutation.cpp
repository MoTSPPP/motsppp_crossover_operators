#ifndef MUTATION_CPP
#define MUTATION_CPP

#include "mutation.h"
#include "../utils/solution_utils.h"
#include "../moead/BoundedParetoSet.cpp"

// Helper: evaluate after mutation, contabilizar viabilidade. Retorna 1 valuation.
// `atomic_type` indica qual mutacao atomica foi aplicada (MUT_ATOMIC_*) ou
// MUT_ATOMIC_NONE para nao desagregar (modo CURRENT). `stats` pode ser nullptr.
static int evaluate_after_mutation(Instance& instance, Solution& solution,
                                   BoundedParetoSet* EP, OperatorStats* stats,
                                   int atomic_type) {
    able_passengers(instance, solution);
    update_objectives(instance, solution);
    bool feasible = solution_validity(instance, solution);
    if (feasible) {
        EP->adicionarSol(&solution);
    }
    if (stats) {
        stats->mutation_calls++;
        if (feasible) stats->mutation_feasible++;
        switch (atomic_type) {
            case MUT_ATOMIC_SWAP:
                stats->swap_calls++;
                if (feasible) stats->swap_feasible++;
                break;
            case MUT_ATOMIC_REMOVE:
                stats->remove_calls++;
                if (feasible) stats->remove_feasible++;
                break;
            case MUT_ATOMIC_ADD:
                stats->add_calls++;
                if (feasible) stats->add_feasible++;
                break;
            case MUT_ATOMIC_BONUS:
                stats->bonus_calls++;
                if (feasible) stats->bonus_feasible++;
                break;
            case MUT_ATOMIC_NONE:
            default:
                break;
        }
    }
    return 1;
}

int apply_mutation_strategy(
    Instance& instance,
    Solution& solution,
    BoundedParetoSet* EP,
    const MutationConfig& config,
    OperatorStats* stats
) {
    int valuations = 0;

    switch (config.mode) {
        case MUTATION_CURRENT: {
            // routes(3) + bonuses compartilham UMA avaliacao -> nao desagrega tipo
            mutate_routes(instance, solution, 3);
            mutate_bonuses(instance, solution);
            valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_NONE);
            break;
        }
        case MUTATION_SEQUENTIAL: {
            mutate_routes(instance, solution, 0);
            valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_SWAP);

            mutate_routes(instance, solution, 1);
            valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_REMOVE);

            mutate_routes(instance, solution, 2);
            valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_ADD);

            mutate_bonuses(instance, solution);
            valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_BONUS);
            break;
        }
        case MUTATION_PARAMETERIZED: {
            // Roleta ponderada mutuamente exclusiva: UM sorteio escolhe UM UNICO
            // operador entre os 4, com probabilidade proporcional ao peso de cada
            // um. prob_swap+prob_remove+prob_add+prob_bonus devem somar 100 (o
            // espaco de busca do irace ja garante isso por construcao -- ver
            // irace/parameters.txt), entao os pesos sao lidos diretamente como
            // percentuais. r cai em exatamente uma das 4 faixas cumulativas.
            int total = config.prob_swap + config.prob_remove + config.prob_add + config.prob_bonus;
            if (total > 0) {
                int r = rand() % total;
                if (r < config.prob_swap) {
                    mutate_routes(instance, solution, 0);
                    valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_SWAP);
                } else if (r < config.prob_swap + config.prob_remove) {
                    mutate_routes(instance, solution, 1);
                    valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_REMOVE);
                } else if (r < config.prob_swap + config.prob_remove + config.prob_add) {
                    mutate_routes(instance, solution, 2);
                    valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_ADD);
                } else {
                    mutate_bonuses(instance, solution);
                    valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_BONUS);
                }
            }
            break;
        }
        case MUTATION_NONE:
            break;
        case MUTATION_ONLY_SWAP: {
            mutate_routes(instance, solution, 0);
            valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_SWAP);
            break;
        }
        case MUTATION_ONLY_REMOVE: {
            mutate_routes(instance, solution, 1);
            valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_REMOVE);
            break;
        }
        case MUTATION_ONLY_ADD: {
            mutate_routes(instance, solution, 2);
            valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_ADD);
            break;
        }
        case MUTATION_ONLY_BONUS: {
            mutate_bonuses(instance, solution);
            valuations += evaluate_after_mutation(instance, solution, EP, stats, MUT_ATOMIC_BONUS);
            break;
        }
    }

    return valuations;
}

#endif // MUTATION_CPP
