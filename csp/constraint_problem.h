#pragma once

#include "pch.h"
#include "constraint.h"


namespace csp
{
	template<typename T>
	using AssignmentHistory = std::deque<std::pair<Variable<T>&, std::optional<T>>>;


	template<typename T> class duplicate_constraint_error;
	template<typename T> using Assignment = std::unordered_map<std::reference_wrapper<Variable<T>>, T>;

	template <typename T>
	class ConstraintProblem final
	{
	private:
		using variable_ref = std::reference_wrapper<Variable<T>>;
		using variables_refs_vector = std::vector<std::reference_wrapper<Variable<T>>>;
		using constraints_refs_vector = std::vector<std::reference_wrapper<Constraint<T>>>;
		using vars_to_constraints_map = std::unordered_map<variable_ref, constraints_refs_vector>;
		using adjacency_list_constraint_graph = std::unordered_map<variable_ref, variables_refs_vector>;
		using name_to_var_ref_map = std::unordered_map<std::string, variable_ref>;


		static const constraints_refs_vector initConstraints(const constraints_refs_vector& constraints)
		{
			std::unordered_set<Constraint<T>*> constraintsAddresses;
			for (Constraint<T>& constr : constraints)
			{
				if (constraintsAddresses.count(&(constr)) == 1)
				{
					throw duplicate_constraint_error<T>(constr);
				}
				constraintsAddresses.insert(&(constr));
			}
			constraints_refs_vector myConstraints{ constraints };
			myConstraints.shrink_to_fit();
			return myConstraints;
		}

		static const vars_to_constraints_map initVariableToConstraints(const constraints_refs_vector& constraints) noexcept
		{
			vars_to_constraints_map variableToConstraints;
			for (Constraint<T>& constr : constraints)
			{
				const variables_refs_vector& constraintVars = constr.getVariables();
				for (Variable<T>& var : constraintVars)
				{
					const auto [it, emplaced] = variableToConstraints.try_emplace(var, constraints_refs_vector{ constr });
					if (!emplaced && std::find(it->second.cbegin(), it->second.cend(), constr) == it->second.cend())
					{
						it->second.emplace_back(constr);
					}
				}
			}
			return variableToConstraints;
		}

		static const variables_refs_vector initVariables(const vars_to_constraints_map& variableToConstraints)
		{
			variables_refs_vector variables;
			for (const std::pair<variable_ref, constraints_refs_vector>& varToConstraints : variableToConstraints)
			{
				variables.emplace_back(varToConstraints.first);
			}
			variables.shrink_to_fit();
			return variables;
		}

		static const adjacency_list_constraint_graph initConstraintGraph(const vars_to_constraints_map& variableToConstraints) noexcept
		{
			adjacency_list_constraint_graph constraintGraph;
			for (const std::pair<variable_ref, constraints_refs_vector>& varToConstraints : variableToConstraints)
			{
				std::unordered_set<variable_ref> neighbors{ varToConstraints.first };
				for (Constraint<T>& constraint : varToConstraints.second)
				{
					const variables_refs_vector& constraintVars = constraint.getVariables();
					for (Variable<T>& neighborVar : constraintVars)
					{
						neighbors.emplace(neighborVar);
					}
				}
				neighbors.erase(varToConstraints.first);

				if (constraintGraph.find(varToConstraints.first) == constraintGraph.end())
				{
					constraintGraph.emplace(varToConstraints.first, variables_refs_vector(neighbors.cbegin(), neighbors.cend()));
				}
				else
				{
					variables_refs_vector& currNeighbors = constraintGraph.at(varToConstraints.first);
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
			const constraints_refs_vector& constraintsContainingVar = m_umapVariableToConstraints.at(var);
			for (Constraint<T>& constraint : constraintsContainingVar)
			{
				const std::vector<T> currConsistentDomain = constraint.getConsistentDomainValues(var);
				allValues.insert(currConsistentDomain.cbegin(), currConsistentDomain.cend());
				allConsistentDomains.emplace(currConsistentDomain.cbegin(), currConsistentDomain.cend());
			}
		}

		const constraints_refs_vector m_vecConstraints;							// vector<reference_wrapper<Constraint<T>>>
		const vars_to_constraints_map m_umapVariableToConstraints;				// unordered_map<variable_ref, constraints_refs_vector>;
		const variables_refs_vector m_vecVariables;								// vector<reference_wrapper<Variable<T>>>;
		const adjacency_list_constraint_graph m_umapConstraintGraph;			// unordered_map<variable_ref, variables_refs_vector>;
		const name_to_var_ref_map m_umapNameToVariableRef;						// unordered_map<string, variable_ref>;


	public:
		ConstraintProblem() = delete;
		ConstraintProblem(const constraints_refs_vector& constraints,
			const name_to_var_ref_map& umapNameToVariableRef = {}) :
			m_vecConstraints{ initConstraints(constraints) }, m_umapVariableToConstraints{ initVariableToConstraints(m_vecConstraints) },
			m_vecVariables{ initVariables(m_umapVariableToConstraints) },
			m_umapConstraintGraph{ initConstraintGraph(m_umapVariableToConstraints) }, m_umapNameToVariableRef{ umapNameToVariableRef } { }

		ConstraintProblem(ConstraintProblem&&) = default;
		ConstraintProblem& operator=(ConstraintProblem&&) = default;
		ConstraintProblem(const ConstraintProblem&) = default;
		ConstraintProblem& operator=(const ConstraintProblem&) = default;
		~ConstraintProblem() = default;

		const name_to_var_ref_map& getNameToVariableMap() const noexcept { return m_umapNameToVariableRef; }

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

		const variables_refs_vector& getVariables() const noexcept
		{
			return m_vecVariables;
		}

		const variables_refs_vector getAssignedVariables() const noexcept
		{
			variables_refs_vector assignedVariables;
			for (Variable<T>& var : m_vecVariables)
			{
				if (var.isAssigned())
				{
					assignedVariables.emplace_back(var);
				}
			}
			return assignedVariables;
		}

		const variables_refs_vector getUnassignedVariables() const noexcept
		{
			variables_refs_vector unassignedVariables;
			for (Variable<T>& var : m_vecVariables)
			{
				if (!var.isAssigned())
				{
					unassignedVariables.emplace_back(var);
				}
			}
			return unassignedVariables;
		}

		const variables_refs_vector getNeighbors(Variable<T>& var)	// TODO: why can't const?
		{
			return m_umapConstraintGraph.at(var);
		}

		const variables_refs_vector getAssignedNeighbors(Variable<T>& var) const
		{
			const variables_refs_vector& neighbors = m_umapConstraintGraph.at(var);
			variables_refs_vector assignedNeighbors;
			for (Variable<T>& neighborVar : neighbors)
			{
				if (neighborVar.isAssigned())
				{
					assignedNeighbors.emplace_back(neighborVar);
				}
			}
			return assignedNeighbors;
		}

		const variables_refs_vector getUnassignedNeighbors(Variable<T>& var) const
		{
			const variables_refs_vector& neighbors = m_umapConstraintGraph.at(var);
			variables_refs_vector unassignedNeighbors;
			for (Variable<T>& neighborVar : neighbors)
			{
				if (!neighborVar.isAssigned())
				{
					unassignedNeighbors.emplace_back(neighborVar);
				}
			}
			return unassignedNeighbors;
		}

		const constraints_refs_vector& getConstraints() const noexcept
		{
			return m_vecConstraints;
		}

		const constraints_refs_vector getConsistentConstraints() const noexcept
		{
			constraints_refs_vector consistentConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (constraint.isConsistent())
				{
					consistentConstraints.emplace_back(constraint);
				}
			}
			return consistentConstraints;
		}

		const constraints_refs_vector getInconsistentConstraints() const noexcept
		{
			constraints_refs_vector inconsistentConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (!constraint.isConsistent())
				{
					inconsistentConstraints.emplace_back(constraint);
				}
			}
			return inconsistentConstraints;
		}

		const constraints_refs_vector getSatisfiedConstraints() const noexcept
		{
			constraints_refs_vector satisfiedConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (constraint.isSatisfied())
				{
					satisfiedConstraints.emplace_back(constraint);
				}
			}
			return satisfiedConstraints;
		}

		const constraints_refs_vector getUnsatisfiedConstraints() const noexcept
		{
			constraints_refs_vector unsatisfiedConstraints;
			for (Constraint<T>& constraint : m_vecConstraints)
			{
				if (!constraint.isSatisfied())
				{
					unsatisfiedConstraints.emplace_back(constraint);
				}
			}
			return unsatisfiedConstraints;
		}

		const constraints_refs_vector& getConstraintsContainingVariable(Variable<T>& var) const
		{
			return m_umapVariableToConstraints.at(var);
		}

		const std::vector<T> getConsistentDomain(Variable<T>& var)
		{
			std::unordered_set<T> allValues;
			std::unordered_multiset<std::unordered_set<T>> allConsistentDomains;
			this->initAllValuesAndAllConsistentDomains(var, allValues, allConsistentDomains);

			std::unordered_set<T> usetConsistentDomain;
			for (const T& val : allValues)
			{
				bool value_in_intersection = true;
				for (const std::unordered_set<T>& consistentDomain : allConsistentDomains)
				{
					if (consistentDomain.count(val) == 0)
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
			std::unordered_map<variable_ref, T> currAssignment;
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

		const adjacency_list_constraint_graph& getConstraintGraph() const noexcept { return m_umapConstraintGraph; }

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
		for (const T& elem : uset)
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