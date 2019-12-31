#pragma once

#include "pch.h"
#include "constraint_problem.h"


namespace csp
{
	template <typename T>
	class BaseGeneticConstraintProblem
	{
		/* Whoever wants to use geneticLocalSearch must inherit this class.
		   See general_genetic_constraint_problem for an example. */	

	protected:
		ConstraintProblem<T> m_ConstraintProb;

		BaseGeneticConstraintProblem<T>(ConstraintProblem<T>& constrProb) : m_ConstraintProb{ constrProb }
		{ }

	public:
		BaseGeneticConstraintProblem<T>() = delete;
		
		BaseGeneticConstraintProblem<T>(const BaseGeneticConstraintProblem<T>& otherBaseGeneticProb): m_ConstraintProb{ otherBaseGeneticProb.m_ConstraintProb }
		{ }

		BaseGeneticConstraintProblem<T>& operator=(const BaseGeneticConstraintProblem<T>& otherBaseGeneticProb)
		{
			return *this = BaseGeneticConstraintProblem<T>(otherBaseGeneticProb);
		}

		BaseGeneticConstraintProblem<T>(BaseGeneticConstraintProblem<T>&& otherBaseGeneticProb) noexcept :
			m_ConstraintProb{ std::move(otherBaseGeneticProb.m_ConstraintProb) }
		{ }

		BaseGeneticConstraintProblem<T>& operator=(BaseGeneticConstraintProblem<T>&& otherBaseGeneticProb) noexcept
		{
			std::swap(m_ConstraintProb, otherBaseGeneticProb.m_ConstraintProb);
		}
		
		virtual ~BaseGeneticConstraintProblem<T>() = default;

		ConstraintProblem<T>& getConstraintProblem() { return m_ConstraintProb;  }

		virtual std::vector<Assignment<T>> generatePopulation(unsigned int populationSize) = 0;

		virtual unsigned int calculateFitness(const Assignment<T>& assignment) = 0;	/* Good fitness is high fitness*/
		
		virtual std::vector<Assignment<T>> performNaturalSelection(std::vector<Assignment<T>>& population) = 0;

		virtual std::vector<Assignment<T>> produceNextGeneration(const std::vector<Assignment<T>>& oldGeneration) = 0;

		virtual void mutateNextGeneration(std::vector<Assignment<T>>& population, double mutationProbability) = 0;

		virtual const std::optional<ConstraintProblem<T>> getSolution(const std::vector<Assignment<T>>& population)
		{
			std::optional<ConstraintProblem<T>> optSolution;
			for (const Assignment<T>& assignment : population)
			{
				m_ConstraintProb.unassignAllVariables();
				m_ConstraintProb.assignFromAssignment(assignment);
				if (m_ConstraintProb.isCompletelyConsistentlyAssigned())
				{
					optSolution = m_ConstraintProb;
					break;
				}
			}
			return optSolution;
		}
	};
}

