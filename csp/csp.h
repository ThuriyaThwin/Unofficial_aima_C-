#pragma once


#include "initial_utilities.h"

// csp classes
#include "variable.h"
#include "constraint.h"
#include "constraint_problem.h"

// csp utilities
#include "constraint_evaluators.h"
#include "domain_sorters.h"
#include "unassigned_variable_selectors.h"
#include "start_state_genarators.h"
#include "successor_generators.h"
#include "score_calculators.h"
#include "base_genetic_constraint_problem.h"
#include "general_genetic_constraint_problem.h"

// csp inferences
#include "forward_checking.h"
#include "maintaining_arc_consistency.h"

// csp preprocessing
#include "arc_consistency_3.h"
#include "arc_consistency_4.h"
#include "path_consistency_2.h"

// csp solvers
#include "backtracking.h"
#include "heuristic_backtracking.h"
#include "min_conflicts.h"
#include "constraints_weighting.h"
#include "hill_climbing.h"
#include "simulated_annealing.h"
#include "general_genetic_constraint_problem.h"
#include "genetic_local_search.h"
#include "tree_csp_solver.h"
#include "naive_cycle_cutset.h"