#pragma once

#include "pch.h"
#include "base_genetic_constraint_problem.h"

namespace csp
{
	template <typename T>
	void geneticLocalSearch(BaseGeneticConstraintProblem<T>& baseGeneticConstrProb,
		unsigned int populationSize, unsigned int maxGenerations, double mutationProbability)
	{
		ConstraintProblem<T>& constrProb = baseGeneticConstrProb.getConstraintProblem();

		std::vector<Assignment<T>> population = baseGeneticConstrProb.generatePopulation(populationSize);
		std::vector<Assignment<T>>* pCurrPopulation = &(population);

		Assignment<T>* pBestIndividual = &(pCurrPopulation->back());
		unsigned int bestFitness = 0;

		for (unsigned i = 0; i < maxGenerations; ++i)
		{
			const std::optional<ConstraintProblem<T>> optSolution = baseGeneticConstrProb.getSolution(*pCurrPopulation);
			if (optSolution)
			{
				return;
			}

			std::vector<Assignment<T>> selectedPopulation = baseGeneticConstrProb.performNaturalSelection(*pCurrPopulation);
			std::vector<Assignment<T>> nextGeneration = baseGeneticConstrProb.produceNextGeneration(selectedPopulation);
			baseGeneticConstrProb.mutateNextGeneration(nextGeneration, mutationProbability);
			*pCurrPopulation = std::move(nextGeneration);

			std::multimap<unsigned int, Ref<Assignment<T>>> scoreToIndividualMap;
			for (Assignment<T>& assignment : *pCurrPopulation)
			{
				scoreToIndividualMap.emplace(baseGeneticConstrProb.calculateFitness(assignment), assignment);
			}

			unsigned int currBestFitness = scoreToIndividualMap.crbegin()->first;
			if (bestFitness < currBestFitness)
			{
				bestFitness = currBestFitness;
				Assignment<T>& bestIndividual = scoreToIndividualMap.crbegin()->second;
				*pBestIndividual = bestIndividual;
				//*pBestIndividual = (scoreToIndividualMap.crbegin()->second).get();
			}
		}

		constrProb.unassignAllVariables();
		constrProb.assignFromAssignment(*pBestIndividual);
	}
}