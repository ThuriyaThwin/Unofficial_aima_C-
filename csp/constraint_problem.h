#pragma once

#include "pch.h"
#include "constraint.h"


namespace csp
{
	template<typename T>
	using AssignmentHistory = std::deque<std::pair<Ref<Variable<T>>, std::optional<T>>>;

	template <typename T>
	using VariableValuePair = std::pair<Ref<csp::Variable<T>>, T>;

	template<typename T>
	using Assignment = std::unordered_map<Ref<Variable<T>>, T>;

	template<typename T> class duplicate_constraint_error;

	template <typename T>
	class ConstraintProblem final
	{
	private:
		using VarToConstraintsMap = std::unordered_map<Ref<Variable<T>>, std::vector<Ref<Constraint<T>>>>;
		using AdjacencyListConstraintGraph = std::unordered_map<Ref<Variable<T>>, std::vector<Ref<Variable<T>>>>;
		using NameToVariableRefMap = std::unordered_map<std::string, Ref<Variable<T>>>;


		static const std::vector<Ref<Constraint<T>>> init_constraints(const std::vector<Ref<Constraint<T>>>& constraints)
		{
			std::unordered_set<Constraint<T>*> constraintsAddresses;
			constraintsAddresses.reserve(constraints.size());
			for (Constraint<T>& constr : constraints)
			{
				if (constraintsAddresses.count(&(constr)))
				{
					throw duplicate_constraint_error<T>(constr);
				}
				constraintsAddresses.emplace(&(constr));
			}
			std::vector<Ref<Constraint<T>>> myConstraints{ constraints };
			return myConstraints;
		}

		static const VarToConstraintsMap init_variableToConstraints(const std::vector<Ref<Constraint<T>>>& constraints) noexcept
		{
			VarToConstraintsMap variableToConstraints;
			for (Constraint<T>& constr : constraints)
			{
				const std::vector<Ref<Variable<T>>>& constraintVars = constr.getVariables();
				for (Variable<T>& var : constraintVars)
				{
					const auto [it, emplaced] = variableToConstraints.try_emplace(var, std::vector<Ref<Constraint<T>>>{ constr });
					const auto constrsEnd = it->second.cend();
					if (!emplaced && std::find(std::execution::par_unseq, it->second.cbegin(), constrsEnd, constr) == constrsEnd)
					{
						it->second.emplace_back(constr);
					}
				}
			}
			return variableToConstraints;
		}

		static const std::vector<Ref<Variable<T>>> init_variables(const VarToConstraintsMap& variableToConstraints) noexcept
		{
			std::vector<Ref<Variable<T>>> variables;
			for (const std::pair<Ref<Variable<T>>, std::vector<Ref<Constraint<T>>>>& varToConstraints : variableToConstraints)
			{
				variables.emplace_back(varToConstraints.first);
			}
			return variables;
		}

		static const AdjacencyListConstraintGraph init_constraintGraph(const VarToConstraintsMap& variableToConstraints) noexcept
		{
			AdjacencyListConstraintGraph constraintGraph;
			for (const std::pair<Ref<Variable<T>>, std::vector<Ref<Constraint<T>>>>& varToConstraints : variableToConstraints)
			{
				std::unordered_set<Ref<Variable<T>>> neighbors{ varToConstraints.first };
				for (Constraint<T>& constraint : varToConstraints.second)
				{
					const std::vector<Ref<Variable<T>>>& constraintVars = constraint.getVariables();
					for (Variable<T>& neighborVar : constraintVars)
					{
						neighbors.emplace(neighborVar);
					}
				}
				neighbors.erase(varToConstraints.first);

				if (constraintGraph.find(varToConstraints.first) == constraintGraph.end())
				{
					constraintGraph.emplace(varToConstraints.first, std::vector<Ref<Variable<T>>>{ neighbors.cbegin(), neighbors.cend() });
				}
				else
				{
					std::vector<Ref<Variable<T>>>& currNeighbors = constraintGraph.at(varToConstraints.first);
					const auto currNeighborsItToStart = currNeighbors.cbegin();
					const auto currNeighborsItToEnd = currNeighbors.cend();
					std::copy_if(neighbors.cbegin(), neighbors.cend(), std::back_inserter(currNeighbors),
						[&currNeighborsItToStart, &currNeighborsItToEnd](const Variable<T>& var) -> bool
						{ return std::find(std::execution::par_unseq, currNeighborsItToStart, currNeighborsItToEnd, var) != currNeighborsItToEnd; });
				}
			}

			return constraintGraph;
		}

		void init_allValues_and_allConsistentDomains(Variable<T>& var, std::unordered_set<T>& allValues,
			std::unordered_multiset<std::unordered_set<T>>& allConsistentDomains) const noexcept
		{
			const std::vector<Ref<Constraint<T>>>& constraintsContainingVar = m_umapVariableToConstraints.at(var);
			for (Constraint<T>& constraint : constraintsContainingVar)
			{
				const std::vector<T> currConsistentDomain = constraint.getConsistentDomainValues(var);
				allValues.insert(currConsistentDomain.cbegin(), currConsistentDomain.cend());
				allConsistentDomains.emplace(currConsistentDomain.cbegin(), currConsistentDomain.cend());
			}
		}

		std::vector<Ref<Constraint<T>>> m_vecConstraints;
		VarToConstraintsMap m_umapVariableToConstraints;
		std::vector<Ref<Variable<T>>> m_vecVariables;
		AdjacencyListConstraintGraph m_umapConstraintGraph;
		NameToVariableRefMap m_umapNameToVariableRef;

	public:
		ConstraintProblem<T>() = delete;
		ConstraintProblem<T>(const std::vector<Ref<Constraint<T>>>& constraints,
			const NameToVariableRefMap& umapNameToVariableRef = std::unordered_map<std::string, Ref<Variable<T>>>{}) :
			m_vecConstraints{ init_constraints(constraints) },
			m_umapVariableToConstraints{ init_variableToConstraints(m_vecConstraints) },
			m_vecVariables{ init_variables(m_umapVariableToConstraints) },
			m_umapConstraintGraph{ init_constraintGraph(m_umapVariableToConstraints) }, 
			m_umapNameToVariableRef{ umapNameToVariableRef }
		{ }

		ConstraintProblem<T>(const ConstraintProblem<T>& otherConstrProb) : 
			m_vecConstraints{ otherConstrProb.m_vecConstraints },
			m_umapVariableToConstraints{ otherConstrProb.m_umapVariableToConstraints }, 
			m_vecVariables{ otherConstrProb.m_vecVariables },
			m_umapConstraintGraph{ otherConstrProb.m_umapConstraintGraph }, 
			m_umapNameToVariableRef{ otherConstrProb.m_umapNameToVariableRef }
		{ }

		ConstraintProblem<T>& operator=(const ConstraintProblem<T>& otherConstrProb)
		{
			return *this = ConstraintProblem<T>(otherConstrProb);
		}


		ConstraintProblem<T>(ConstraintProblem<T>&& otherConstrProb) noexcept :
			m_vecConstraints{ std::move(otherConstrProb.m_vecConstraints) },
			m_umapVariableToConstraints{ std::move(otherConstrProb.m_umapVariableToConstraints) },
			m_vecVariables{ std::move(otherConstrProb.m_vecVariables) },
			m_umapConstraintGraph{ std::move(otherConstrProb.m_umapConstraintGraph) },
			m_umapNameToVariableRef{ std::move(otherConstrProb.m_umapNameToVariableRef) }
		{ }

		ConstraintProblem<T>& operator=(ConstraintProblem<T>&& otherConstrProb) noexcept
		{
			std::swap(m_vecConstraints, otherConstrProb.m_vecConstraints);
			std::swap(m_umapVariableToConstraints, otherConstrProb.m_umapVariableToConstraints);
			std::swap(m_vecVariables, otherConstrProb.m_vecVariables);
			std::swap(m_umapConstraintGraph, otherConstrProb.m_umapConstraintGraph);
			std::swap(m_umapNameToVariableRef, otherConstrProb.m_umapNameToVariableRef);
			return *this;
		}

		~ConstraintProblem<T>() = default;

		ConstraintProblem<T> deepCopy(std::vector<Variable<T>>& copiedVars,
			std::vector<Constraint<T>>& copiedConstraints) const
		{
			copiedVars.reserve(m_vecVariables.size());
			std::unordered_map<Variable<T>*, size_t> pOrigVarToCopiedVarIdx;
			pOrigVarToCopiedVarIdx.reserve(m_vecVariables.size());

			copiedConstraints.reserve(m_vecConstraints.size());
			std::vector<Ref<Constraint<T>>> copiedConstraintRefs;
			copiedConstraintRefs.reserve(m_vecConstraints.size());

			for (Constraint<T>& constr : m_vecConstraints)
			{
				const std::vector<Ref<Variable<T>>>& constraintVars = constr.getVariables();
				std::vector<Ref<Variable<T>>> constrCopiedVarRefs;
				constrCopiedVarRefs.reserve(constraintVars.size());
				for (Variable<T>& var : constraintVars)
				{
					if (!pOrigVarToCopiedVarIdx.count(&var))
					{
						copiedVars.push_back(var);
						pOrigVarToCopiedVarIdx.emplace(&var, copiedVars.size() - 1);
						constrCopiedVarRefs.emplace_back(copiedVars.back());
					}
					else
					{
						constrCopiedVarRefs.emplace_back(copiedVars[pOrigVarToCopiedVarIdx[&var]]);
					}
				}

				copiedConstraints.push_back({ constrCopiedVarRefs, constr.getConstraintEvaluator() });
				copiedConstraintRefs.emplace_back(copiedConstraints.back());
			}

			ConstraintProblem<T> copiedConstraintProblem{ copiedConstraintRefs };
			return copiedConstraintProblem;
		}

		const NameToVariableRefMap& getNameToVariableMap() const noexcept { return m_umapNameToVariableRef; }

		constexpr bool isCompletelyAssigned() const noexcept
		{
			for (const Variable<T>& var : m_vecVariables)
			{
				if (!var.isAssigned())
				{
					return false;
				}
			}
			return true;
		}

		constexpr bool isCompletelyUnassigned() const noexcept
		{
			for (const Variable<T>& var : m_vecVariables)
			{
				if (var.isAssigned())
				{
					return false;
				}
			}
			return true;
		}

		constexpr bool isConsistentlyAssigned() const noexcept
		{
			for (const Constraint<T>& constraint : m_vecConstraints)
			{
				if (!constraint.isConsistent())
				{
					return false;
				}
			}
			return true;
		}

		constexpr bool isCompletelyConsistentlyAssigned() const noexcept
		{
			bool probIsCompletelyConsistentlyAssigned = false;
			std::future<bool> probIsCompletelyAssigned = std::async(std::launch::async, &ConstraintProblem<T>::isCompletelyAssigned, this);
			std::future<bool> probisConsistentlyAssigned = std::async(std::launch::async, &ConstraintProblem<T>::isConsistentlyAssigned, this);
			if (probIsCompletelyAssigned.get() && probisConsistentlyAssigned.get())
			{
				probIsCompletelyConsistentlyAssigned = true;
			}
			return probIsCompletelyConsistentlyAssigned;
		}

		void unassignAllVariables()
		{
			for (Variable<T>& var : m_vecVariables)
			{
				var.unassign();
			}
		}

		const std::vector<Ref<Variable<T>>>& getVariables() const noexcept
		{
			return m_vecVariables;
		}

		const std::vector<Ref<Variable<T>>> getAssignedVariables() const noexcept
		{
			std::vector<Ref<Variable<T>>> assignedVariables;
			for (Variable<T>& var : m_vecVariables)
			{
				if (var.isAssigned())
				{
					assignedVariables.emplace_back(var);
				}
			}
			return assignedVariables;
		}

		const std::vector<Ref<Variable<T>>> getUnassignedVariables() const noexcept
		{
			std::vector<Ref<Variable<T>>> unassignedVariables;
			for (Variable<T>& var : m_vecVariables)
			{
				if (!var.isAssigned())
				{
					unassignedVariables.emplace_back(var);
				}
			}
			return unassignedVariables;
		}

		const std::vector<Ref<Variable<T>>> getNeighbors(Variable<T>& var) const
		{
			return m_umapConstraintGraph.at(var);
		}

		const std::vector<Ref<Variable<T>>> getAssignedNeighbors(Variable<T>& var) const
		{
			const std::vector<Ref<Variable<T>>>& neighbors = m_umapConstraintGraph.at(var);
			std::vector<Ref<Variable<T>>> assignedNeighbors;
			for (Variable<T>& neighborVar : neighbors)
			{
				if (neighborVar.isAssigned())
				{
					assignedNeighbors.emplace_back(neighborVar);
				}
			}
			return assignedNeighbors;
		}

		const std::vector<Ref<Variable<T>>> getUnassignedNeighbors(Variable<T>& var) const
		{
			const std::vector<Ref<Variable<T>>>& neighbors = m_umapConstraintGraph.at(var);
			std::vector<Ref<Variable<T>>> unassignedNeighbors;
			for (Variable<T>& neighborVar : neighbors)
			{
				if (!neighborVar.isAssigned())
				{
					unassignedNeighbors.emplace_back(neighborVar);
				}
			}
			return unassignedNeighbors;
		}

		const std::vector<Ref<Constraint<T>>>& getConstraints() const noexcept
		{
			return m_vecConstraints;
		}

		const std::vector<Ref<Constraint<T>>> getConsistentConstraints() const noexcept
		{
			std::vector<Ref<Constraint<T>>> consistentConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (constraint.isConsistent())
				{
					consistentConstraints.emplace_back(constraint);
				}
			}
			return consistentConstraints;
		}

		constexpr size_t getConsistentConstraintsSize() const noexcept
		{
			size_t consistentConstraintsSize = 0;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (constraint.isConsistent())
				{
					++consistentConstraintsSize;
				}
			}
			return consistentConstraintsSize;
		}

		const std::vector<Ref<Constraint<T>>> getInconsistentConstraints() const noexcept
		{
			std::vector<Ref<Constraint<T>>> inconsistentConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (!constraint.isConsistent())
				{
					inconsistentConstraints.emplace_back(constraint);
				}
			}
			return inconsistentConstraints;
		}

		const std::vector<Ref<Constraint<T>>> getSatisfiedConstraints() const noexcept
		{
			std::vector<Ref<Constraint<T>>> satisfiedConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (constraint.isSatisfied())
				{
					satisfiedConstraints.emplace_back(constraint);
				}
			}
			return satisfiedConstraints;
		}

		const std::vector<Ref<Constraint<T>>> getUnsatisfiedConstraints() const noexcept
		{
			std::vector<Ref<Constraint<T>>> unsatisfiedConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (!constraint.isSatisfied())
				{
					unsatisfiedConstraints.emplace_back(constraint);
				}
			}
			return unsatisfiedConstraints;
		}

		constexpr size_t getUnsatisfiedConstraintsSize() const noexcept
		{
			size_t unsatisfiedConstraintsSize = 0;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (!constraint.isSatisfied())
				{
					++unsatisfiedConstraintsSize;
				}
			}
			return unsatisfiedConstraintsSize;
		}

		const std::vector<Ref<Constraint<T>>>& getConstraintsContainingVariable(Variable<T>& var) const
		{
			return m_umapVariableToConstraints.at(var);
		}

		const std::vector<T> getConsistentDomain(Variable<T>& var) noexcept
		{
			std::unordered_set<T> allValues;
			std::unordered_multiset<std::unordered_set<T>> allConsistentDomains;
			this->init_allValues_and_allConsistentDomains(var, allValues, allConsistentDomains);

			std::unordered_set<T> usetConsistentDomain;
			usetConsistentDomain.reserve(allValues.size());
			for (T val : allValues)
			{
				bool value_in_intersection = true;
				for (const std::unordered_set<T>& consistentDomain : allConsistentDomains)
				{
					if (!consistentDomain.count(val))
					{
						value_in_intersection = false;
						break;
					}
				}

				if (!value_in_intersection)
				{
					continue;
				}
				usetConsistentDomain.emplace(val);
			}

			std::vector<T> vecConsistentDomain(usetConsistentDomain.cbegin(), usetConsistentDomain.cend());
			return vecConsistentDomain;
		}

		const Assignment<T> getCurrentAssignment() const noexcept
		{
			std::unordered_map<Ref<Variable<T>>, T> currAssignment;
			for (Variable<T>& var : m_vecVariables)
			{
				if (var.isAssigned())
				{
					currAssignment.emplace(var, var.getValue());
				}
			}
			return currAssignment;
		}

		void assignFromAssignment(const Assignment<T>& assignment) noexcept
		{
			for (const std::pair<Ref<Variable<T>>, T>& varToVal : assignment)
			{
				Variable<T>& variable = varToVal.first.get();
				if (!variable.isAssigned())
				{
					variable.assign(varToVal.second);
				}
				else if (variable.getValue() != varToVal.second)
				{
					variable.unassign();
					variable.assign(varToVal.second);
				}
			}
		}

		// CSPDO: write test
		void assignVarsWithRandomValues(std::optional<std::unordered_set<Ref<Variable<T>>>> optReadOnlyVars =
			std::optional<std::unordered_set<Ref<Variable<T>>>>{},
			std::optional<AssignmentHistory<T>> optAssignmentHistory = std::optional<AssignmentHistory<T>>{}) noexcept
		{
			for (Variable<T>& var : m_vecVariables)
			{
				bool varIsAssigned = var.isAssigned();
				if (varIsAssigned && optReadOnlyVars && (*optReadOnlyVars).count(var))
				{
					continue;
				}
				else if (varIsAssigned)
				{
					var.unassign();
					if (optAssignmentHistory)
					{
						(*optAssignmentHistory).emplace_back(var, std::optional<T>{});
					}
				}

				var.assignWithRandomlySelectedValue();
				if (optAssignmentHistory)
				{
					(*optAssignmentHistory).emplace_back(var, std::optional<T>{ var.getValue() });
				}
			}
		}

		const AdjacencyListConstraintGraph& getConstraintGraph() const noexcept { return m_umapConstraintGraph; }

		bool isPotentiallySolvable() noexcept
		{
			for (Variable<T>& var : m_vecVariables)
			{
				if (var.getDomain().empty() || this->getConsistentDomain(var).empty())
				{
					return false;
				}
			}
			return true;
		}

		friend std::ostream& operator<<(std::ostream& os, const ConstraintProblem<T>& constraintProblem) noexcept
		{
			os << '{';
			for (const Constraint<T>& constraint : constraintProblem.m_vecConstraints)
			{
				os << constraint << '\n';
			}
			os << "constraint problem is completely assigned: " << constraintProblem.isCompletelyAssigned() << ".\n";
			os << "constraint problem is consistently assigned: " << constraintProblem.isConsistentlyAssigned() << ".\n";
			os << "constraint problem is completely consistently assigned: " << constraintProblem.isCompletelyConsistentlyAssigned() << '}';
			return os;
		}

		constexpr std::string toString() const noexcept
		{
			std::ostringstream outStringStream;
			outStringStream << *this;
			return outStringStream.str();
		}

		friend bool operator==(const ConstraintProblem<T>& left, const ConstraintProblem<T>& right) noexcept
		{
			return &(left) == &(right);
		}

		friend bool operator!=(const ConstraintProblem<T>& left, const ConstraintProblem<T>& right) noexcept
		{
			return !(left == right);
		}
	};


	template<typename T>
	class duplicate_constraint_error : public std::domain_error
	{
	public:
		duplicate_constraint_error(const Constraint<T>& constr) : 
			std::domain_error{ constr.toString() + " is duplicate in input constraints vector." }
		{ }
	};
}


namespace csp
{
	template <typename T>
	struct std::hash<typename std::unordered_set<T>>
	{
		size_t operator()(const std::unordered_set<T>& uset) const
		{
			hash<T> hasher;
			size_t hashValue = 0;
			for (T elem : uset)
			{
				hashValue += hasher(elem);
			}
			return hashValue;
		}
	};
}


namespace std
{
	template <typename T>
	struct std::hash<typename csp::Assignment<T>>
	{
		size_t operator()(const csp::Assignment<T>& assignment) const
		{
			hash<T> valueHasher;
			hash<std::reference_wrapper<csp::Variable<T>>> variableHasher;
			double hashKey = 0;

			for (const std::pair<std::reference_wrapper<csp::Variable<T>>, T>& varToVal : assignment)
			{
				double variableHashValue = static_cast<double>(variableHasher(varToVal.first));
				double currHashValue = variableHashValue / valueHasher(varToVal.second);
				hashKey += currHashValue;
			}
			return static_cast<size_t>(hashKey);
		}
	};
}
