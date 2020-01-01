#pragma once

#include "pch.h"
#include "initial_utilities.h"


namespace csp
{
	template<typename T> class template_argument_T_not_writable_to_cout_error;
	template<typename T> class unassigned_value_extraction_error;
	template<typename T> class over_assignment_error;
	template<typename T> class uncontained_value_error;
	template<typename T> class domain_alteration_error;
	
	/*
	Assumptions on T:
	1. T has a copy constructor (used in Variable<T>::getValue()).
	2. hash<T> exists for T (used in ConstraintProblem<T>::getConsistentDomain(Variable<T>& var)).
	3. T is writable to ostream (used in friend ostream& operator<<(ostream& os, const Variable<T>& variable))
	Note: Assumptions 1 and 2 always hold by virtue of demanding unordered_set<T> in Variable<T>'s constructor.
		 Assumption 3 hold by the static method Variable<T>::initDomain(const unordered_set<T>& domain).
	*/
	template <typename T>
	class Variable final
	{
	private:
		static constexpr size_t UNASSIGNED = std::numeric_limits<size_t>::max();


		static optCompare<T> init_compare()
		{
			optCompare<T> optCompare;

			static_assert(__is_to_stream_writable<std::ostream, T>::value, "T must be writable to std::cout.");

			if constexpr (__is_comparable_to_T<T>::value &&
				std::is_same<__T_less_than_operator_return_value<T>, bool>::value)
			{
				optCompare = std::less<T>();
			}

			return optCompare;
		}

		std::vector<T> init_domain(const std::unordered_set<T>& domain) noexcept
		{
			std::vector<T> vecDomain{ domain.cbegin(), domain.cend() };

			if (m_optCompare)
			{
				std::sort(std::execution::par_unseq, vecDomain.begin(), vecDomain.end(), *m_optCompare);
			}

			return vecDomain;
		}

		//static const std::vector<T> init_domain(const std::unordered_set<T>& domain,
		//	const std::function<bool(T left, T right)>& compare) noexcept
		//{
		//	// CSPDO: test it
		//	if constexpr (!__is_to_stream_writable<std::ostream, T>::value)
		//	{
		//		throw template_argument_T_not_writable_to_cout_error<T>{};
		//	}

		//	std::vector<T> vecDomain{ domain.cbegin(), domain.cend() };
		//	std::sort(std::execution::par_unseq, vecDomain.begin(), vecDomain.end(), compare);
		//	return vecDomain;
		//}

		optCompare<T> m_optCompare;
		std::vector<T> m_vecDomain;
		size_t m_size_tValueIdx;
		
	public:
		Variable<T>() = delete;
		
		Variable<T>(const std::unordered_set<T>& domain) :
			m_optCompare{ init_compare() },
			m_vecDomain{ init_domain(domain) },
			m_funcCompare{ compare },
			m_size_tValueIdx{ UNASSIGNED }
		{ }

		/*Variable<T>(const std::unordered_set<T>& domain, 
			std::function<constexpr bool(T left, T right)> compare = std::less<T>()) :
			m_vecDomain{ init_domain(domain, compare) }, 
			m_funcCompare{ compare }, 
			m_size_tValueIdx{ UNASSIGNED }
		{ }*/

		~Variable<T>() = default;

		Variable<T>(const Variable<T>& otherVar) : 
			m_vecDomain{ otherVar.m_vecDomain }, 
			m_funcCompare{ otherVar.m_funcCompare }, 
			m_size_tValueIdx{ otherVar.m_size_tValueIdx }
		{ }

		Variable<T>& operator=(const Variable<T>& otherVar)
		{
			return *this = Variable<T>(otherVar);
		}

		Variable<T>(Variable<T>&& otherVar) noexcept : 
			m_vecDomain{ std::move(otherVar.m_vecDomain) },
			m_funcCompare{ std::move(otherVar.m_funcCompare) },
			m_size_tValueIdx{ otherVar.m_size_tValueIdx }
		{ }

		Variable<T>& operator=(Variable<T>&& otherVar) noexcept
		{
			std::swap(m_vecDomain, otherVar.m_vecDomain);
			std::swap(m_funcCompare, otherVar.m_funcCompare);
			std::swap(m_size_tValueIdx, otherVar.m_size_tValueIdx);
			return *this;
		}

		constexpr bool isAssigned() const noexcept { return m_size_tValueIdx != UNASSIGNED; }

		constexpr T getValue() const
		{
			if (!this->isAssigned())
			{
				throw unassigned_value_extraction_error<T>{ *this };
			}
			return m_vecDomain[m_size_tValueIdx];
		}

		void unassign() noexcept { m_size_tValueIdx = UNASSIGNED; }

		void assign(T val)
		{
			if (this->isAssigned())
			{
				throw over_assignment_error<T>{ *this };
			}
			const auto itToBeginDomain = m_vecDomain.cbegin();
			const auto itToValPosition = std::lower_bound(itToBeginDomain, m_vecDomain.cend(), val);
			if (*itToValPosition != val)
			{
				throw uncontained_value_error<T>{ *this, val};
			}
			else
			{
				m_size_tValueIdx = itToValPosition - itToBeginDomain;
			}
		}

		void assignWithRandomlySelectedValue()
		{
			if (this->isAssigned())
			{
				throw over_assignment_error<T>{ *this };
			}

			std::random_device randomDevice;
			std::default_random_engine defaultRandomEngine{ randomDevice() };
			std::uniform_int_distribution<size_t> zeroToDomainLenDistribution(0, m_vecDomain.size() - 1);
			m_size_tValueIdx = zeroToDomainLenDistribution(defaultRandomEngine);
		}

		const std::vector<T>& getDomain() const noexcept
		{
			return m_vecDomain;
		}

		void removeFromDomain(size_t idx)
		{
			if (this->isAssigned())
			{
				throw domain_alteration_error<T>(*this);
			}
			m_vecDomain.erase(m_vecDomain.begin() + idx);
			m_size_tValueIdx = UNASSIGNED;
		}

		bool setSubsetDomain(const std::vector<T>& vecSubsetDomain)
		{
			bool wasDomainShortened = false;
			if (this->isAssigned())
			{
				throw domain_alteration_error<T>{ *this };
			}

			if (m_vecDomain.size() <= vecSubsetDomain.size())
			{
				return wasDomainShortened;
			}

			std::unordered_set<T> usetDomain{ m_vecDomain.cbegin(), m_vecDomain.cend() };
			for (T value : vecSubsetDomain)
			{
				if (!usetDomain.count(value))
				{
					return wasDomainShortened;
				}
			}

			m_vecDomain = vecSubsetDomain;
			std::sort(std::execution::par_unseq, m_vecDomain.begin(), m_vecDomain.end(), m_funcCompare);
			m_size_tValueIdx = UNASSIGNED;
			wasDomainShortened = true;
			return wasDomainShortened;
		}

		friend std::ostream& operator<<(std::ostream& os, const Variable<T>& variable) noexcept
		{
			os << "(variable's value: ";
			if (variable.isAssigned())
			{
				os << variable.getValue();
			}
			else
			{
				os << "unassigned";
			}

			os << ", variable's domain: ";
			char sep = '\0';
			for (T elem : variable.m_vecDomain)
			{
				os << sep << elem;
				if (!sep)
				{
					sep = ' ';
				}
			}
			os << ')';
			return os;
		}

		constexpr std::string toString() const noexcept
		{
			std::ostringstream outStringStream;
			outStringStream << *this;
			return outStringStream.str();
		}

		friend bool operator==(const Variable<T>& left, const Variable<T>& right) noexcept
		{
			return &(left) == &(right);
		}

		friend bool operator!=(const Variable<T>& left, const Variable<T>& right) noexcept
		{
			return !(left == right);
		}

		static void constructFromNamesToEqualDomain(std::unordered_map<std::string, Variable<T>>& NameToVarUMap,
			const std::unordered_set<std::string>& names, const std::unordered_set<T>& domain) noexcept
		{
			NameToVarUMap.reserve(names.size());
			for (const std::string& varName : names)
			{
				NameToVarUMap.emplace(varName, domain);
			}
		}

		static std::unordered_map<std::string, Variable<T>> constructFromNamesToEqualDomain(const std::unordered_set<std::string>& names,
			const std::unordered_set<T>& domain) noexcept
		{
			std::unordered_map<std::string, Variable<T>> NameToVarUMap;
			Variable<T>::constructFromNamesToEqualDomain(NameToVarUMap, names, domain);
			return NameToVarUMap;
		}
	};


	template<typename T>
	class template_argument_T_not_writable_to_cout_error : public std::invalid_argument
	{
	public:
		template_argument_T_not_writable_to_cout_error() :
			invalid_argument{ "Given template argument T with the name: " + typeid(T).name() + " is not writable to std::cout." }
		{ }
	};

	template<typename T>
	class unassigned_value_extraction_error : public std::logic_error
	{
	public:
		unassigned_value_extraction_error(const Variable<T>& var) :
			logic_error{ "Extracting value of unassigned variable: " + var.toString() +
				", variable must be assigned for value extraction." }
		{ }
	};


	template<typename T>
	class over_assignment_error : public std::logic_error
	{
	public:
		over_assignment_error(const Variable<T>& var) : 
			logic_error{ "Over-assignment of an assigned variable: " + var.toString() + 
			", variable must be unassigned before assignment." }
		{ }
	};

	template<typename T>
	class domain_alteration_error : public std::logic_error
	{
	public:
		domain_alteration_error(const Variable<T>& var) : 
			std::logic_error{ "Tried to alter domain of assigned variable: " + var.toString() +
			", variable must be unassigned before domain could be altered." }
		{ }
	};

	template<typename T>
	class uncontained_value_error : public std::domain_error
	{
	private:
		static const std::string getValueStr(T value)
		{
			std::ostringstream outStringStream;
			outStringStream << value;
			return outStringStream.str();
		}

	public:
		uncontained_value_error(const Variable<T>& var, T value) : 
			std::domain_error{ "Cannot assign variable: " + var.toString()
			+ " with value: " + getValueStr(value) + " since it is not contained in variable's domain." }
		{ }
	};
}


namespace std
{
	template <typename T>
	struct std::hash<typename csp::Variable<T>>
	{
		size_t operator()(const csp::Variable<T>& var) const
		{
			csp::Variable<T>* pVar = const_cast<csp::Variable<T>*>(&(var));
			std::hash<csp::Variable<T>*> varPtrHasher;
			return varPtrHasher(pVar);
		}
	};
}

namespace std
{
	template <typename T>
	struct std::hash<typename std::reference_wrapper<csp::Variable<T>>>
	{
		size_t operator()(const std::reference_wrapper<csp::Variable<T>>& varRef) const
		{
			std::hash<csp::Variable<T>> varHasher;
			return varHasher(varRef.get());
		}
	};
}