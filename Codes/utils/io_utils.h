#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>

// Forward declarations
struct Instance;
struct Solution;
struct Population;
struct Generations;
struct ExperimentConfig;
struct OperatorStats;

// Input functions
Instance get_instance(std::string file_path);
std::vector<std::string> generate_instance_paths(const std::string& type_filter);

// Output functions
void print_solution(Solution solution);
void print_solution(Instance instance, Solution &solution);
void print_instance(Instance& instance);

// File and directory management
void create_all_directories(int num_tests, int num_instances, std::string folder_name, std::vector<std::string> instances_addresses);
void create_test_archives(
  const Instance& instance,
  std::string algorithm_name,
  std::string folder_name,
  std::string instance_address,
  int current_test,
  Generations& saved_generations,
  unsigned int seed,
  std::chrono::duration<double> tempoTotal,
  const ExperimentConfig& config,
  const OperatorStats& test_stats
);
