#pragma once

#include "pch.h"


// MEDOs in general:
// switch syntax of all "constructor(container.cbegin(), container.cend())" to constructor{ container.cbegin(), container.cend() }"
/* figure out a way to know in compile time whether we should use "for (T val : values)" or "for (const T& val : values).
   same goes for functions returning T by-value instead of by reference, like Variabke.getValue() for example. std::is_fundamental might be useful. */
// allow users to determine it they want the whole assignment history or simply the number of assignments and un-assignments.
// rewrite various solvers so that they'll output their assignment history.


// MEDOs for c++20:
// use coroutines in backtracking
// use wait and notify on atomics in ConstraintProblem<T>::isCompletelyConsistentlyAssigned()


template <typename T, typename Container>
T __selectElementRandomly(const Container& containerIn)
{
	Container containerOut;
	std::sample(containerIn.cbegin(), containerIn.cend(), std::inserter(containerOut, containerOut.end()), 1,
		std::default_random_engine{ std::random_device{}() });
	return *(containerOut.cbegin());
}

// https://stackoverflow.com/a/49026811
template<typename S, typename T, typename = void>
struct is_to_stream_writable : std::false_type
{ };

template<typename S, typename T>
struct is_to_stream_writable<S, T, std::void_t<decltype(std::declval<S&>() << std::declval<T>())>> : std::true_type
{ };

namespace csp
{
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
		static const std::vector<T> initDomain(const std::unordered_set<T>& domain) noexcept
		{
			static_assert(is_to_stream_writable<std::ostream, T>::value);
			// MEDO: also write an exception 
			std::vector<T> vecDomain(domain.cbegin(), domain.cend());
			return std::move(vecDomain);
		}


		std::vector<T> m_vecDomain;
		typename std::vector<T>::const_iterator m_itValue;
		typename std::vector<T>::const_iterator m_itEnd;


	public:
		Variable<T>() = delete;
		Variable<T>(const std::unordered_set<T>& domain):
			m_vecDomain{ initDomain(domain) }, m_itValue{ m_vecDomain.cend() }, m_itEnd{ m_vecDomain.cend() } { }
		~Variable<T>() = default;

		// MEDO: write test
		Variable<T>(const Variable<T>& otherVar): m_vecDomain{ otherVar.m_vecDomain }, m_itEnd{ m_vecDomain.cend() }
		{
			if (otherVar.m_itValue == otherVar.m_itEnd)
			{
				m_itValue = m_itEnd;
			}
			else
			{
				m_itValue = std::find(m_vecDomain.cbegin(), m_itEnd, *(otherVar.m_itValue));
			}
		}

		// MEDO: write test
		Variable<T>& operator=(const Variable<T>& otherVar)
		{
			return *this = Variable<T>(otherVar);
		}
		
		Variable<T>(Variable<T>&& otherVar) noexcept : m_vecDomain{ std::move(otherVar.m_vecDomain) },
			m_itValue{ std::move(otherVar.m_itValue) }, m_itEnd{ std::move(otherVar.m_itEnd) }
		{ }
		
		// MEDO: write test
		Variable<T>& operator=(Variable<T>&& otherVar) noexcept
		{
			std::swap(m_vecDomain, otherVar.m_vecDomain);
			std::swap(m_itValue, otherVar.m_itValue);
			std::swap(m_itEnd, otherVar.m_itEnd);
			return *this;
		}
		

		constexpr bool isAssigned() const noexcept { return m_itValue != m_itEnd; }

		constexpr T getValue() const
		{
			if (!this->isAssigned())
			{
				throw unassigned_value_extraction_error<T>(*this);
			}
			return *m_itValue;
		}

		void unassign() noexcept { m_itValue = m_itEnd; }

		void assign(const T& value)
		{
			if (this->isAssigned())
			{
				throw over_assignment_error<T>(*this);
			}

			typename std::vector<T>::const_iterator searchResult = find(m_vecDomain.cbegin(), m_itEnd, value);
			if (searchResult == m_itEnd)
			{
				throw uncontained_value_error<T>(*this, value);
			}
			m_itValue = searchResult;
		}

		T assignWithRandomlySelectedValue()
		{
			T selectedValue = __selectElementRandomly<T, std::vector<T>>(m_vecDomain);
			this->assign(selectedValue);
			return selectedValue;
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
			m_itEnd = m_vecDomain.cend();
			m_itValue = m_itEnd;
		}

		bool setSubsetDomain(const std::vector<T>& vecSubsetDomain)
		{
			if (this->isAssigned())
			{
				throw domain_alteration_error<T>(*this);
			}

			std::unordered_set<T> usetDomain(m_vecDomain.cbegin(), m_vecDomain.cend());
			for (T value : vecSubsetDomain)
			{
				if (!usetDomain.count(value))
				{
					return false;
				}
			}

			m_vecDomain = vecSubsetDomain;
			m_itEnd = m_vecDomain.cend();
			m_itValue = m_itEnd;
			return true;
		}

		friend std::ostream& operator<<(std::ostream& os, const Variable<T>& variable) noexcept
		{
			os << "(variable's value: ";
			if (variable.isAssigned())
			{
				os << *(variable.m_itValue);
			}
			else
			{
				os << "unassigned";
			}

			os << ", variable's domain: ";
			std::string sep = "";
			for (T elem : variable.m_vecDomain)
			{
				os << sep << elem;
				if (sep.empty())
				{
					sep = " ";
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
			return std::move(NameToVarUMap);
		}
	};


	template<typename T>
	class unassigned_value_extraction_error : public std::logic_error
	{
	public:
		unassigned_value_extraction_error(const Variable<T>& var) :
			logic_error("Extracting value of unassigned variable: " + var.toString() +
				", variable must be assigned for value extraction.") { }
	};


	template<typename T>
	class over_assignment_error : public std::logic_error
	{
	public:
		over_assignment_error(const Variable<T>& var) : logic_error("Over-assignment of an assigned variable: " + var.toString() +
			", variable must be unassigned before assignment.") { }
	};

	template<typename T>
	class domain_alteration_error : public std::logic_error
	{
	public:
		domain_alteration_error(const Variable<T>& var) : std::logic_error("Tried to alter domain of assigned variable: " + var.toString() +
			", variable must be unassigned before domain could be altered.") { }
	};

	template<typename T>
	class uncontained_value_error : public std::domain_error
	{
	private:
		static const std::string getValueStr(const T& value)
		{
			std::ostringstream outStringStream;
			outStringStream << value;
			return outStringStream.str();
		}

	public:
		uncontained_value_error(const Variable<T>& var, const T& value) : std::domain_error("Cannot assign variable: " + var.toString()
			+ " with value: " + getValueStr(value) + " since it is not contained in variable's domain.") { }
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