#pragma once

#include "pch.h"


namespace csp
{
	template <typename T>
	bool alwaysSatisfied(const std::vector<T>& assignedValues)
	{
		return true;
	}

	template <typename T>
	bool neverSatisfied(const std::vector<T>& assignedValues)
	{
		return false;
	}

	template <typename T>
	bool allEqual(const std::vector<T>& assignedValues)
	{
		T lastValue = assignedValues.back();
		for (size_t i = 0; i < assignedValues.size() - 2; ++i)
		{
			if (assignedValues[i] != lastValue)
			{
				return false;
			}
		}
		return true;
	}

	template <typename T>
	bool allDiff(const std::vector<T>& assignedValues)
	{
		std::unordered_set<T> seenValues;
		seenValues.reserve(assignedValues.size());
		for (size_t i = 0; i < assignedValues.size(); ++i)
		{
			if (seenValues.count(assignedValues[i]))
			{
				return false;
			}
			seenValues.emplace(assignedValues[i]);
		}
		return true;
	}

	template <typename T>
	struct ExactLengthExactSum
	{
		unsigned int valuesAmount;
		T targetSum;

		ExactLengthExactSum(unsigned int _valsAmount, PassType<T> valuesSum): valuesAmount{ _valsAmount }, targetSum{ valuesSum }
		{
			static_assert(std::is_default_constructible_v<T>, "T must be default constructible for std::accumulate");
		}

		bool operator()(const std::vector<T>& assignedValues) const noexcept
		{
			if (assignedValues.size() < valuesAmount)
			{
				return true;
			}
			else if (assignedValues.size() == valuesAmount)
			{
				T sum = std::accumulate(assignedValues.cbegin(), assignedValues.cend(), T{ });
				return sum == targetSum;
			}
			else
			{
				return false;
			}
		}
	};

	template <typename T>
	struct TimeDelayer
	{
		T timeDelay;

		TimeDelayer(T __delayTime): timeDelay{ __delayTime }
		{ }

		bool operator()(const std::vector<T>& assignedValues) const noexcept
		{
			if (assignedValues.size() < 2)
			{
				return true;
			}
			else
			{
				return assignedValues[0] + timeDelay <= assignedValues[1];
			}
		}
	};
}