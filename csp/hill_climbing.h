#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	using StartStateGenarator = std::function<ConstraintProblem<T>(ConstraintProblem<T>&, 
		std::vector<Variable<T>>&, std::vector<Constraint<T>>&)>;

	template <typename T>	// good score is high score
	using ScoreCalculator = std::function<unsigned int(ConstraintProblem<T>&)>;

	template <typename T>
	using SuccessorGenerator = std::function<ConstraintProblem<T>(ConstraintProblem<T>&,
		std::vector<Variable<T>>&, std::vector<Constraint<T>>&)>;


	template <typename T>
	ConstraintProblem<T> generateStartStateRandomly(ConstraintProblem<T>& srcConstraintProblem,
		std::vector<Variable<T>>& destVars, std::vector<Constraint<T>>& destConstraints)
	{
		ConstraintProblem<std::string>& copiedConstraintProb = srcConstraintProblem.deepCopy(destVars, destConstraints);
		copiedConstraintProb.unassignAllVariables();
		copiedConstraintProb.assignVarsWithRandomValues();
		return std::move(copiedConstraintProb);
	}

	template <typename T>
	unsigned int consistentConstraintsAmount(ConstraintProblem<T>& constraintProblem)
	{
		return static_cast<unsigned int>(constraintProblem.getConsistentConstraintsSize());
	}

	template <typename T>
	ConstraintProblem<T> alterRandomVariableValuePair(ConstraintProblem<T>& srcConstraintProblem,
		std::vector<Variable<T>>& destVars, std::vector<Constraint<T>>& destConstraints)
	{
		ConstraintProblem<std::string>& copiedConstraintProb = srcConstraintProblem.deepCopy(destVars, destConstraints);
		Variable<T>& randomlySelectedVar = __selectElementRandomly<std::reference_wrapper<Variable<T>>, 
			std::vector<std::reference_wrapper<Variable<T>>>>(copiedConstraintProb.getVariables());
		std::optional<T> optOldValue;
		if (randomlySelectedVar.isAssigned())
		{
			optOldValue = randomlySelectedVar.getValue();
		}
		const std::vector<T>& varDomain = randomlySelectedVar.getDomain();
		T newValue = __selectElementRandomly<T, std::vector<T>>(varDomain);
		while (optOldValue && newValue == *optOldValue && 1 < varDomain.size())
		{
			newValue = __selectElementRandomly<T, std::vector<T>>(varDomain);
		}
		randomlySelectedVar.unassign();
		randomlySelectedVar.assign(newValue);
		return std::move(copiedConstraintProb);
	}
}

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