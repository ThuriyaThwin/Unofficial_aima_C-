#pragma once

#include "pch.h"
#include "constraint_problem.h"
#include "start_state_genarators.h"
#include "score_calculators.h"
#include "successor_generators.h"


namespace csp
{
	template <typename T>
	ConstraintProblem<T> simulatedAnnealing(ConstraintProblem<T>& constraintProblem,
		std::vector<Constraint<T>>& bestConstraints, std::vector<Variable<T>>& bestVars,
		unsigned int maxSteps, double temperature, double coolingRate,
		const StartStateGenarator<T>& generateStartState = generateStartStateRandomly<T>,
		const SuccessorGenerator<T>& generateSuccessor = alterRandomVariableValuePair<T>,
		const ScoreCalculator<T>& calculateScore = consistentConstraintsAmount<T>)
	{
		ConstraintProblem<T>& bestProblem = generateStartState(constraintProblem, bestVars, bestConstraints);
		ConstraintProblem<T>* pBestProblem = &(bestProblem);
		if (pBestProblem->isCompletelyConsistentlyAssigned() || maxSteps == 1)
		{
			return std::move(*pBestProblem);
		}
		--maxSteps;

		std::random_device randomDevice;
		std::default_random_engine defaultRandomEngine{ randomDevice() };
		std::uniform_real_distribution<double> zeroToOneDistribution(0.0, 1.0);

		unsigned int bestScore = calculateScore(*pBestProblem);
		std::vector<csp::Variable<T>> currVars;
		std::vector<csp::Constraint<T>> currConstrs;
		ConstraintProblem<T>& currConstrProb = pBestProblem->deepCopy(currVars, currConstrs);
		ConstraintProblem<T>* pCurrConstrProb = &(currConstrProb);
		for (unsigned int i = 0; i < maxSteps; ++i)
		{
			if (pCurrConstrProb->isCompletelyConsistentlyAssigned())
			{
				bestConstraints.clear();
				bestVars.clear();
				*pBestProblem = pCurrConstrProb->deepCopy(bestVars, bestConstraints);
				return std::move(*pBestProblem);
			}

			unsigned int currScore = calculateScore(*pCurrConstrProb);
			if (bestScore < currScore)
			{
				bestScore = currScore;
				bestConstraints.clear();
				bestVars.clear();
				*pBestProblem = pCurrConstrProb->deepCopy(bestVars, bestConstraints);
			}

			std::vector<csp::Variable<T>> successorVars;
			std::vector<csp::Constraint<T>> successorConstrs;
			ConstraintProblem<T>& successorProb = generateSuccessor(*pCurrConstrProb, successorVars, successorConstrs);
			unsigned int successorScore = calculateScore(successorProb);
			int delta = successorScore - currScore;
			if (0 < delta || zeroToOneDistribution(defaultRandomEngine) < std::exp(delta / temperature))
			{
				currConstrs.clear();
				currVars.clear();
				*pCurrConstrProb = successorProb.deepCopy(currVars, currConstrs);
			}
			temperature *= coolingRate;
		}

		return std::move(*pBestProblem);
	}
}