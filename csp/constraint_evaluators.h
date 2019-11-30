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
		for (size_t i = 0; i < assignedValues.size() - 1; ++i)
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
		for (T value : assignedValues)
		{
			if (seenValues.count(value) == 1)
			{
				return false;
			}
			seenValues.insert(value);
		}
		return true;
	}
}