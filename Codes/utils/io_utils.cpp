#include "io_utils.h"
#include "../main.h"
#include "../operators/mutation.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>
#include <sstream>

using namespace std;
using namespace std::chrono;

// Input functions
Instance get_instance(string file_path) {
  Instance instance;

  ifstream file(file_path.c_str());
  if (!file.is_open()) {
    cerr << "Erro ao abrir o arquivo " << file_path << endl;
    return instance;
  }

  // Reading data archives
  file >> instance.number_of_cities >> instance.number_of_passengers >> instance.car_capacity;

  // resizing matrices
  instance.cost_matrix.resize(instance.number_of_cities, vector<int>(instance.number_of_cities));
  instance.time_matrix.resize(instance.number_of_cities, vector<int>(instance.number_of_cities));

  // Reading cost_matrix
  for (int i = 0; i < instance.number_of_cities; ++i) {
    for (int j = 0; j < instance.number_of_cities; ++j) {
      file >> instance.cost_matrix[i][j];
    }
  }

  // Reading time_matrix
  for (int i = 0; i < instance.number_of_cities; ++i) {
    for (int j = 0; j < instance.number_of_cities; ++j) {
      file >> instance.time_matrix[i][j];
    }
  }

  // Reading passengers
  int max_cost, origin, destiny, max_time;
  for (int i = 0; i < instance.number_of_passengers; i++) {
    Passenger passenger;
    file >> max_cost >> origin >> destiny >> max_time;   
    passenger.max_cost = max_cost;
    passenger.origin = origin;
    passenger.destiny = destiny;
    passenger.max_time = max_time;
    instance.passengers.push_back(passenger);
  }

  // Reading min_quota
  file >> instance.min_quota;

  // Reading bonus_and_time
  vector<pair<int, int>> temp_bonus_and_time;
  vector<int> index;
  int city, bonus, time;
  while (file >> city >> bonus >> time) {
    temp_bonus_and_time.push_back(make_pair(bonus, time));
    index.push_back(city);
  }
  for(int i=0; i<temp_bonus_and_time.size(); i++){
    for(int j=0; j<temp_bonus_and_time.size(); j++){
      if(index[j] == i){
        instance.bonus_and_time.push_back(temp_bonus_and_time[j]);
        break;
      }
    }
  }

  file.close();

  return instance;
}

vector<string> generate_instance_paths(const string& type_filter) {
    vector<string> instances;
    // type_filter permite rodar symmetric e asymmetric como processos separados,
    // dividindo a bateria pela metade sem que as saidas se sobreponham.
    vector<string> types;
    if (type_filter == "both") {
        types = {"symmetric","asymmetric"};
    } else {
        types = {type_filter};
    }
    vector<int> sizes = {5, 8, 10, 20, 50, 100, 200};
    int files_per_instance = 6;

    for (const auto& type : types) {
        for (int size : sizes) {
            for (int i = 1; i <= files_per_instance; ++i) {
                ostringstream path;
                path << "instances/" << type << "/" << size << "." << i << ".in";
                instances.push_back(path.str());
            }
        }
    }

    return instances;
}

// Output functions
void print_solution(Solution solution) {
  cout << "Route: ";
  for(int i = 0; i < solution.route.size(); i++){
    cout << solution.route[i] << " ";
  }
  cout<<endl;
  cout << "Cost: " << solution.cost << endl;
  cout << "Bonus: " << solution.total_bonus << endl;
  cout << "Time: " << solution.time << "\n \n";
}

void print_solution(Instance instance, Solution &solution) {
  cout << "Route: ";
  for(int i = 0; i < solution.route.size(); i++){
    cout << solution.route[i] << " ";
  }
  cout<<endl;

  cout<<"Route:"<<endl;
  for(int city = 0; city < solution.route.size(); city++){
    int next_city;
    if(city == solution.route.size() - 1){
      next_city = 0;
    }
    else{
      next_city = city+1;
    }
    cout << instance.cost_matrix[solution.route[city]][solution.route[next_city]]<< " ";
  }
  cout<<endl;

  cout<<"Route times:"<<endl;
  for(int city = 0; city < solution.route.size(); city++){
    int next_city;
    if(city == solution.route.size() - 1){
      next_city = 0;
    }
    else{
      next_city = city+1;
    }
    cout << instance.time_matrix[solution.route[city]][solution.route[next_city]]<< " ";
  }
  cout<<endl;

  cout << "Cities_colected: "<<endl;
  bool any_city_colected = false;
  for(int i =0; i<solution.cities_colected.size();i++){
    if(solution.cities_colected[i]==true){
      cout<<"City: "<< solution.route[i] << " bonus " << instance.bonus_and_time[solution.route[i]].first << " time " << instance.bonus_and_time[solution.route[i]].second << endl;
      any_city_colected = true;
    }
  }
  if(!any_city_colected){
    cout<<"No city colected"<<endl;
  }
  cout<<endl;
  int passengers_on = 0;
  for (int i = 0; i < solution.passengers_riding.size(); i++) {
    if (solution.passengers_riding[i]) {
      passengers_on++;
      cout<< "passenger "<<i<<" with origin "<<instance.passengers[i].origin<<", destin: "<<instance.passengers[i].destiny<< " max cost: "<<instance.passengers[i].max_cost<<" max time: "<<instance.passengers[i].max_time<<endl;
    }
  }
  cout << "boarded passengers: " << passengers_on << endl;
  cout << "Cost: " << solution.cost << endl;
  cout << "Bonus: " << solution.total_bonus << endl;
  cout << "Time: " << solution.time << "\n \n";
}

void print_instance(Instance& instance){
  cout << "Number of cities: " << instance.number_of_cities << endl;
  cout << "Number of passengers: " << instance.number_of_passengers << endl;
  cout << "Car capacity: " << instance.car_capacity <<"\n " <<endl;
}

// File and directory management
void create_all_directories(int num_tests, int num_instances, std::string folder_name, vector<string> instances_addresses) {
  for (int instance = 0; instance < num_instances; ++instance) {
      std::string instance_dir = folder_name + "/" + instances_addresses[instance];
        for (int test = 0; test < num_tests; ++test) {
          std::string teste_dir = instance_dir + "/test_" + std::to_string(test);
          filesystem::create_directories(teste_dir);
         }
      }
}

// Helper: formata porcentagem segura para division-by-zero.
static string fmt_pct(int feasible, int calls) {
  if (calls == 0) return "n/a";
  double pct = (100.0 * feasible) / calls;
  ostringstream s;
  s.setf(std::ios::fixed);
  s.precision(2);
  s << pct << "%";
  return s.str();
}

void create_test_archives(
  const Instance& instance,
  string algorithm_name,
  string folder_name,
  string instance_address,
  int current_test,
  Generations& saved_generations,
  unsigned int seed,
  duration<double> tempoTotal,
  const ExperimentConfig& config,
  const OperatorStats& test_stats){
  for(int generation = 0; generation < saved_generations.generations.size(); generation++){
    string endereço_do_arquivo = folder_name+ "/" + instance_address +"/"+ "test_" + to_string(current_test) +"/paretogeneration_" + to_string(generation) + ".txt";
    string endereço_dados = folder_name+ "/" + instance_address +"/"+ "test_" + to_string(current_test) + "/data";
    ofstream arquivo(endereço_do_arquivo);
    ofstream arquivo_dados(endereço_dados);
    for(int pareto_set = 0; pareto_set< saved_generations.generations[generation].pareto_set.size() ; pareto_set++){
      arquivo<<saved_generations.generations[generation].pareto_set[pareto_set].cost<<" "<< saved_generations.generations[generation].pareto_set[pareto_set].time<<" " << saved_generations.generations[generation].pareto_set[pareto_set].total_bonus;
      if(pareto_set<saved_generations.generations[generation].pareto_set.size()-1){
        arquivo<<endl;
      }
    }

    // === EXPERIMENT CONFIG ===
    arquivo_dados << "# === EXPERIMENT CONFIG ==="<<endl;
    arquivo_dados << "algorithm: " << config.algorithm << endl;
    arquivo_dados << "mutation_mode: " << config.mutation_mode << " (" << config.mutation_mode_name << ")" << endl;
    arquivo_dados << "disable_crossover: " << (config.disable_crossover ? "true" : "false") << endl;
    arquivo_dados << "crossover_mode: " << crossover_mode_name(config.crossover_mode) << endl;
    arquivo_dados << "instance_type: " << config.instance_type << endl;
    arquivo_dados << "crossover_chance: " << config.crossover_chance << endl;
    arquivo_dados << "mutation_chance: " << config.mutation_chance << endl;
    arquivo_dados << "prob_mut_swap: " << config.prob_mut_swap << endl;
    arquivo_dados << "prob_mut_remove: " << config.prob_mut_remove << endl;
    arquivo_dados << "prob_mut_add: " << config.prob_mut_add << endl;
    arquivo_dados << "prob_mut_bonus: " << config.prob_mut_bonus << endl;
    arquivo_dados << "N_subproblems: " << config.N_subproblems << endl;
    arquivo_dados << "T_neighborhood_size: " << config.T_neighborhood_size << endl;
    arquivo_dados << "max_population: " << config.max_population << endl;
    arquivo_dados << "max_valuations: " << config.max_valuations << endl;
    arquivo_dados << "instance_path: " << instance_address << endl;
    arquivo_dados << endl;

    // === RUN INFO ===
    arquivo_dados << "# === RUN INFO ==="<<endl;
    arquivo_dados << "seed: " << seed << endl;
    arquivo_dados << "total time in seconds: " << tempoTotal.count() << endl;
    arquivo_dados << endl;

    // === OPERATOR FEASIBILITY (este teste) ===
    arquivo_dados << "# === OPERATOR FEASIBILITY (este teste) ===" << endl;
    arquivo_dados << "crossover_calls: " << test_stats.crossover_calls << endl;
    arquivo_dados << "crossover_feasible: " << test_stats.crossover_feasible
                  << " (" << fmt_pct(test_stats.crossover_feasible, test_stats.crossover_calls) << ")" << endl;
    arquivo_dados << "crossover_infeasible: " << (test_stats.crossover_calls - test_stats.crossover_feasible)
                  << " (" << fmt_pct(test_stats.crossover_calls - test_stats.crossover_feasible, test_stats.crossover_calls) << ")" << endl;
    arquivo_dados << "random_init_calls: " << test_stats.random_init_calls << endl;
    arquivo_dados << "random_init_feasible: " << test_stats.random_init_feasible
                  << " (" << fmt_pct(test_stats.random_init_feasible, test_stats.random_init_calls) << ")" << endl;
    arquivo_dados << "random_init_infeasible: " << (test_stats.random_init_calls - test_stats.random_init_feasible)
                  << " (" << fmt_pct(test_stats.random_init_calls - test_stats.random_init_feasible, test_stats.random_init_calls) << ")" << endl;
    arquivo_dados << "mutation_calls: " << test_stats.mutation_calls << endl;
    arquivo_dados << "mutation_feasible: " << test_stats.mutation_feasible
                  << " (" << fmt_pct(test_stats.mutation_feasible, test_stats.mutation_calls) << ")" << endl;
    arquivo_dados << "mutation_infeasible: " << (test_stats.mutation_calls - test_stats.mutation_feasible)
                  << " (" << fmt_pct(test_stats.mutation_calls - test_stats.mutation_feasible, test_stats.mutation_calls) << ")" << endl;
    arquivo_dados << endl;
    arquivo_dados << "# Mutation breakdown by atomic type (zeros no modo CURRENT/NONE):" << endl;
    arquivo_dados << "swap_calls: " << test_stats.swap_calls
                  << " feasible: " << test_stats.swap_feasible << " (" << fmt_pct(test_stats.swap_feasible, test_stats.swap_calls) << ")" << endl;
    arquivo_dados << "remove_calls: " << test_stats.remove_calls
                  << " feasible: " << test_stats.remove_feasible << " (" << fmt_pct(test_stats.remove_feasible, test_stats.remove_calls) << ")" << endl;
    arquivo_dados << "add_calls: " << test_stats.add_calls
                  << " feasible: " << test_stats.add_feasible << " (" << fmt_pct(test_stats.add_feasible, test_stats.add_calls) << ")" << endl;
    arquivo_dados << "bonus_calls: " << test_stats.bonus_calls
                  << " feasible: " << test_stats.bonus_feasible << " (" << fmt_pct(test_stats.bonus_feasible, test_stats.bonus_calls) << ")" << endl;
    arquivo_dados << endl;

    // === TIMING PER 10K VALUATIONS BLOCK ===
    arquivo_dados << "# === TIMING PER 10K VALUATIONS BLOCK ===" << endl;
    for(int duration =0;duration < saved_generations.durations.size(); duration++){
      arquivo_dados<<"time from "<<duration*10000<<" valuations to "<<(duration+1)*10000<<" valuations:"<<endl<< saved_generations.durations[duration].count()<<endl;
    }
    arquivo.close();
    arquivo_dados.close();
  }
}


