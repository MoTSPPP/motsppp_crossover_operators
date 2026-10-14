#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <limits>
#include <algorithm>
using namespace std;
using namespace std::chrono;

// Qual operador de recombinacao o MOEA/D usa. Antes disso, a escolha ficava
// FIXA no codigo (uma linha em moead.cpp), o que obrigava a editar e recompilar
// a cada versao -- e permitia rodar uma bateria inteira com o operador errado se
// o binario estivesse desatualizado. Como string na linha de comando, da para
// rodar varias versoes em paralelo com o MESMO binario.
enum CrossoverMode {
    XO_ORIGINAL = 0,  // one_crossover: 1 ponto, pivo fixo em route.size()/2
    XO_BLOCO,         // one_block_crossover: blocos por carga de passageiros
    XO_PRIORIDADE,    // one_priority_crossover: 1o pai = o de mais passageiros
    XO_RANDOM_POINT,  // random_point_crossover: 1 ponto, pivo sorteado
    XO_GUIDED,        // passenger_guided_insertion_crossover
};

// nome <-> enum (o nome vai para o cabecalho do `data`, para rastreabilidade)
const char* crossover_mode_name(CrossoverMode m);
bool parse_crossover_mode(const std::string& s, CrossoverMode& out);

// Configuracao do experimento - usada para escrever cabecalhos rastreaveis
// nos arquivos `data` por teste e no `summary` por cenario.
struct ExperimentConfig {
    std::string algorithm;          // "MOEAD"
    int mutation_mode = 0;          // 0..7 (ver MutationMode em operators/mutation.h)
    std::string mutation_mode_name; // "CURRENT", "SEQUENTIAL", ...
    int prob_mut_swap = 50;
    int prob_mut_remove = 50;
    int prob_mut_add = 50;
    int prob_mut_bonus = 50;
    int crossover_chance = 0;
    int mutation_chance = 0;
    int N_subproblems = 0;
    int T_neighborhood_size = 0;
    int max_population = 0;
    int max_valuations = 0;
    bool disable_crossover = false; // fases 3/4: pula one_crossover; baby = clone de vizinho
    CrossoverMode crossover_mode = XO_ORIGINAL;
    std::string instance_type;      // "symmetric", "asymmetric", "both"
    std::string folder_name;        // caminho-base de saida
};


struct NormalizedObjectives {
  double cost;
  double time;
  double bonus;
};

struct ObjectiveBounds {
  double min_cost, max_cost;
  double min_time, max_time;
  double min_bonus, max_bonus;
};

struct Passenger {
  int max_cost;
  int origin;
  int destiny;
  int max_time;
};

struct Instance {
  int number_of_cities;
  int number_of_passengers;
  int car_capacity;
  vector<vector<int>> cost_matrix;
  vector<vector<int>> time_matrix;
  vector<Passenger> passengers;
  int min_quota; // we will ignore this for the purpose of MOTSPP
  vector<pair<int, int>> bonus_and_time;
};

struct Solution {
  vector<int> route;
  vector<bool> cities_colected;
  vector<bool> passengers_riding; //(instance.number_of_passengers,false)
  double cost;
  int time;
  int total_bonus;
  double fitness;   
};


struct Population {
  vector<Solution> population;

};

struct Objectives{
  double cost;
  int time;
  int total_bonus;
};

struct Pareto_objectives{
  vector<Objectives> pareto_set;
  vector<Solution> solutions;
};

struct Generations{
  vector<Pareto_objectives> generations;
  vector<duration<double>> durations;
};

struct Tests{
  vector<Generations> tests;
  vector<unsigned int> seeds;
  vector<duration<double>> final_durations;
  vector<Pareto_objectives> final_generations;
};