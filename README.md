# Passenger-guided crossover and mutation operators for the multi-objective traveling salesman problem with profits and passengers

This repository contains the implementation, benchmark instances, and experimental data for the paper:  

> **"Passenger-guided crossover and mutation operators for the multi-objective traveling salesman problem with profits and passengers"**  
> *Submitted to [JOURNAL]*  

## 📄 Abstract  

In industrial plants, a maintenance vehicle can perform services at production units while transporting items between them. Combining both services in one vehicle can reduce costs and environmental impacts. The Multi-objective Traveling Salesman Problem with Profits and Passengers (MoTSPPP) models this setting, where each service yields a bonus and each transported item is a passenger who shares the route cost. The MoTSPPP minimizes route cost and travel time and maximizes the collected bonus subject to capacity, budget, and time constraints. Despite its practical relevance, the literature lacks studies on metaheuristic design for the MoTSPPP, and classical evolutionary operators may disrupt passenger rides. To fill this gap, this paper proposes the passenger-guided crossover (PGX), which uses the parents' passengers to select inherited vertices, and compares it with a passenger-agnostic one-point crossover (OPX). This paper also proposes four mutation operators, each targeting a different decision level, and three policies to combine them. We evaluated 23 MOEA/D variants with these crossover and mutation operators. Experiments covered 84 instances with 5 to 200 vertices and compared the top-ranked variants with exact and constructive approaches. The PGX significantly outperformed the OPX in solution quality and diversity, mainly on large instances. Mutation performance depended on the crossover, and combining mutations with tuned probabilities surpassed any single operator. The best variants were near-optimal on small instances and outperformed a constructive heuristic in most instances. These results shed light on the difficulty of the MoTSPPP and show that passenger information is key to variation operators for this problem.

## 📂 Repository structure

```
.
├── /Codes/                          # Source code (C++17)
│   ├── main.cpp, main.h, Makefile
│   ├── /moead/                      # MOEA/D and its external Pareto archive (adaptive grid)
│   ├── /operators/                  # Crossover and mutation operators
│   └── /utils/                      # Instance I/O, solution evaluation, population handling
├── /Instances/                      # Benchmark instances (symmetric and asymmetric)
├── /Results/
│   ├── /HV_IGD_vs_evaluations.csv                # HV and IGD along the run (every 1,000 evaluations)
│   ├── /HV_solver_metaheuristics.csv   # Final HV, running time and gap to the exact solver
│   ├── /Pareto_fronts/              # Pareto fronts: exact solver vs. each method
└── README.md
```

