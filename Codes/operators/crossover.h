#pragma once

#include <vector>

struct Instance;
struct Solution;
struct Population;

// One-point-like crossover: first half of route from father, second half from mother
// excluding cities already present in father's half. Does NOT call passenger loading
// heuristics — caller is responsible for able_passengers / update_objectives.
Solution one_crossover(const Instance& instance, std::vector<Solution>& population, int father, int mother);

// Random-parent overload of one_crossover.
Solution one_crossover(Instance instance, std::vector<Solution>& population);

// Same as one_crossover, but the cut point is drawn at random instead of fixed at route.size() / 2.
// If parent2's route doesn't have enough positions for the drawn point, parent2 is ignored.
Solution random_point_crossover(const Instance& instance, const Solution& parent1, const Solution& parent2);

Solution one_priority_crossover(const Instance& instance, const Solution& parent1, const Solution& parent2);

Solution random_point_priority_crossover(const Instance& instance, const Solution& parent1, const Solution& parent2);

// Copies the spine parent's full route (preserving its bonus/passenger structure),
// then inserts cities from the donor parent that are the origin or destiny of a
// passenger actually riding in the donor, at the position matching the donor's order.
Solution passenger_guided_insertion_crossover(const Instance& instance, const Solution& spine, const Solution& donor);

Solution passenger_guided_insertion_crossover(const Instance& instance, std::vector<Solution>& population, int father, int mother);

Solution passenger_guided_insertion_crossover(Instance instance, std::vector<Solution>& population);

// Generates a children population the same size as the parent population.
// Each child is evaluated (update_objectives) before being added.
std::vector<Solution> crossover(Instance& instance, Population& parents);

Solution one_block_crossover(const Instance& instance, const Solution& spine, const Solution& donor);

Solution one_block_crossover(Instance instance, std::vector<Solution>& population);

Solution one_block_crossover(const Instance& instance, std::vector<Solution>& population, int father, int mother);

std::vector<Solution> block_crossover(Instance& instance, Population& parents);
