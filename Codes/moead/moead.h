#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <numeric>
#include <chrono>
#include "../main.h"          // CrossoverMode
#include "../operators/mutation.h"

// Forward declarations
struct Instance;
struct Solution;
struct Population;
struct Generations;
struct BoundedParetoSet;
struct Objectives;

// MOEA/D main function
Population MOEAD(
  Generations& MOEAD_generations,
  BoundedParetoSet *EP,
  std::chrono::duration<double>& tempoTotal,
  Instance instance,
  Population &population,
  int max_valuations,
  int N_subproblems,
  int T_neighborhood_size,
  std::vector<std::vector<double>> weight_vectors,
  int mutation_chance,
  int crossover_chance,
  int valuations_before_alg,
  const MutationConfig& mutation_config,
  OperatorStats& stats,
  bool disable_crossover,
  CrossoverMode crossover_mode
);

// MOEA/D helper functions
double euclidian_distance(std::vector<double> &weight_vector1, std::vector<double> &weight_vector2);
std::vector<std::vector<float>> get_T_neighbors(std::vector<std::vector<double>> weight_vectors, int T_neighborhood_size);
std::vector<float> get_initial_z(Instance instance, Population population);
void update_z(std::vector<float> z, Solution solution);
double get_tchebycheff(const Solution& solution, std::vector<float> z, std::vector<double> weight_vector);
double get_tchebycheff(const Objectives& solution, std::vector<float> z, std::vector<double> weight_vector);
std::pair<std::vector<Solution>, std::vector<int>> initial_population_moead(Population Population, std::vector<float> z, std::vector<std::vector<double>> weight_vectors);
void update_neighboring_solutions(int n, std::pair<std::vector<Solution>, std::vector<int>> &internal_population, Solution baby, std::vector<std::vector<double>> weight_vectors, std::vector<std::vector<float>> T_neighbors, std::vector<float> z);
std::vector<std::vector<double>> generateWeightVectors(int NUMSUMPROBLEMAS);
