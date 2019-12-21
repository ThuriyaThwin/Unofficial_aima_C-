#pragma once

#include "pch.h"
#include "base_genetic_constraint_problem.h"


namespace csp
{
	template <typename T>
	class GeneralGeneticConstraintProblem : public BaseGeneticConstraintProblem<T>
	{
	private:
		std::vector<std::reference_wrapper<Variable<T>>> m_vecReadOnlyVars;

		Assignment<T> reproduce(const Assignment<T>& firstParent, const Assignment<T>& secondParent) const
		{
			std::random_device randomDevice;
			std::default_random_engine defaultRandomEngine{ randomDevice() };
			std::uniform_real_distribution<double> zeroToOneDistribution(0.0, 1.0);
			Assignment<T> child;
			for (Variable<T>& var : m_ConstraintProb.getVariables())
			{
				if (zeroToOneDistribution(defaultRandomEngine) < 0.5)
				{
					child.emplace(var, firstParent.at(var));
				}
				else
				{
					child.emplace(var, secondParent.at(var));
				}
			}
			return std::move(child);
		}

		void mutate(Assignment<T>& assignment) noexcept
		{
			/* for each selected mutation variable assign a new value randomly. */
			size_t numberOfMutations = static_cast<size_t>(assignment.size() * m_dMutationFraction);
			if (!numberOfMutations)
			{
				return;
			}
			const std::vector<std::reference_wrapper<Variable<T>>>& variables = m_ConstraintProb.getVariables();
			std::vector<std::reference_wrapper<Variable<T>>> varsToBeSampledFrom;
			varsToBeSampledFrom.reserve(variables.size() - m_vecReadOnlyVars.size());
			const auto& itToReadOnlyBegin = m_vecReadOnlyVars.cbegin();
			const auto& itToReadOnlyEnd = m_vecReadOnlyVars.cend();
			for (Variable<T>& var : variables)
			{
				if (std::find(itToReadOnlyBegin, itToReadOnlyEnd, var) != itToReadOnlyEnd)
				{
					continue;
				}
				varsToBeSampledFrom.emplace_back(var);
			}
			
			std::random_device randomDevice;
			std::default_random_engine randomDefaultEng{ randomDevice() };
			std::vector<std::reference_wrapper<Variable<T>>> sampledVars;
			std::sample(varsToBeSampledFrom.cbegin(), varsToBeSampledFrom.cend(), std::back_inserter(sampledVars), numberOfMutations, randomDefaultEng);

			for (Variable<T>& var : sampledVars)
			{
				T oldVal = var.getValue();
				size_t domainSize = var.getDomain().size();
				var.unassign();
				T newVal = var.assignWithRandomlySelectedValue();
				while (1 < domainSize && newVal == oldVal)
				{
					var.unassign();
					newVal = var.assignWithRandomlySelectedValue();
				}
			}
		}


	public:
		double m_dMutationFraction;

		GeneralGeneticConstraintProblem<T>() = delete;

		GeneralGeneticConstraintProblem<T>(ConstraintProblem<T>& constrProb, double mutationFraction, 
			std::vector<std::reference_wrapper<Variable<T>>> readOnlyVars = std::vector<std::reference_wrapper<Variable<T>>>{}) :
			BaseGeneticConstraintProblem<T>{ constrProb }, m_dMutationFraction{ mutationFraction }, m_vecReadOnlyVars{ readOnlyVars }
		{ }

		GeneralGeneticConstraintProblem<T>(const GeneralGeneticConstraintProblem<T>& other): 
			BaseGeneticConstraintProblem<T>{ other.m_ConstraintProb }, m_dMutationFraction{ other.m_dMutationFraction},
			m_vecReadOnlyVars{ other.m_vecReadOnlyVars }
		{ }

		GeneralGeneticConstraintProblem<T>& operator=(const GeneralGeneticConstraintProblem<T>& other)
		{
			return *this = GeneralGeneticConstraintProblem<T>(other);
		}

		GeneralGeneticConstraintProblem<T>(GeneralGeneticConstraintProblem<T>&& other) noexcept :
			BaseGeneticConstraintProblem<T>{ std::move(other.m_ConstraintProb) },
			m_dMutationFraction{ other.m_dMutationFraction },
			m_vecReadOnlyVars{ std::move(other.m_vecReadOnlyVars) }
		{ }

		GeneralGeneticConstraintProblem<T>& operator=(GeneralGeneticConstraintProblem<T>&& other) noexcept
		{
			std::swap(m_ConstraintProb; , other.m_ConstraintProb);
			std::swap(m_vecReadOnlyVars, other.m_vecReadOnlyVars);
			std::swap(m_dMutationFraction, other.m_dMutationFraction);
		}

		~GeneralGeneticConstraintProblem<T>() = default;

		std::vector<Assignment<T>> generatePopulation(unsigned int populationSize) override
		{
			/* generating individuals by random assignments. */
			std::vector<Assignment<T>> population;
			population.reserve(populationSize);
			const auto& vecReadOnlyItToBegin = m_vecReadOnlyVars.cbegin();
			const auto& vecReadOnlyItToEnd = m_vecReadOnlyVars.cend();
			for (unsigned i = 0; i < populationSize; ++i)
			{
				for (Variable<T>& var : m_ConstraintProb.getVariables())
				{
					if (std::find(vecReadOnlyItToBegin, vecReadOnlyItToEnd, var) != vecReadOnlyItToEnd)
					{
						continue;
					}
					var.assignWithRandomlySelectedValue();
				}
				population.emplace_back(m_ConstraintProb.getCurrentAssignment());
				m_ConstraintProb.unassignAllVariables();
			}
			return std::move(population);
		}

		unsigned int calculateFitness(const Assignment<T>& assignment) override
		{
			/* fitness is the number of consistent constraints. */
			m_ConstraintProb.unassignAllVariables();
			m_ConstraintProb.assignFromAssignment(assignment);
			return static_cast<unsigned int>(m_ConstraintProb.getConsistentConstraintsSize());
		}
		
		std::vector<Assignment<T>> performNaturalSelection(std::vector<Assignment<T>>& population) override
		{
			/* half truncation selection. */
			std::multimap<unsigned int, std::reference_wrapper<Assignment<T>>> scoreToIndividualMap;
			for (Assignment<T>& assignment : population)
			{
				scoreToIndividualMap.emplace(this->calculateFitness(assignment), assignment);
			}

			std::vector<Assignment<T>> selectedPopulation;
			size_t trunctedPopulationSize = population.size() >> 1;
			selectedPopulation.reserve(trunctedPopulationSize);
			size_t i = 0;
			for (auto& it = scoreToIndividualMap.crbegin(); i < trunctedPopulationSize; ++i, ++it)
			{
				selectedPopulation.emplace_back(it->second);
			}

			return std::move(selectedPopulation);
		}

		std::vector<Assignment<T>> produceNextGeneration(const std::vector<Assignment<T>>& oldGeneration) override
		{
			std::random_device randomDevice;
			std::default_random_engine randomDefaultEng{ randomDevice() };

			size_t nextGenerationSize = oldGeneration.size() << 1;
			std::vector<Assignment<T>> nextGeneration;
			nextGeneration.reserve(nextGenerationSize);
			for (size_t i = 0; i < nextGenerationSize; ++i)
			{
				std::vector<Assignment<T>> parents;
				parents.reserve(2);
				std::sample(oldGeneration.cbegin(), oldGeneration.cend(), std::back_inserter(parents), 2, randomDefaultEng);
				nextGeneration.emplace_back(this->reproduce(parents[0], parents[1]));
			}
			return std::move(nextGeneration);
		}

		void mutateNextGeneration(std::vector<Assignment<T>>& population, double mutationProbability) override
		{
			std::random_device randomDevice;
			std::default_random_engine defaultRandomEngine{ randomDevice() };
			std::uniform_real_distribution<double> zeroToOneDistribution(0.0, 1.0);

			for (Assignment<T>& assignment : population)
			{
				if (zeroToOneDistribution(defaultRandomEngine) < mutationProbability)
				{
					this->mutate(assignment);
				}
			}
		}
	};
}