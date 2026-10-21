# Passenger-guided crossover and mutation operators for the multi-objective traveling salesman problem with profits and passengers

This repository contains the implementation, benchmark instances, and experimental data for the paper:  

> **"Passenger-guided crossover and mutation operators for the multi-objective traveling salesman problem with profits and passengers"**  
> *Submitted to [JOURNAL]*  

## 📄 Abstract  

In industrial plants, combining local services at production units with the transport of items along a single maintenance route can reduce costs and environmental impacts. The Multi-objective Traveling Salesman Problem with Profits and Passengers (MoTSPPP) models this setting, where each service yields a bonus, and each item is a passenger who shares the route cost. The MoTSPPP minimizes route cost and travel time while maximizing the collected bonus under capacity, budget, and time constraints. Despite its practical relevance, metaheuristic design for the MoTSPPP remains unexplored, and classical evolutionary operators may disrupt passenger rides. To fill this gap, this paper proposes the passenger-guided crossover (PGX), which uses the parents' passengers to select inherited vertices, and compares it with a standard passenger-agnostic one-point crossover (OPX). This paper also proposes four mutation operators targeting different decision levels and three combination policies (random, parameterized, and sequential). We address the effective integration of these operators within the multi-objective evolutionary algorithm based on decomposition (MOEA/D). Experiments involved 23 variants on 84 instances with 5 to 200 vertices and compared the top-ranked variants with exact and constructive approaches. The PGX significantly outperformed the OPX in both quality and diversity, particularly for large instances. Mutation performance depended on the crossover, and the parameterized policy, with tuned mutation probabilities, surpassed any single operator. The best variants were near-optimal on small instances and outperformed a constructive heuristic in most instances. These results reveal the difficulty of the MoTSPPP and show that passenger information is key to its variation operators.

## 📂 Repository structure

```
.
├── /Codes/                           # Source code (C++17)
│   ├── main.cpp, main.h, Makefile
│   ├── /moead/                       # MOEA/D and its external Pareto archive (adaptive grid)
│   ├── /operators/                   # Crossover and mutation operators
│   └── /utils/                       # Instance I/O, solution evaluation, population handling
├── /Instances/                       # Benchmark instances (symmetric and asymmetric)
├── /Results/
│   ├── /HV_IGD_vs_evaluations.csv    # HV and IGD along the run 
│   ├── /HV_solver_metaheuristics.csv # Final HV, running time and gap to the exact solver
│   ├── /Pareto_fronts/               # Pareto fronts: exact solver vs. each method
└── README.md
```

