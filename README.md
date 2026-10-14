# Variation operators for the multi-objective traveling salesman problem with profits and passengers

This repository contains the implementation, benchmark instances and experimental results of a
study on **variation operators** (crossover and mutation) for the Multi-objective Traveling
Salesman Problem with Profits and Passengers (MoTSPPP), evaluated within **MOEA/D**.

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
│   ├── /Convergence/                # HV and IGD along the run (every 1,000 evaluations)
│   ├── /Solver_vs_metaheuristics/   # Final HV, running time and gap to the exact solver
│   ├── /Pareto_fronts/              # Pareto fronts: exact solver vs. each method
│   └── /Raw data/MOEAD/             # Raw MOEA/D output of every run
│       ├── /Final/                  # 6 variants × 84 instances × 20 runs
│       └── /Convergence/            # re-runs logging every 1,000 evaluations (100 and 200 vertices)
└── README.md
```

---

## 🧬 Variation operators

### Crossover

| Label | Operator | Description |
|---|---|---|
| **PGX** | `passenger_guided_insertion_crossover` | The child starts as a full copy of the first parent's route. Cities of the second parent that are not yet in the child and are origin/destination of a passenger actually served by that parent are inserted after the last common city, keeping their relative order. |
| **OPX** | `random_point_crossover` | One-point crossover with a randomly drawn cut point: the first part of the route comes from one parent and the remainder from the other, skipping repeated cities. |

### Mutation

| Label | `--mutation_mode` | Description |
|---|---:|---|
| `random` | 0 | Route mutation chosen at random, followed by bonus mutation. |
| `param` | 2 | Weighted roulette: a single mutation operator per call, with weights (summing to 100) tuned with irace for each crossover. |
| `mu3` | 5 | Remove-vertex operator only (μ3). |
| `mu4` | 7 | Bonus-flip operator only (μ4). |

Tuned roulette weights of `param` (irace, hypervolume):

| Crossover | swap | remove | add | bonus |
|---|---:|---:|---:|---:|
| PGX | 17 | 56 | 6 | 21 |
| OPX | 54 | 1 | 2 | 43 |

### Evaluated variants

| Variant | Crossover | Mutation | Label in the figures |
|---|---|---|---|
| `PGX+random` | PGX | random | — (convergence only) |
| `PGX+param` | PGX | param | PGX-T |
| `PGX+mu3` | PGX | mu3 | PGX-R |
| `OPX+random` | OPX | random | — (convergence only) |
| `OPX+param` | OPX | param | OPX-T |
| `OPX+mu4` | OPX | mu4 | OPX-B |

The results also include two reference methods: **Solver**, the exact solution of the
mixed-integer linear programming model (2-hour time limit), and **NH**, a constructive heuristic.

---

## ⚙️ Experimental setup

| Parameter | Value |
|---|---|
| Algorithm | MOEA/D (N = 300 subproblems, T = 20 neighbors) |
| Population size | 300 |
| Stopping criterion | 80,000 objective-function evaluations |
| Crossover / mutation rates | 56% / 53% |
| Instances | 84 (7 sizes: 5, 8, 10, 20, 50, 100, 200 vertices × 6 instances × symmetric/asymmetric) |
| Independent runs | 20 per instance and variant |

Objectives: route cost (f1, min), travel time (f2, min) and collected bonus (f3, max).

## 📏 Quality indicators

Each instance has its own reference: the objective space is normalized with the bounds of the
union of the fronts of Solver (when available), NH and the runs of `PGX+param`, `PGX+mu3`,
`OPX+param` and `OPX+mu4`, and the **reference set** is the non-dominated subset of that union.
`PGX+random` and `OPX+random` are measured against the same reference but do not take part in it.
Values are therefore comparable within an instance, not across instances.

- **HV (absolute)** — hypervolume of the front; higher is better.
- **HV (relative)** — `HV(reference set) − HV(front)`; ≥ 0, lower is better.
- **IGD** — inverted generational distance to the reference set; lower is better.
- **gap** — `100 · (HV_solver − HV_method) / HV_solver`, on absolute HV values.

---

## 📊 Results

### `Convergence/HV_IGD_vs_evaluations.csv`

HV and IGD of the archive every 1,000 evaluations, for the six variants, instances with 100 and
200 vertices (symmetric and asymmetric) and 20 runs (227,520 rows).

| Column | Description |
|---|---|
| `instance` | `<size>.<id>`, e.g. `200.1` |
| `type` | `sym` or `asym` |
| `variant` | one of the six variants |
| `run` | run number (1–20) |
| `evaluations` | number of evaluations (1,000, 2,000, …) |
| `hv` | **relative** HV (lower is better) |
| `igd` | IGD (lower is better) |

### `Solver_vs_metaheuristics/HV_solver_metaheuristics.csv`

Final results on all 84 instances (6,888 rows) for Solver, NH, `PGX+param`, `PGX+mu3`,
`OPX+param` and `OPX+mu4`.

| Column | Description |
|---|---|
| `instance`, `type` | as above |
| `method` | Solver, NH or one of the four variants |
| `run` | 1 for Solver and NH; 1–20 for the variants |
| `hv` | **absolute** HV (higher is better) |
| `time_s` | running time in seconds |
| `optimal` | Solver only: 1 = complete Pareto front proven within the time limit, 0 = not proven (partial or no front); `--` = not run |
| `gap` | gap to the Solver (%); `--` where the Solver has no front |

The Solver produced a front for 36 instances with 5, 8 and 10 vertices (complete in 21 of them,
partial in 15); for the remaining instances its HV and gap are `--`. On partial fronts the gap can
be negative, since the metaheuristics find solutions the solver did not reach.

### `Pareto_fronts/`

180 figures `<instance>_<sym|asym>_<method>.png` for the instances with 5, 8 and 10 vertices: the Solver front
(black crosses) against the front of one method (red dots) — `nh`, `pgx_param`, `pgx_mu3`,
`opx_param` or `opx_mu4`.

### `Raw data/MOEAD/`

Raw output of MOEA/D, one `.tar.gz` archive per variant (`PGX_random`, `PGX_param`, `PGX_mu3`,
`OPX_random`, `OPX_param`, `OPX_mu4`). Each archive extracts to

```
<variant>/instances/<symmetric|asymmetric>/<size>.<id>.in/test_<k>/
    data                       # run configuration, seed, running time, operator statistics
    paretogeneration_<g>.txt   # snapshot g of the external Pareto archive
```

Each line of a `paretogeneration_<g>.txt` file is one solution: `cost time bonus`. Run `test_<k>`
corresponds to `run = k + 1` in the CSV files.

| Folder | Content | Snapshots | Used in |
|---|---|---|---|
| `Final/` | all 84 instances, 20 runs (1,680 runs per variant) | every 10,000 evaluations; the highest `g` is the final front | `Solver_vs_metaheuristics/`, `Pareto_fronts/` |
| `Convergence/` | instances with 100 and 200 vertices, 20 runs (480 runs per variant), re-executed with the same seeds | every 1,000 evaluations: `g = 0` is the initial archive and `g = 1 … 79` correspond to 1,000 … 79,000 evaluations | `Convergence/` |

```bash
tar -xzf "Results/Raw data/MOEAD/Final/PGX_param.tar.gz"
```

---

## 🛠️ Building and running

```bash
cd Codes
make                            # produces ./main (g++ with C++17)
ln -s ../Instances instances    # the executable reads instances from ./instances/
```

Example — MOEA/D with PGX and the tuned mutation (`PGX+param`):

```bash
./main --crossover_mode guided --mutation_mode 2 \
       --prob_mut_swap 17 --prob_mut_remove 56 --prob_mut_add 6 --prob_mut_bonus 21 \
       --qtd_testes 20 --max_valuations 80000 --folder_name results/PGX_param
```

Main options:

| Option | Values |
|---|---|
| `--instance_type` | `symmetric`, `asymmetric`, `both` (default) |
| `--instance` | run a single instance, e.g. `instances/symmetric/10.1.in` |
| `--crossover_mode` | `guided` (PGX), `random_point` (OPX), plus other experimental operators |
| `--mutation_mode` | 0 random, 1 sequential, 2 param, 3 none, 4 swap, 5 remove (μ3), 6 add, 7 bonus (μ4) |
| `--prob_mut_swap/remove/add/bonus` | roulette weights of mode 2 |
| `--crossover`, `--mutation` | crossover and mutation rates (%) |
| `--qtd_testes` | number of independent runs |
| `--max_valuations` | evaluation budget |
| `--folder_name` | output folder |

Output: `<folder>/instances/<type>/<instance>.in/test_<k>/` with a `data` file (configuration, seed,
running time, operator statistics) and `paretogeneration_<g>.txt` snapshots of the external archive
(one solution per line: `cost time bonus`). Runs already finished are skipped, so an interrupted
execution can be resumed with the same command.
