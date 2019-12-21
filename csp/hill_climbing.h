#pragma once

#include "pch.h"
#include "constraint_problem.h"
#include "start_state_genarators.h"
#include "score_calculators.h"
#include "successor_generators.h"


namespace csp
{
	template <typename T>
	ConstraintProblem<T> randomRestartFirstChoiceHillClimbing(ConstraintProblem<T>& constraintProblem,
		std::vector<Constraint<T>>& bestConstraints, std::vector<Variable<T>>& bestVars,
		unsigned int maxRestarts, unsigned int maxSteps, unsigned int maxSuccessors,
		const StartStateGenarator<T>& generateStartState = generateStartStateRandomly<T>,
		const SuccessorGenerator<T>& generateSuccessor = alterRandomVariableValuePair<T>,
		const ScoreCalculator<T>& calculateScore = consistentConstraintsAmount<T>)
	{
		ConstraintProblem<T>& bestProblem = generateStartState(constraintProblem, bestVars, bestConstraints);
		ConstraintProblem<T>* pBestProblem = &(bestProblem);
		if (pBestProblem->isCompletelyConsistentlyAssigned() || maxRestarts == 1)
		{
			return std::move(*pBestProblem);
		}
		--maxRestarts;

		unsigned int bestScore = calculateScore(*pBestProblem);
		
		for (unsigned int i = 0; i < maxRestarts; ++i)
		{
			std::vector<csp::Variable<std::string>> currVars;
			std::vector<csp::Constraint<std::string>> currConstrs;
			ConstraintProblem<T>& currConstrProb = generateStartState(*pBestProblem, currVars, currConstrs);
			ConstraintProblem<T>* pCurrConstrProb = &(currConstrProb);
			for (unsigned int j = 0; j < maxSteps; ++j)
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

				for (unsigned int k = 0; k < maxSuccessors; ++k)
				{
					std::vector<csp::Variable<std::string>> successorVars;
					std::vector<csp::Constraint<std::string>> successorConstrs;
					ConstraintProblem<T>& successorProb = generateSuccessor(*pCurrConstrProb, successorVars, successorConstrs);
					unsigned int successorScore = calculateScore(successorProb);
					if (currScore < successorScore)
					{
						currConstrs.clear();
						currVars.clear();
						*pCurrConstrProb = successorProb.deepCopy(currVars, currConstrs);
						break;
					}
				}
			}
		}

		return std::move(*pBestProblem);
	}
}