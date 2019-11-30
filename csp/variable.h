#pragma once

#include "pch.h"


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
	3. T is writeable to ostream (used in friend ostream& operator<<(ostream& os, const Variable<T>& variable))
	Note: Assumptions 1 and 2 always hold by virtue of demanding unordered_set<T> in Variable<T>'s constructor.
		 Assumption 3 hold by the static method Variable<T>::initDomain(const unordered_set<T>& domain).
	*/
	template <typename T>
	class Variable final
	{
	private:
		static const std::vector<T> initDomain(const std::unordered_set<T> & domain) noexcept
		{
			static_assert(is_to_stream_writable<std::ostream, T>::value);
			std::vector<T> vecDomain(domain.cbegin(), domain.cend());
			vecDomain.shrink_to_fit();
			return vecDomain;
		}


		std::vector<T> m_vecDomain;
		typename std::vector<T>::const_iterator m_itValue;
		typename std::vector<T>::const_iterator m_itEnd;
		/* if you don't plan on using preprocessing algorithms, an optimazation might be achieved by casting m_vecDomain and m_itEnd as
		const variables, and deleting the methods removeFromDomain. */

	public:
		Variable() = delete;
		Variable(const std::unordered_set<T> & domain) :
			m_vecDomain{ initDomain(domain) }, m_itValue{ m_vecDomain.cend() }, m_itEnd{ m_vecDomain.cend() } { }

		Variable(Variable&&) = default;
		Variable& operator=(Variable&&) = default;
		Variable(const Variable&) = default;
		Variable& operator=(const Variable&) = default;
		~Variable() = default;


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

		void assign(const T & value)
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

		const std::vector<T>& getDomain() const noexcept
		{
			return m_vecDomain;
		}

		// TODO: write unit test
		void setDomain(const std::vector<T> & domainToBeSet)
		{
			if (this->isAssigned())
			{
				throw domain_alteration_error<T>(*this);
			}
			m_vecDomain = domainToBeSet;
			m_itEnd = m_vecDomain.cend();
			m_itValue = m_itEnd;
		}

		// TODO: write unit test
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

		// TODO: write unit test
		void removeFromDomain(const typename std::vector<T>::const_iterator & it)
		{
			if (this->isAssigned())
			{
				throw domain_alteration_error<T>(*this);
			}
			m_vecDomain.erase(it);
			m_itEnd = m_vecDomain.cend();
			m_itValue = m_itEnd;
		}

		friend std::ostream& operator<<(std::ostream & os, const Variable<T> & variable) noexcept
		{
			os << "(variable's value: ";
			if (variable.isAssigned())
			{
				os << *(variable.m_itValue);
			}
			else
			{
				os << "unassgined";
			}

			os << ", variable's domain: ";
			std::string sep = "";
			for (const T& elem : variable.m_vecDomain)
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

		friend bool operator==(const Variable<T> & left, const Variable<T> & right)
		{
			return &(left) == &(right);
		}

		static void constructFromNamesToEqualDomain(std::unordered_map<std::string, Variable<T>> & NameToVarUMap,
			const std::unordered_set<std::string> & names, const std::unordered_set<T> & domain)
		{
			for (const std::string& varName : names)
			{
				NameToVarUMap.emplace(varName, domain);
			}
		}

		static std::unordered_map<std::string, Variable<T>> constructFromNamesToEqualDomain(const std::unordered_set<std::string> & names,
			const std::unordered_set<T> & domain)
		{
			std::unordered_map<std::string, Variable<T>> NameToVarUMap;
			Variable<T>::constructFromNamesToEqualDomain(NameToVarUMap, names, domain);
			return NameToVarUMap;
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