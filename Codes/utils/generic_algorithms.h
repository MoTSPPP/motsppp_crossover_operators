#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <numeric>

// Swap functions
void swap(std::pair<double, int> &a, std::pair<double, int> &b);

// Heapify functions
void heapify(std::vector<std::pair<double, int>> &vec, int n, int i);

// HeapSort functions
void heapSort(std::vector<std::pair<double, int>> &vec);
