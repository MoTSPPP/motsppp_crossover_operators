#include <iostream>

#include <unistd.h>   // getpid() para a semente
#include "main.h"
#include "moead/BoundedParetoSet.cpp"
#include "utils/io_utils.h"
#include "utils/population_utils.h"
#include "moead/moead.h"
#include "operators/mutation.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm> 
#include <chrono>
#include <filesystem>
#include <queue>
#include <cstdlib>
#include <random> 
#include <limits>
#include <numeric>

using namespace std;
using namespace std::chrono;

const char* crossover_mode_name(CrossoverMode m) {
  switch (m) {
    case XO_BLOCO:        return "BLOCO";
    case XO_PRIORIDADE:   return "PRIORIDADE";
    case XO_RANDOM_POINT: return "RANDOM_POINT";
    case XO_GUIDED:       return "GUIDED";
    case XO_ORIGINAL:
    default:              return "ORIGINAL";
  }
}

bool parse_crossover_mode(const std::string& s, CrossoverMode& out) {
  if (s == "original")          { out = XO_ORIGINAL;     return true; }
  if (s == "bloco")             { out = XO_BLOCO;        return true; }
  if (s == "prioridade")        { out = XO_PRIORIDADE;   return true; }
  if (s == "random_point")      { out = XO_RANDOM_POINT; return true; }
  if (s == "guided")            { out = XO_GUIDED;       return true; }
  return false;
}

int main(int argc, char* argv[]) {
  string folder_name = "results/banana";
  string algorithm = "moead";
  string instance_type = "both";          // symmetric | asymmetric | both
  string single_instance = "";            // --instance: roda SO esta instancia
  CrossoverMode crossover_mode = XO_ORIGINAL;

  //Setup quantidade de testes
  int qtd_testes=20;

  //SETUP DOS ALGORITMOS
  int max_population =300;
  int max_valuations = 80000;

  int crossover_chance = 56;
  int mutation_chance = 53;

  // MOEAD specific
  int N_subproblems = 300;
  int T_neighborhood_size = 20;
  
  // (PARAMETRIZADA) DOIS SORTEIOS PARA ENTRAR NA MUTAÇÃO E A OUTRA PARA ESCOLHER QUAL OPERADOR DE MUTAÇÃO SERÁ APLICADO.
  // SOMATORIO DA TAXA DE MUTAÇAO DEVE SER IGUAL A 100;

  
  // Mutation strategy configuration (default: current behavior)
  MutationConfig mutation_config;

  // Crossover toggle (default: ativado). Fases 3/4 usam --disable_crossover.
  bool disable_crossover = false;
  for (int i = 1; i < argc; i++) {
    string arg = argv[i];
    if (arg == "--algorithm" && i + 1 < argc) {
      algorithm = argv[++i];
      if (algorithm != "moead") {
        cerr << "Erro: --algorithm deve ser moead" << endl;
        return 1;
      }
    } else if (arg == "--folder_name" && i + 1 < argc) {
      folder_name = argv[++i];
    } else if (arg == "--mutation_mode" && i + 1 < argc) {
      int mode_val = atoi(argv[++i]);
      if (mode_val < 0 || mode_val > 7) {
        cerr << "Erro: --mutation_mode deve ser 0 (atual), 1 (sequencial), 2 (parametrizado), 3 (sem mutacao), "
             << "4 (so swap), 5 (so remove), 6 (so add) ou 7 (so bonus)" << endl;
        return 1;
      }
      mutation_config.mode = static_cast<MutationMode>(mode_val);
    } else if (arg == "--prob_mut_swap" && i + 1 < argc) {
      mutation_config.prob_swap = atoi(argv[++i]);
      if (mutation_config.prob_swap < 0 || mutation_config.prob_swap > 100) {
        cerr << "Erro: --prob_mut_swap deve ser entre 0 e 100" << endl;
        return 1;
      }
    } else if (arg == "--prob_mut_remove" && i + 1 < argc) {
      mutation_config.prob_remove = atoi(argv[++i]);
      if (mutation_config.prob_remove < 0 || mutation_config.prob_remove > 100) {
        cerr << "Erro: --prob_mut_remove deve ser entre 0 e 100" << endl;
        return 1;
      }
    } else if (arg == "--prob_mut_add" && i + 1 < argc) {
      mutation_config.prob_add = atoi(argv[++i]);
      if (mutation_config.prob_add < 0 || mutation_config.prob_add > 100) {
        cerr << "Erro: --prob_mut_add deve ser entre 0 e 100" << endl;
        return 1;
      }
    } else if (arg == "--prob_mut_bonus" && i + 1 < argc) {
      mutation_config.prob_bonus = atoi(argv[++i]);
      if (mutation_config.prob_bonus < 0 || mutation_config.prob_bonus > 100) {
        cerr << "Erro: --prob_mut_bonus deve ser entre 0 e 100" << endl;
        return 1;
      }
    } else if (arg == "--crossover" && i + 1 < argc) {
      crossover_chance = atoi(argv[++i]);
    } else if (arg == "--mutation" && i + 1 < argc) {
      mutation_chance = atoi(argv[++i]);
    } else if (arg == "--qtd_testes" && i + 1 < argc) {
      qtd_testes = atoi(argv[++i]);
    } else if (arg == "--max_valuations" && i + 1 < argc) {
      max_valuations = atoi(argv[++i]);
    } else if (arg == "--disable_crossover") {
      disable_crossover = true;
    } else if (arg == "--crossover_mode" && i + 1 < argc) {
      // Escolhe o operador de recombinacao em tempo de execucao. Antes a
      // escolha era fixa no codigo, o que exigia recompilar por versao e
      // impedia rodar varias em paralelo com o mesmo binario.
      if (!parse_crossover_mode(argv[++i], crossover_mode)) {
        cerr << "Erro: --crossover_mode deve ser original, bloco, prioridade, "
             << "random_point ou guided" << endl;
        return 1;
      }
    } else if (arg == "--instance" && i + 1 < argc) {
      // Roda uma unica instancia (caminho relativo ao projeto). Usado pelo
      // irace: o target-runner recebe do irace qual instancia avaliar, e sem
      // isto a unica alternativa era rodar o grupo inteiro e medir uma
      // instancia arbitraria -- caro e sem relacao com o que foi pedido.
      single_instance = argv[++i];
    } else if (arg == "--instance_type" && i + 1 < argc) {
      instance_type = argv[++i];
      if (instance_type != "symmetric" && instance_type != "asymmetric" &&
          instance_type != "both") {
        cerr << "Erro: --instance_type deve ser symmetric, asymmetric ou both"
             << endl;
        return 1;
      }
    }
  }

  string alg_display_name = "MOEAD";

  // Resolve nome legivel do modo de mutacao
  string mutation_mode_name;
  switch (mutation_config.mode) {
    case MUTATION_CURRENT:       mutation_mode_name = "CURRENT"; break;
    case MUTATION_SEQUENTIAL:    mutation_mode_name = "SEQUENTIAL"; break;
    case MUTATION_PARAMETERIZED: mutation_mode_name = "PARAMETERIZED"; break;
    case MUTATION_NONE:          mutation_mode_name = "NONE"; break;
    case MUTATION_ONLY_SWAP:     mutation_mode_name = "ONLY_SWAP"; break;
    case MUTATION_ONLY_REMOVE:   mutation_mode_name = "ONLY_REMOVE"; break;
    case MUTATION_ONLY_ADD:      mutation_mode_name = "ONLY_ADD"; break;
    case MUTATION_ONLY_BONUS:    mutation_mode_name = "ONLY_BONUS"; break;
    default:                     mutation_mode_name = "UNKNOWN"; break;
  }

  // Empacota configuracao do experimento para escrita no `data` e `summary`
  ExperimentConfig exp_config;
  exp_config.algorithm           = alg_display_name;
  exp_config.mutation_mode       = static_cast<int>(mutation_config.mode);
  exp_config.mutation_mode_name  = mutation_mode_name;
  exp_config.prob_mut_swap       = mutation_config.prob_swap;
  exp_config.prob_mut_remove     = mutation_config.prob_remove;
  exp_config.prob_mut_add        = mutation_config.prob_add;
  exp_config.prob_mut_bonus      = mutation_config.prob_bonus;
  exp_config.crossover_chance    = crossover_chance;
  exp_config.mutation_chance     = mutation_chance;
  exp_config.N_subproblems       = N_subproblems;
  exp_config.T_neighborhood_size = T_neighborhood_size;
  exp_config.max_population      = max_population;
  exp_config.max_valuations      = max_valuations;
  exp_config.disable_crossover   = disable_crossover;
  exp_config.instance_type       = instance_type;
  exp_config.crossover_mode      = crossover_mode;
  exp_config.folder_name         = folder_name;

  // Generate instances
  vector<string> instances_addresses;
  if (!single_instance.empty()) {
    instances_addresses.push_back(single_instance);
  } else {
    instances_addresses = generate_instance_paths(instance_type);
  }
  int numero_de_instancias = instances_addresses.size();

  cout << "Algoritmo: " << alg_display_name << " | Instancias: " << numero_de_instancias
       << " | Testes: " << qtd_testes
       << " | Max avaliacoes: " << max_valuations
       << " | Mutation mode: " << mutation_config.mode
       << " | Crossover: " << (disable_crossover ? "DESACOPLADO (fase 3/4)" : "ATIVO") << endl;

  try {
    create_all_directories(qtd_testes, numero_de_instancias, folder_name, instances_addresses);
    cout << "Diretorios criados com sucesso!" << endl;
  } catch (const std::filesystem::filesystem_error& e) {
    cerr << "Erro ao criar diretorios: " << e.what() << endl;
  }

  // MOEAD weight vectors (generated once)
  vector<vector<double>> weight_vectors = generateWeightVectors(max_population);

  for (int instance_index = 0; instance_index < numero_de_instancias; instance_index++) {
    Instance instance = get_instance(instances_addresses[instance_index]);
    int progress_pct = ((instance_index + 1) * 100) / numero_de_instancias;
    cout << "[" << (instance_index + 1) << "/" << numero_de_instancias << "] ("
         << progress_pct << "%) INICIO DA EXECUCAO: " << instances_addresses[instance_index] << endl;
    print_instance(instance);

    for (int current_test = 0; current_test < qtd_testes; current_test++) {
      // Resume: se ja existe o arquivo `data` deste teste, pula. O `data` e
      // o ULTIMO arquivo escrito por create_test_archives -> se existe, o
      // teste foi completo. Testes parciais (sem `data`) sao refeitos do zero.
      string data_path = folder_name + "/" + instances_addresses[instance_index]
                         + "/test_" + to_string(current_test) + "/data";
      if (std::filesystem::exists(data_path)) {
        cout << "test " << current_test << " | " << instances_addresses[instance_index]
             << " [SKIP - ja completo]" << endl;
        continue;
      }
      cout << "test " << current_test << " | " << instances_addresses[instance_index] << endl;
      BoundedParetoSet *EP = new BoundedParetoSet();
      Population population;
      Generations saved_generations;
      // time(0) tem resolucao de 1 segundo: varios processos rodando em
      // paralelo iniciariam testes no mesmo segundo e receberiam a MESMA
      // semente, gerando execucoes identicas sem nenhum sinal de erro.
      // Misturar pid e indices garante sementes distintas entre processos.
      unsigned int seed = static_cast<unsigned int>(time(0))
                        ^ (static_cast<unsigned int>(getpid()) << 16)
                        ^ (static_cast<unsigned int>(instance_index) << 8)
                        ^ static_cast<unsigned int>(current_test);
      srand(seed);
      int valuations_before_alg = 0;
      Population initial_population;
      get_random_population(instance, initial_population, EP, max_population, valuations_before_alg);
      int max_valuations_after_alg = max_valuations - valuations_before_alg;
      duration<double> tempoTotal = duration<double>::zero();

      OperatorStats per_test_stats;

      population = MOEAD(saved_generations, EP, tempoTotal, instance, initial_population, max_valuations_after_alg, N_subproblems, T_neighborhood_size, weight_vectors, mutation_chance, crossover_chance, valuations_before_alg, mutation_config, per_test_stats, disable_crossover, crossover_mode);

      create_test_archives(instance, alg_display_name, folder_name, instances_addresses[instance_index], current_test, saved_generations, seed, tempoTotal, exp_config, per_test_stats);
      delete EP;
    }
    cout << "Resultados disponiveis para " << alg_display_name << " nessa instancia!" << endl;
  }

}