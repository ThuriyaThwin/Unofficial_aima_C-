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
		for (T value : assignedValues)
		{
			if (seenValues.count(value))
			{
				return false;
			}
			seenValues.emplace(value);
		}
		return true;
	}
}