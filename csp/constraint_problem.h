#pragma once

#include "pch.h"
#include "constraint.h"


namespace csp
{
	template<typename T>
	using AssignmentHistory = std::deque<std::pair<std::reference_wrapper<Variable<T>>, std::optional<T>>>;

	template <typename T>
	using VariableValuePair = std::pair<std::reference_wrapper<csp::Variable<T>>, T>;

	template<typename T> 
	using Assignment = std::unordered_map<std::reference_wrapper<Variable<T>>, T>;

	template<typename T> class duplicate_constraint_error;
	
	template <typename T>
	class ConstraintProblem final
	{
	private:
		using VariableRef = std::reference_wrapper<Variable<T>>;
		using VariableRefsVector = std::vector<std::reference_wrapper<Variable<T>>>;
		using ConstraintsRefsVector = std::vector<std::reference_wrapper<Constraint<T>>>;
		using VarToConstraintsMap = std::unordered_map<VariableRef, ConstraintsRefsVector>;
		using AdjacencListConstraintGraph = std::unordered_map<VariableRef, VariableRefsVector>;
		using NameToVariableRefMap = std::unordered_map<std::string, VariableRef>;


		static const ConstraintsRefsVector initConstraints(const ConstraintsRefsVector& constraints)
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
			ConstraintsRefsVector myConstraints{ constraints };
			return std::move(myConstraints);
		}

		static const VarToConstraintsMap initVariableToConstraints(const ConstraintsRefsVector& constraints) noexcept
		{
			VarToConstraintsMap variableToConstraints;
			for (Constraint<T>& constr : constraints)
			{
				const VariableRefsVector& constraintVars = constr.getVariables();
				for (Variable<T>& var : constraintVars)
				{
					const auto [it, emplaced] = variableToConstraints.try_emplace(var, ConstraintsRefsVector{ constr });
					if (!emplaced && std::find(it->second.cbegin(), it->second.cend(), constr) == it->second.cend())
					{
						it->second.emplace_back(constr);
					}
				}
			}
			return std::move(variableToConstraints);
		}

		static const VariableRefsVector initVariables(const VarToConstraintsMap& variableToConstraints) noexcept
		{
			VariableRefsVector variables;
			for (const std::pair<VariableRef, ConstraintsRefsVector>& varToConstraints : variableToConstraints)
			{
				variables.emplace_back(varToConstraints.first);
			}
			return std::move(variables);
		}

		static const AdjacencListConstraintGraph initConstraintGraph(const VarToConstraintsMap& variableToConstraints) noexcept
		{
			AdjacencListConstraintGraph constraintGraph;
			for (const std::pair<VariableRef, ConstraintsRefsVector>& varToConstraints : variableToConstraints)
			{
				std::unordered_set<VariableRef> neighbors{ varToConstraints.first };
				for (Constraint<T>& constraint : varToConstraints.second)
				{
					const VariableRefsVector& constraintVars = constraint.getVariables();
					for (Variable<T>& neighborVar : constraintVars)
					{
						neighbors.emplace(neighborVar);
					}
				}
				neighbors.erase(varToConstraints.first);

				if (constraintGraph.find(varToConstraints.first) == constraintGraph.end())
				{
					constraintGraph.emplace(varToConstraints.first, VariableRefsVector(neighbors.cbegin(), neighbors.cend()));
				}
				else
				{
					VariableRefsVector& currNeighbors = constraintGraph.at(varToConstraints.first);
					const auto& currNeighborsItToStart = currNeighbors.cbegin();
					const auto& currNeighborsItToEnd = currNeighbors.cend();
					std::copy_if(neighbors.cbegin(), neighbors.cend(), back_inserter(currNeighbors),
						[&currNeighborsItToStart, &currNeighborsItToEnd] (const Variable<T>& var) -> bool
						{ return std::find(currNeighborsItToStart, currNeighborsItToEnd, var) != currNeighborsItToEnd; });
				}
			}
			return std::move(constraintGraph);
		}

		void initAllValuesAndAllConsistentDomains(Variable<T>& var, std::unordered_set<T>& allValues,
			std::unordered_multiset<std::unordered_set<T>>& allConsistentDomains) const noexcept
		{
			const ConstraintsRefsVector& constraintsContainingVar = m_umapVariableToConstraints.at(var);
			for (Constraint<T>& constraint : constraintsContainingVar)
			{
				const std::vector<T>& currConsistentDomain = constraint.getConsistentDomainValues(var);
				allValues.insert(currConsistentDomain.cbegin(), currConsistentDomain.cend());
				allConsistentDomains.emplace(currConsistentDomain.cbegin(), currConsistentDomain.cend());
			}
		}

		ConstraintsRefsVector m_vecConstraints;							// vector<reference_wrapper<Constraint<T>>>
		VarToConstraintsMap m_umapVariableToConstraints;				// unordered_map<variable_ref, constraints_refs_vector>;
		VariableRefsVector m_vecVariables;								// vector<reference_wrapper<Variable<T>>>;
		AdjacencListConstraintGraph m_umapConstraintGraph;				// unordered_map<variable_ref, variables_refs_vector>;
		NameToVariableRefMap m_umapNameToVariableRef;					// unordered_map<string, variable_ref>;


	public:
		ConstraintProblem<T>() = delete;
		ConstraintProblem<T>(const ConstraintsRefsVector& constraints,
			const NameToVariableRefMap& umapNameToVariableRef = std::unordered_map<std::string, VariableRef>{}) :
			m_vecConstraints{ initConstraints(constraints) }, m_umapVariableToConstraints{ initVariableToConstraints(m_vecConstraints) },
			m_vecVariables{ initVariables(m_umapVariableToConstraints) },
			m_umapConstraintGraph{ initConstraintGraph(m_umapVariableToConstraints) }, m_umapNameToVariableRef{ umapNameToVariableRef } 
		{ }

		// CSPDO: write test
		ConstraintProblem<T>(const ConstraintProblem<T>& otherConstrProb): m_vecConstraints{ otherConstrProb.m_vecConstraints },
			m_umapVariableToConstraints{ otherConstrProb.m_umapVariableToConstraints }, m_vecVariables{ otherConstrProb.m_vecVariables },
			m_umapConstraintGraph{ otherConstrProb.m_umapConstraintGraph }, m_umapNameToVariableRef{ otherConstrProb.m_umapNameToVariableRef }
		{ }

		// CSPDO: write test
		ConstraintProblem<T>& operator=(const ConstraintProblem<T>& otherConstrProb)
		{
			return *this = ConstraintProblem<T>(otherConstrProb);
		}


		// CSPDO: write test
		ConstraintProblem<T>(ConstraintProblem<T>&& otherConstrProb) noexcept :
			m_vecConstraints{ std::move(otherConstrProb.m_vecConstraints) },
			m_umapVariableToConstraints{ std::move(otherConstrProb.m_umapVariableToConstraints) },
			m_vecVariables{ std::move(otherConstrProb.m_vecVariables) },
			m_umapConstraintGraph{ std::move(otherConstrProb.m_umapConstraintGraph) },
			m_umapNameToVariableRef{ std::move(otherConstrProb.m_umapNameToVariableRef) }
		{ }

		// CSPDO: write test
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
			std::vector<Constraint<T>>& copiedConstraints) const noexcept
		{
			copiedVars.reserve(m_vecVariables.size());
			std::unordered_map<Variable<T>*, size_t> origVarPtrToCopiedVarIdx;
			origVarPtrToCopiedVarIdx.reserve(m_vecVariables.size());
			copiedConstraints.reserve(m_vecConstraints.size());
			std::vector<std::reference_wrapper<Constraint<T>>> copiedConstraintRefs;
			copiedConstraintRefs.reserve(m_vecConstraints.size());
			
			for (Constraint<T>& constr : m_vecConstraints)
			{
				const std::vector<std::reference_wrapper<Variable<T>>>& constraintVars = constr.getVariables();
				std::vector<std::reference_wrapper<Variable<T>>> constrCopiedVarRefs;
				constrCopiedVarRefs.reserve(constraintVars.size());
				for (Variable<T>& var : constraintVars)
				{
					if (!origVarPtrToCopiedVarIdx.count(&var))
					{
						copiedVars.push_back(var);
						origVarPtrToCopiedVarIdx.emplace(&var, copiedVars.size() - 1);
						constrCopiedVarRefs.emplace_back(copiedVars.back());
					}
					else
					{
						constrCopiedVarRefs.emplace_back(copiedVars[origVarPtrToCopiedVarIdx[&var]]);
					}
				}

				copiedConstraints.emplace_back(constrCopiedVarRefs, constr.getConstraintEvaluator());
				copiedConstraintRefs.emplace_back(copiedConstraints.back());
			}

			ConstraintProblem<T> copiedConstraintProblem{ copiedConstraintRefs };
			return std::move(copiedConstraintProblem);
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
			std::future<bool>& probIsCompletelyAssigned = std::async(std::launch::async, &ConstraintProblem<T>::isCompletelyAssigned, this);
			std::future<bool>& probisConsistentlyAssigned = std::async(std::launch::async, &ConstraintProblem<T>::isConsistentlyAssigned, this);
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

		const VariableRefsVector& getVariables() const noexcept
		{
			return m_vecVariables;
		}

		const VariableRefsVector getAssignedVariables() const noexcept
		{
			VariableRefsVector assignedVariables;
			for (Variable<T>& var : m_vecVariables)
			{
				if (var.isAssigned())
				{
					assignedVariables.emplace_back(var);
				}
			}
			return std::move(assignedVariables);
		}

		const VariableRefsVector getUnassignedVariables() const noexcept
		{
			VariableRefsVector unassignedVariables;
			for (Variable<T>& var : m_vecVariables)
			{
				if (!var.isAssigned())
				{
					unassignedVariables.emplace_back(var);
				}
			}
			return std::move(unassignedVariables);
		}

		const VariableRefsVector getNeighbors(Variable<T>& var) const
		{
			return m_umapConstraintGraph.at(var);
		}

		const VariableRefsVector getAssignedNeighbors(Variable<T>& var) const
		{
			const VariableRefsVector& neighbors = m_umapConstraintGraph.at(var);
			VariableRefsVector assignedNeighbors;
			for (Variable<T>& neighborVar : neighbors)
			{
				if (neighborVar.isAssigned())
				{
					assignedNeighbors.emplace_back(neighborVar);
				}
			}
			return std::move(assignedNeighbors);
		}

		const VariableRefsVector getUnassignedNeighbors(Variable<T>& var) const
		{
			const VariableRefsVector& neighbors = m_umapConstraintGraph.at(var);
			VariableRefsVector unassignedNeighbors;
			for (Variable<T>& neighborVar : neighbors)
			{
				if (!neighborVar.isAssigned())
				{
					unassignedNeighbors.emplace_back(neighborVar);
				}
			}
			return std::move(unassignedNeighbors);
		}

		const ConstraintsRefsVector& getConstraints() const noexcept
		{
			return m_vecConstraints;
		}

		const ConstraintsRefsVector getConsistentConstraints() const noexcept
		{
			ConstraintsRefsVector consistentConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (constraint.isConsistent())
				{
					consistentConstraints.emplace_back(constraint);
				}
			}
			return std::move(consistentConstraints);
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

		const ConstraintsRefsVector getInconsistentConstraints() const noexcept
		{
			ConstraintsRefsVector inconsistentConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (!constraint.isConsistent())
				{
					inconsistentConstraints.emplace_back(constraint);
				}
			}
			return std::move(inconsistentConstraints);
		}

		const ConstraintsRefsVector getSatisfiedConstraints() const noexcept
		{
			ConstraintsRefsVector satisfiedConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (constraint.isSatisfied())
				{
					satisfiedConstraints.emplace_back(constraint);
				}
			}
			return std::move(satisfiedConstraints);
		}

		const ConstraintsRefsVector getUnsatisfiedConstraints() const noexcept
		{
			ConstraintsRefsVector unsatisfiedConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (!constraint.isSatisfied())
				{
					unsatisfiedConstraints.emplace_back(constraint);
				}
			}
			return std::move(unsatisfiedConstraints);
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

		const ConstraintsRefsVector& getConstraintsContainingVariable(Variable<T>& var) const
		{
			return m_umapVariableToConstraints.at(var);
		}

		const std::vector<T> getConsistentDomain(Variable<T>& var) noexcept
		{
			std::unordered_set<T> allValues;
			std::unordered_multiset<std::unordered_set<T>> allConsistentDomains;
			this->initAllValuesAndAllConsistentDomains(var, allValues, allConsistentDomains);

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
			return std::move(vecConsistentDomain);
		}

		const Assignment<T> getCurrentAssignment() const noexcept
		{
			std::unordered_map<VariableRef, T> currAssignment;
			for (Variable<T>& var : m_vecVariables)
			{
				if (var.isAssigned())
				{
					currAssignment.emplace(var, var.getValue());
				}
			}
			return std::move(currAssignment);
		}

		void assignFromAssignment(const Assignment<T>& assignment) noexcept
		{
			for (const std::pair<std::reference_wrapper<Variable<T>>, T>& varToVal : assignment)
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
		void assignVarsWithRandomValues(std::optional<std::unordered_set<std::reference_wrapper<Variable<T>>>> optReadOnlyVars = 
			std::optional<std::unordered_set<std::reference_wrapper<Variable<T>>>>{},
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

				T selectedValue = var.assignWithRandomlySelectedValue();
				if (optAssignmentHistory)
				{
					(*optAssignmentHistory).emplace_back(var, std::optional<T>{selectedValue});
				}
			}
		}

		const AdjacencListConstraintGraph& getConstraintGraph() const noexcept { return m_umapConstraintGraph; }

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
	};


	template<typename T>
	class duplicate_constraint_error : public std::domain_error
	{
	public:
		duplicate_constraint_error(const Constraint<T>& constr) : std::domain_error(constr.toString() + " is duplicate in input constraints vector.") { }
	};
}


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