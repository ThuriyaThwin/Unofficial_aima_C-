#pragma once

#include "pch.h"
#include "variable.h"


namespace csp
{
	template<typename T> class duplicate_variable_error;
	template<typename T> class uncontained_variable_error;


	template <typename T>
	class Constraint final
	{
	private:		
		static const std::unordered_set<Variable<T>*> init_varsAddresses(const std::vector<Ref<Variable<T>>>& variables)
		{
			std::unordered_set<Variable<T>*> varsAddresses;
			varsAddresses.reserve(variables.size());
			for (Variable<T>& var : variables)
			{
				if (varsAddresses.count(&var))
				{
					throw duplicate_variable_error<T>(var);
				}
				varsAddresses.insert(&var);
			}
			return varsAddresses;
		}

		void enforce_unary_constraint()
		{
			Variable<T>& var = m_vecVariables.back();
			if (!var.isAssigned())
			{
				const std::vector<T> vecConsistentDomain = this->getConsistentDomainValues(var);
				var.setSubsetDomain(vecConsistentDomain);
			}
		}

		void verify_variable_is_contained(Variable<T>& var) const
		{
			if (!m_usetVariableAddresses.count(&(var)))
			{
				throw uncontained_variable_error<T>(*this, var);
			}
		}

		std::unordered_set<Variable<T>*> m_usetVariableAddresses;
		std::vector<Ref<Variable<T>>> m_vecVariables;
		using ConstraintEvaluator = std::function<bool(const std::vector<T>& assignedValues)>;
		ConstraintEvaluator m_ceEvaluateConstraint;

	public:
		Constraint() = delete;
		Constraint(const std::vector<Ref<Variable<T>>>& variables, const ConstraintEvaluator& evaluateConstraint) :
			m_usetVariableAddresses{ init_varsAddresses(variables) },
			m_vecVariables{ variables }, 
			m_ceEvaluateConstraint{ evaluateConstraint }
		{
			if (m_vecVariables.size() == 1)
			{
				enforce_unary_constraint();
			}
		}

		Constraint(const Constraint<T>& otherConstraint) : 
			m_usetVariableAddresses{ otherConstraint.m_usetVariableAddresses },
			m_vecVariables{ otherConstraint.m_vecVariables }, 
			m_ceEvaluateConstraint{ otherConstraint.m_ceEvaluateConstraint }
		{ }

		Constraint<T>& operator=(const Constraint<T>& otherConstraint)
		{
			return *this = Constraint<T>(otherConstraint);
		}

		Constraint<T>(Constraint<T>&& otherConstraint) noexcept :
			m_usetVariableAddresses{ std::move(otherConstraint.m_usetVariableAddresses) },
			m_vecVariables{ std::move(otherConstraint.m_vecVariables) },
			m_ceEvaluateConstraint{ std::move(otherConstraint.m_ceEvaluateConstraint) }
		{ }

		Constraint<T>& operator=(Constraint<T>&& otherConstraint) noexcept
		{
			std::swap(m_usetVariableAddresses, otherConstraint.m_usetVariableAddresses);
			std::swap(m_vecVariables, otherConstraint.m_vecVariables);
			std::swap(m_ceEvaluateConstraint, otherConstraint.m_ceEvaluateConstraint);
			return *this;
		}

		~Constraint() = default;

		const std::vector<Ref<Variable<T>>>& getVariables() const noexcept { return m_vecVariables; }
		const ConstraintEvaluator& getConstraintEvaluator() const noexcept { return m_ceEvaluateConstraint; }

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

		constexpr bool isConsistent() const noexcept
		{
			std::vector<T> values;
			values.reserve(m_vecVariables.size());
			for (const Variable<T>& var : m_vecVariables)
			{
				if (var.isAssigned())
				{
					values.emplace_back(var.getValue());
				}
			}
			return m_ceEvaluateConstraint(values);
		}

		constexpr bool isSatisfied() const noexcept
		{
			std::vector<T> values;
			values.reserve(m_vecVariables.size());
			for (const Variable<T>& var : m_vecVariables)
			{
				if (var.isAssigned())
				{
					values.emplace_back(var.getValue());
				}
				else
				{
					return false;
				}
			}
			return m_ceEvaluateConstraint(values);
		}

		const std::vector<T> getConsistentDomainValues(Variable<T>& var) const
		{
			this->verify_variable_is_contained(var);

			size_t assignmentIdx = var.getAssignmentIdx();
			if (assignmentIdx != UNASSIGNED)
			{
				var.unassign();
			}

			std::vector<T> consistentDomain;
			const std::vector<T>& domain = var.getDomain();
			consistentDomain.reserve(domain.size());
			for (size_t i = 0; i < domain.size(); ++i)
			{
				var.assignByIdx(i);
				if (this->isConsistent())
				{
					consistentDomain.emplace_back(domain[i]);
				}
				var.unassign();
			}

			if (assignmentIdx != UNASSIGNED)
			{
				var.assignByIdx(assignmentIdx);
			}

			return consistentDomain;
		}

		friend std::ostream& operator<<(std::ostream& os, const Constraint<T>& constraint) noexcept
		{
			os << '[';
			for (const Variable<T>& var : constraint.getVariables())
			{
				os << var << '\n';
			}
			os << "constraint is completely assigned: " << constraint.isCompletelyAssigned();
			os << ". constraint is consistent: " << constraint.isConsistent();
			os << ". constraint is satisfied: " << constraint.isSatisfied() << ']';
			return os;
		}

		constexpr std::string toString() const noexcept
		{
			std::ostringstream outStringStream;
			outStringStream << *this;
			return outStringStream.str();
		}

		friend bool operator==(const Constraint<T>& left, const Constraint<T>& right) noexcept
		{
			return &(left) == &(right);
		}

		friend bool operator!=(const Constraint<T>& left, const Constraint<T>& right) noexcept
		{
			return !(left == right);
		}

		friend bool operator<(const Constraint<T>& left, const Constraint<T>& right) noexcept
		{
			return &(left) < &(right);
		}
	};


	template<typename T>
	class duplicate_variable_error : public std::domain_error
	{
	public:
		duplicate_variable_error(const Variable<T>& var) : 
			std::domain_error{ var.toString() + " is duplicate in input variables vector." }
		{ }
	};

	template<typename T>
	class uncontained_variable_error : public std::domain_error
	{
	public:
		uncontained_variable_error(const Constraint<T>& constr, const Variable<T>& var) :
			std::domain_error{ "Cannot return consistent domain of " + var.toString() + " since it's not contained in\n" 
			+ constr.toString() }
		{ }
	};
}




namespace std
{
	template <typename T>
	struct std::hash<typename csp::Constraint<T>>
	{
		size_t operator()(const csp::Constraint<T>& constr) const
		{
			csp::Constraint<T>* pConstr = const_cast<csp::Constraint<T>*>(&(constr));
			std::hash<csp::Constraint<T>*> constrPtrHasher;
			return constrPtrHasher(pConstr);
		}
	};
}

namespace std
{
	template <typename T>
	struct std::hash<typename std::reference_wrapper<csp::Constraint<T>>>
	{
		size_t operator()(const std::reference_wrapper<csp::Constraint<T>>& constrRef) const
		{
			std::hash<csp::Constraint<T>> constrHasher;
			return constrHasher(constrRef.get());
		}
	};
}