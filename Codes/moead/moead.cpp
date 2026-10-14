#include "moead.h"
#include "../main.h"
#include "BoundedParetoSet.cpp"
#include "../utils/solution_utils.h"
#include "../utils/population_utils.h"
#include "../utils/generic_algorithms.h"
#include "../operators/mutation.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <numeric>
#include <chrono>

using namespace std;
using namespace std::chrono;

// MOEA/D main function
Population MOEAD(Generations& MOEAD_generations, BoundedParetoSet *EP ,duration<double>& tempoTotal, Instance instance,  Population &population, int max_valuations, int N_subproblems, int T_neighborhood_size, vector<vector<double> > weight_vectors, int mutation_chance, int crossover_chance, int valuations_before_alg, const MutationConfig& mutation_config, OperatorStats& stats, bool disable_crossover, CrossoverMode crossover_mode ) {
  //Population EP; //1.1
  vector<vector<float>> T_neighbors(weight_vectors.size());
  T_neighbors = get_T_neighbors(weight_vectors, T_neighborhood_size);//1.2
  vector<float> z = get_initial_z(instance, population);//1.4
  pair<vector<Solution>, vector<int>> internal_population = initial_population_moead(population, z, weight_vectors); //1.3
  //end of initialization
  int valuations = valuations_before_alg ;
  int biggest_multiple = 0;
  int multiple = biggest_multiple+1;
  Pareto_objectives sets_of_objectives = get_pareto_objectives(EP);
  MOEAD_generations.generations.push_back(sets_of_objectives);
  auto inicio = high_resolution_clock::now();
  while (valuations < max_valuations) {
    int n=0;
    while(n<N_subproblems){
        int father,mother;
        Solution baby;
        // ================================================================
        // Geracao do `baby` no ramo de variacao dirigida por vizinhanca:
        //   - Fases 1/2 (disable_crossover=false): passenger_guided_insertion_crossover(father, mother)
        //   - Fases 3/4 (disable_crossover=true):  baby = clone de father (vizinho)
        // Em ambos os casos os contadores `crossover_*` representam "quantas
        // vezes este ramo foi escolhido"; a interpretacao semantica deve ser
        // feita pelo Python lendo `disable_crossover` no cabecalho do `data`.
        // ================================================================
        if(crossover_chance < random_chance()){
          father= T_neighbors[n][rand()%T_neighborhood_size];
          if (disable_crossover) {
            // Fase 3/4: desacopla a funcao de crossover. O vizinho selecionado
            // (father) segue o fluxo direto, sem combinar com mother.
            baby = internal_population.first[father];
          } else {
            mother= T_neighbors[n][rand()%T_neighborhood_size];
            const Solution& pai = internal_population.first[father];
            const Solution& mae = internal_population.first[mother];
            switch (crossover_mode) {
              case XO_BLOCO:
                baby = one_block_crossover(instance, pai, mae); break;
              case XO_PRIORIDADE:
                baby = one_priority_crossover(instance, pai, mae); break;
              case XO_RANDOM_POINT:
                baby = random_point_crossover(instance, pai, mae); break;
              case XO_GUIDED:
                baby = passenger_guided_insertion_crossover(instance, pai, mae); break;
              case XO_ORIGINAL:
              default:
                baby = one_crossover(instance, internal_population.first,
                                     father, mother); break;
            }
          }
          able_passengers(instance, baby);
          update_objectives(instance, baby);
          bool baby_feasible = solution_validity(instance, baby);
          stats.crossover_calls++;
          if (baby_feasible) stats.crossover_feasible++;
          if(baby_feasible){
            if(EP->adicionarSol(&baby)){
              continue;
            }
          }
          valuations++;
          int multiple = biggest_multiple+1;
          if(multiple*10000<=valuations){
              save_data_routine(MOEAD_generations,EP, inicio, biggest_multiple, valuations, tempoTotal);
              if(valuations>=max_valuations){
                break;
              }
              biggest_multiple++;
          }
          
          // Apply mutation strategy
          bool should_mutate = false;
          if (disable_crossover && mutation_config.mode != MUTATION_NONE) {
            // Fase 3/4: baby = clone do vizinho. Forca mutacao para que o
            // candidato nao seja identico ao father (senao algoritmo congela).
            should_mutate = true;
          } else if (mutation_config.mode == MUTATION_PARAMETERIZED) {
            should_mutate = true;
          } else if (mutation_config.mode != MUTATION_NONE) {
            should_mutate = (mutation_chance < random_chance());
          }
          if (should_mutate) {
            int consumed = apply_mutation_strategy(instance, baby, EP, mutation_config, &stats);
            valuations += consumed;
            if (consumed > 0) {
              int multiple = biggest_multiple+1;
              if(multiple*10000<=valuations){
                  save_data_routine(MOEAD_generations, EP, inicio, biggest_multiple, valuations, tempoTotal);
                  if(valuations>=max_valuations){
                    break;
                  }
                  biggest_multiple++;
              }
            }
          }//2.2
        }
        else{
          get_random_solution(instance, baby);
          bool baby_feasible = solution_validity(instance, baby);
          stats.random_init_calls++;
          if (baby_feasible) stats.random_init_feasible++;
          if(baby_feasible){
            if(EP->adicionarSol(&baby)){
              continue;
            }
          }
          valuations++;
          int multiple = biggest_multiple+1;
          if(multiple*10000<=valuations){
              save_data_routine(MOEAD_generations,EP, inicio, biggest_multiple, valuations, tempoTotal);
              if(valuations>=max_valuations){
                break;
              }
              biggest_multiple++;
          }
        }
        if(solution_validity(instance,baby)==true){
          update_z(z,baby);//2.3
          update_neighboring_solutions(n,internal_population, baby, weight_vectors, T_neighbors, z); //2.4 
          n++;
        }
    }    
  }
  return population;
}

// MOEA/D helper functions
double euclidian_distance(vector<double> &weight_vector1, vector<double> &weight_vector2) {
  float distance = 0.0;
  for (int i = 0; i < weight_vector1.size(); i++) {
    distance += pow(weight_vector1[i] - weight_vector2[i], 2);
  }
  return sqrt(distance);
} // calculate distance between two weight vectors

vector<vector<float>> get_T_neighbors(vector<vector<double> > weight_vectors, int T_neighborhood_size){
  vector<vector<float>> T_neighbors(weight_vectors.size());
  for(int i = 0; i<weight_vectors.size();i++){
    vector<pair<double,int>> distances;
    for(int j = 0; j< weight_vectors.size();j++){
      if(i!=j){
        distances.push_back(make_pair(euclidian_distance(weight_vectors[i], weight_vectors[j]),j));
      }
    }
    heapSort(distances);
    for(int t = 0; t<T_neighborhood_size;t++){
      T_neighbors[i].push_back(distances[t].second);
    }
  }
  return T_neighbors;
}

vector<float> get_initial_z(Instance instance, Population population) {
  vector<float> z(3, 0);

  z[0] = population.population[0].cost;
  for (int i = 1; i < population.population.size(); i++) {
    double cost = get_cost(instance, population.population[i]);
    if (cost < z[0]) {
      z[0] = cost;
    }
  }

  z[1] = population.population[0].time;
  for (int i = 1; i < population.population.size(); i++) {
    double time = get_time(instance, population.population[i]);
    if (time < z[1]) {
      z[1] = time;
    }
  }

  z[2] = population.population[0].total_bonus;
  for (int i = 1; i < population.population.size(); i++) {
    double bonus = get_bonus(instance, population.population[i]);
    if (bonus > z[2]) {
      z[2] = bonus;
    }
  }

  return z;
} // returns z = [cost, time, bonus]

void update_z(vector<float> z, Solution solution){
  if(solution.cost<z[0]){//step 2.3 of MOEAD paper
    z[0] = solution.cost;
  }
  if(solution.time<z[1]){
    z[1] = solution.time;
  }
  if(solution.total_bonus> z[2]){
    z[2] = solution.total_bonus;
  }
}//z[cost,time,bonus]

double get_tchebycheff(const Solution& solution, vector<float> z, vector<double> weight_vector){
  double result = fabs(weight_vector[0]*(solution.cost - z[0]));
  if(result<fabs(weight_vector[1]*(solution.time - z[1]))){
    result = fabs(weight_vector[1]*(solution.time - z[1]));
  }
  if(result<fabs(weight_vector[2]*(solution.total_bonus - z[2]))){
    result = fabs(weight_vector[2]*(solution.total_bonus - z[2]));
  }
  return result;
}

double get_tchebycheff(const Objectives& solution, vector<float> z, vector<double> weight_vector){
  double result = fabs(weight_vector[0]*(solution.cost - z[0]));
  if(result<fabs(weight_vector[1]*(solution.time - z[1]))){
    result = fabs(weight_vector[1]*(solution.time - z[1]));
  }
  if(result<fabs(weight_vector[2]*(solution.total_bonus - z[2]))){
    result = fabs(weight_vector[2]*(solution.total_bonus - z[2]));
  }
  return result;
}

pair<vector<Solution>, vector<int>> initial_population_moead (Population Population, vector<float> z, vector<vector<double> > weight_vectors){
  pair<vector<Solution>, vector<int>> initial_population;
  for(int i = 0; i<weight_vectors.size();i++){
    pair<double,int> best_for_this_vector;
    best_for_this_vector.second = 0;
    best_for_this_vector.first = get_tchebycheff(Population.population[0], z, weight_vectors[i]);

    for(int j=1;j<Population.population.size();j++){
      double j_tchebycheff = get_tchebycheff(Population.population[j], z, weight_vectors[i]);
      if(best_for_this_vector.first> j_tchebycheff){
        best_for_this_vector.second = j;
        best_for_this_vector.first = j_tchebycheff;
      }
      //define which solution of the population is best for weight vector i
    }
    initial_population.first.push_back(Population.population[best_for_this_vector.second]);
    initial_population.second.push_back(best_for_this_vector.first);
  }
  return initial_population;
}

void update_neighboring_solutions(int n, pair<vector<Solution>, vector<int> > &internal_population, Solution baby, vector<vector<double> > weight_vectors, vector<vector<float>> T_neighbors,vector<float> z ){
  for(int t=0;t<T_neighbors[n].size(); t++){
    double baby_tchebycheff = get_tchebycheff(baby, z, weight_vectors[T_neighbors[n][t]]);
    double neighbor_tchebycheff = get_tchebycheff(internal_population.first[T_neighbors[n][t]], z, weight_vectors[T_neighbors[n][t]]);
    if(baby_tchebycheff < neighbor_tchebycheff){
        internal_population.first[T_neighbors[n][t]] = baby;
        internal_population.second[T_neighbors[n][t]] = baby_tchebycheff;
    }
  }
}

vector<vector<double>> generateWeightVectors(int NUMSUMPROBLEMAS) {
  vector<vector<double>> lambda(NUMSUMPROBLEMAS, vector<double>(3, 0.0f));
  int  s= 23; // dont change
	int cont =0;
	double l1, l2, l3;
    for (int i=0; i<=s && cont<NUMSUMPROBLEMAS; i++){
        l1 = i;
        for (int j=0; j<=s-i && cont<NUMSUMPROBLEMAS; j++){
            l2 = j;
            l3 = s-i - l2;
            lambda[cont][0] = static_cast<double>(l1/s); // first coordinate
            lambda[cont][1] = static_cast<double>(l2/s); // second coordinate
            lambda[cont][2] = static_cast<double>(l3/s); // third coordinate
            cont++; 
        }
    }
    return lambda;
}
