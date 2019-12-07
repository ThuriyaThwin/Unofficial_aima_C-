#pragma once

#include "pch.h"
#include "constraint.h"


namespace csp
{
	template<typename T>
	using AssignmentHistory = std::deque<std::pair<std::reference_wrapper<Variable<T>>, std::optional<T>>>;


	template<typename T> class duplicate_constraint_error;
	template<typename T> using Assignment = std::unordered_map<std::reference_wrapper<Variable<T>>, T>;

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
			return myConstraints;
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
			return variableToConstraints;
		}

		static const VariableRefsVector initVariables(const VarToConstraintsMap& variableToConstraints)
		{
			VariableRefsVector variables;
			for (const std::pair<VariableRef, ConstraintsRefsVector>& varToConstraints : variableToConstraints)
			{
				variables.emplace_back(varToConstraints.first);
			}
			return variables;
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
					std::copy_if(neighbors.cbegin(), neighbors.cend(), back_inserter(currNeighbors),
						[&currNeighbors](const Variable<T>& var) -> bool
						{ return std::find(currNeighbors.cbegin(), currNeighbors.cend(), var) != currNeighbors.cend(); });
				}
			}
			return constraintGraph;
		}

		void initAllValuesAndAllConsistentDomains(Variable<T>& var, std::unordered_set<T>& allValues,
			std::unordered_multiset<std::unordered_set<T>>& allConsistentDomains)
		{
			const ConstraintsRefsVector& constraintsContainingVar = m_umapVariableToConstraints.at(var);
			for (Constraint<T>& constraint : constraintsContainingVar)
			{
				const std::vector<T>& currConsistentDomain = constraint.getConsistentDomainValues(var);
				allValues.insert(currConsistentDomain.cbegin(), currConsistentDomain.cend());
				allConsistentDomains.emplace(currConsistentDomain.cbegin(), currConsistentDomain.cend());
			}
		}

		const ConstraintsRefsVector m_vecConstraints;							// vector<reference_wrapper<Constraint<T>>>
		const VarToConstraintsMap m_umapVariableToConstraints;				// unordered_map<variable_ref, constraints_refs_vector>;
		const VariableRefsVector m_vecVariables;								// vector<reference_wrapper<Variable<T>>>;
		const AdjacencListConstraintGraph m_umapConstraintGraph;			// unordered_map<variable_ref, variables_refs_vector>;
		const NameToVariableRefMap m_umapNameToVariableRef;						// unordered_map<string, variable_ref>;

	public:
		ConstraintProblem() = delete;
		ConstraintProblem(const ConstraintsRefsVector& constraints,
			const NameToVariableRefMap& umapNameToVariableRef = {}) :
			m_vecConstraints{ initConstraints(constraints) }, m_umapVariableToConstraints{ initVariableToConstraints(m_vecConstraints) },
			m_vecVariables{ initVariables(m_umapVariableToConstraints) },
			m_umapConstraintGraph{ initConstraintGraph(m_umapVariableToConstraints) }, m_umapNameToVariableRef{ umapNameToVariableRef } { }

		ConstraintProblem(ConstraintProblem&&) = default;
		ConstraintProblem& operator=(ConstraintProblem&&) = default;
		ConstraintProblem(const ConstraintProblem&) = default;
		ConstraintProblem& operator=(const ConstraintProblem&) = default;
		~ConstraintProblem() = default;

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

		constexpr bool isConsistentlyAssigned() const  noexcept
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
			return this->isCompletelyAssigned() && this->isConsistentlyAssigned();
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
			return assignedVariables;
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
			return unassignedVariables;
		}

		const VariableRefsVector getNeighbors(Variable<T>& var)
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
			return assignedNeighbors;
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
			return unassignedNeighbors;
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
			return consistentConstraints;
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
			return inconsistentConstraints;
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
			return satisfiedConstraints;
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
			return unsatisfiedConstraints;
		}

		const size_t getUnsatisfiedConstraintsSize() const noexcept
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

		const std::vector<T> getConsistentDomain(Variable<T>& var)
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
			return vecConsistentDomain;
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
			return currAssignment;
		}

		void assignFromAssignment(const Assignment<T>& assignment)
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

		// TODO: write test
		void assignWithRandomValues(std::optional<std::unordered_set<std::reference_wrapper<Variable<T>>>> optReadOnlyVars = 
			std::optional<std::unordered_set<std::reference_wrapper<Variable<T>>>>{},
			std::optional<AssignmentHistory<T>> optAssignmentHistory = std::optional<AssignmentHistory<T>>{})
		{
			for (Variable<T>& var : m_vecVariables)
			{
				bool varIsAssigned = var.isAssigned();
				if (varIsAssigned && optReadOnlyVars && optReadOnlyVars.value().count(var))
				{
					continue;
				}
				else if (varIsAssigned)
				{
					var.unassign();
					if (optAssignmentHistory)
					{
						optAssignmentHistory.value().emplace_back(var, std::optional<T>{});
					}
				}

				const std::vector<T>& varDomain = var.getDomain();
				int randIdx = rand() % varDomain.size();
				T selectedValue = varDomain[randIdx];
				var.assign(selectedValue);
				if (optAssignmentHistory)
				{
					optAssignmentHistory.value().emplace_back(var, std::optional<T>{selectedValue});
				}
			}
		}

		const AdjacencListConstraintGraph& getConstraintGraph() const noexcept { return m_umapConstraintGraph; }

		bool isPotentiallySolvable()
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



/*
// https://stackoverflow.com/a/29855973
template <typename T>
struct hash<typename vector<T>>
{
	size_t operator()(const vector<T>& vec) const
	{
		std::hash<T> hasher;
		size_t seed = 0;
		for (const T& elem : vec)
		{
			seed ^= hasher(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};
*/

/*
// https://stackoverflow.com/a/29855973
template <typename T>
struct hash<typename unordered_set<T>>
{
	size_t operator()(const unordered_set<T>& uset) const
	{
		std::hash<T> hasher;
		size_t seed = 0;
		for (const T& elem : uset)
		{
			seed ^= hasher(elem) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}
};
*/

/*
template<typename T>
struct hash_on_sum : private std::hash<typename T::element_type>
{
	typedef T::element_type count_type;
	typedef std::hash<count_type> base;
	std::size_t operator()(T const& obj) const
	{
		return base::operator()(std::accumulate(obj.begin(), obj.end(), count_type()));
	}
};
*/


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

/*
template <typename T>
struct hash<typename reference_wrapper<Assignment<T>>>
{
	size_t operator()(const reference_wrapper<Assignment<T>>& assignment) const
	{
		hash<Assignment<T>> assignmentHasher;
		return assignmentHasher(assignment.get());
	}
};
*/