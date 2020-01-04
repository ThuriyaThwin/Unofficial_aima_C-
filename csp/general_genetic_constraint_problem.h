#pragma once

#include "pch.h"
#include "base_genetic_constraint_problem.h"


namespace csp
{
	template <typename T>
	class GeneralGeneticConstraintProblem : public BaseGeneticConstraintProblem<T>
	{
	private:
		std::unordered_set<Ref<Variable<T>>> m_usetReadOnlyVars;

		Assignment<T> reproduce(const Assignment<T>& firstParent, const Assignment<T>& secondParent) const
		{
			std::random_device randomDevice;
			std::default_random_engine defaultRandomEngine{ randomDevice() };
			std::uniform_real_distribution<double> zeroToOneDistribution(0.0, 1.0);
			Assignment<T> child;
			for (Variable<T>& var : this->m_ConstraintProb.getVariables())
			{
				if (zeroToOneDistribution(defaultRandomEngine) < 0.5)
				{
					child.emplace(var, firstParent.at(std::ref(var)));
				}
				else
				{
					child.emplace(var, secondParent.at(std::ref(var)));
				}
			}
			return child;
		}

		void mutate(Assignment<T>& assignment) noexcept
		{
			/* for each selected mutation variable assign a new value randomly. */
			size_t numberOfMutations = static_cast<size_t>(assignment.size() * m_dMutationFraction);
			if (!numberOfMutations)
			{
				return;
			}
			const std::vector<Ref<Variable<T>>>& variables = this->m_ConstraintProb.getVariables();
			std::vector<Ref<Variable<T>>> varsToBeSampledFrom;
			varsToBeSampledFrom.reserve(variables.size() - m_usetReadOnlyVars.size());
			for (Variable<T>& var : variables)
			{
				if (m_usetReadOnlyVars.count(var))
				{
					continue;
				}
				varsToBeSampledFrom.emplace_back(var);
			}
			
			std::random_device randomDevice;
			std::default_random_engine randomDefaultEng{ randomDevice() };
			std::vector<Ref<Variable<T>>> sampledVars;
			std::sample(varsToBeSampledFrom.cbegin(), varsToBeSampledFrom.cend(), std::back_inserter(sampledVars), 
				numberOfMutations, randomDefaultEng);

			for (Variable<T>& var : sampledVars)
			{
				T oldVal = var.getValue();
				size_t domainSize = var.getDomain().size();
				var.unassign();
				var.assignWithRandomlySelectedValue();
				T newVal = var.getValue();
				while (1 < domainSize && newVal == oldVal)
				{
					var.unassign();
					var.assignWithRandomlySelectedValue();
					newVal = var.getValue();
				}
			}
		}


	public:
		double m_dMutationFraction;

		GeneralGeneticConstraintProblem<T>() = delete;

		GeneralGeneticConstraintProblem<T>(ConstraintProblem<T>& constrProb, double mutationFraction, 
			std::unordered_set<Ref<Variable<T>>> readOnlyVars = std::unordered_set<Ref<Variable<T>>>{}) :
			BaseGeneticConstraintProblem<T>{ constrProb }, 
			m_dMutationFraction{ mutationFraction }, 
			m_usetReadOnlyVars{ readOnlyVars }
		{ }

		GeneralGeneticConstraintProblem<T>(const GeneralGeneticConstraintProblem<T>& other): 
			BaseGeneticConstraintProblem<T>{ other.m_ConstraintProb }, 
			m_dMutationFraction{ other.m_dMutationFraction},
			m_usetReadOnlyVars{ other.m_usetReadOnlyVars }
		{ }

		GeneralGeneticConstraintProblem<T>& operator=(const GeneralGeneticConstraintProblem<T>& other)
		{
			return *this = GeneralGeneticConstraintProblem<T>(other);
		}

		GeneralGeneticConstraintProblem<T>(GeneralGeneticConstraintProblem<T>&& other) noexcept :
			BaseGeneticConstraintProblem<T>{ std::move(other.m_ConstraintProb) },
			m_dMutationFraction{ other.m_dMutationFraction },
			m_usetReadOnlyVars{ std::move(other.m_usetReadOnlyVars) }
		{ }

		GeneralGeneticConstraintProblem<T>& operator=(GeneralGeneticConstraintProblem<T>&& other) noexcept
		{
			std::swap(this->m_ConstraintProb, other.m_ConstraintProb);
			std::swap(m_usetReadOnlyVars, other.m_usetReadOnlyVars);
			std::swap(m_dMutationFraction, other.m_dMutationFraction);
		}

		~GeneralGeneticConstraintProblem<T>() = default;

		std::vector<Assignment<T>> generatePopulation(unsigned int populationSize) override
		{
			/* generating individuals by random assignments. */
			std::vector<Assignment<T>> population;
			population.reserve(populationSize);
			for (unsigned i = 0; i < populationSize; ++i)
			{
				for (Variable<T>& var : this->m_ConstraintProb.getVariables())
				{
					if (m_usetReadOnlyVars.count(var))
					{
						continue;
					}
					var.assignWithRandomlySelectedValue();
				}
				population.emplace_back(this->m_ConstraintProb.getCurrentAssignment());
				this->m_ConstraintProb.unassignAllVariables();
			}
			return population;
		}

		unsigned int calculateFitness(const Assignment<T>& assignment) override
		{
			/* fitness is the number of consistent constraints. */
			this->m_ConstraintProb.unassignAllVariables();
			this->m_ConstraintProb.assignFromAssignment(assignment);
			return static_cast<unsigned int>(this->m_ConstraintProb.getConsistentConstraintsSize());
		}
		
		std::vector<Assignment<T>> performNaturalSelection(std::vector<Assignment<T>>& population) override
		{
			/* half truncation selection. */
			std::multimap<unsigned int, Ref<Assignment<T>>> scoreToIndividualMap;
			for (Assignment<T>& assignment : population)
			{
				scoreToIndividualMap.emplace(this->calculateFitness(assignment), assignment);
			}

			std::vector<Assignment<T>> selectedPopulation;
			size_t trunctedPopulationSize = population.size() >> 1;
			selectedPopulation.reserve(trunctedPopulationSize);
			for (auto it = scoreToIndividualMap.crbegin(); 0 < trunctedPopulationSize; --trunctedPopulationSize, ++it)
			{
				selectedPopulation.emplace_back(it->second);
			}

			return selectedPopulation;
		}

		std::vector<Assignment<T>> produceNextGeneration(const std::vector<Assignment<T>>& oldGeneration) override
		{
			std::random_device randomDevice;
			std::default_random_engine randomDefaultEng{ randomDevice() };

			size_t nextGenerationSize = oldGeneration.size() << 1;
			std::vector<Assignment<T>> nextGeneration;
			nextGeneration.reserve(nextGenerationSize);
			for ( ; 0 < nextGenerationSize; --nextGenerationSize)
			{
				std::vector<Assignment<T>> parents{ 2 };
				std::sample(oldGeneration.cbegin(), oldGeneration.cend(), std::back_inserter(parents), 2, randomDefaultEng);
				nextGeneration.emplace_back(this->reproduce(parents.front(), parents.back()));
			}
			return nextGeneration;
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