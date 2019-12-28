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
		static const std::unordered_set<Variable<T>*> initVariableAddresses(const std::vector<Ref<Variable<T>>>& variables)
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
			return std::move(varsAddresses);
		}

		void enforceUnaryConstraint()
		{
			Variable<T>& var = m_vecVariables.back();
			if (!var.isAssigned())
			{
				const std::vector<T>& vecConsistentDomain = this->getConsistentDomainValues(var);
				var.setSubsetDomain(vecConsistentDomain);
			}
		}

		void verifyVariableIsContained(Variable<T>& var) const
		{
			if (!m_usetVariableAddresses.count(&(var)))
			{
				throw uncontained_variable_error<T>(*this, var);
			}
		}

		std::optional<T> getVariableOptValue(Variable<T>& var) const noexcept
		{
			std::optional<T> optValue;
			if (var.isAssigned())
			{
				optValue = var.getValue();
			}
			return std::move(optValue);
		}

		std::unordered_set<Variable<T>*> m_usetVariableAddresses;
		std::vector<Ref<Variable<T>>> m_vecVariables;
		using ConstraintEvaluator = std::function<bool(const std::vector<T>& assignedValues)>;
		ConstraintEvaluator m_ceEvaluateConstraint;

	public:
		Constraint() = delete;
		Constraint(const std::vector<Ref<Variable<T>>>& variables, const ConstraintEvaluator& evaluateConstraint) :
			m_usetVariableAddresses{ initVariableAddresses(variables) },
			m_vecVariables{ variables }, 
			m_ceEvaluateConstraint{ evaluateConstraint }
		{
			if (m_vecVariables.size() == 1)
			{
				enforceUnaryConstraint();
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
			this->verifyVariableIsContained(var);
			std::optional<T> optValue = this->getVariableOptValue(var);
			if (optValue)
			{
				var.unassign();
			}

			std::vector<T> consistentDomain;
			const std::vector<T>& domain = var.getDomain();
			consistentDomain.reserve(domain.size());
			for (T value : domain)
			{
				var.assign(value);
				if (this->isConsistent())
				{
					consistentDomain.emplace_back(value);
				}
				var.unassign();
			}

			if (optValue)
			{
				var.assign(*optValue);
			}
			return std::move(consistentDomain);
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