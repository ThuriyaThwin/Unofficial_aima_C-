#include "pch.h"
#include "einstein_five_house_riddle_problem.h"


static bool hintOne(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& nationality = assignedValues[0];
    const std::string& color = assignedValues[1];
    return nationality != "brit" || color == "red";
}

static bool hintTwo(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& nationality = assignedValues[0];
    const std::string& pet = assignedValues[1];
    return nationality != "swede" || pet == "dogs";
}

static bool hintThree(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& nationality = assignedValues[0];
    const std::string& drink = assignedValues[1];
    return nationality != "dane" || drink == "tea";
}

static bool hintFour_a(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& firstColor = assignedValues[0];
    const std::string& secondColor = assignedValues[1];
    return firstColor != "green" || secondColor  == "white";
}

static bool hintFour_b(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.empty())
    {
        return true;
    }
    return assignedValues[0] != "green";
}

static bool hintFive(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& color = assignedValues[0];
    const std::string& drink = assignedValues[1];
    return color != "green" || drink == "coffee";
}

static bool hintSix(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& smoke = assignedValues[0];
    const std::string& pet = assignedValues[1];
    return smoke != "pallmall" || pet == "birds";
}

static bool hintSeven(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& color = assignedValues[0];
    const std::string& smoke = assignedValues[1];
    return color != "yellow" || smoke == "dunhill";
}

static bool hintEight(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.empty())
    {
        return true;
    }
    return assignedValues[0] == "milk";
}

static bool hintNine(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.empty())
    {
        return true;
    }
    return assignedValues[0] == "norwegian";
}

static bool hintTen_a(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 3)
    {
        return true;
    }
    const std::string& smoke = assignedValues[0];
    const std::string& pet1 = assignedValues[1];
    const std::string& pet2 = assignedValues[2];
    return smoke != "blends" || pet1 == "cats" || pet2 == "cats";
}

static bool hintTen_b(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& smoke = assignedValues[0];
    const std::string& pet = assignedValues[1];
    return smoke != "blends" || pet == "cats";
}

static bool hintEleven_a(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 3)
    {
        return true;
    }
    const std::string& pet = assignedValues[0];
    const std::string& smoke1 = assignedValues[1];
    const std::string& smoke2 = assignedValues[2];
    return pet != "horses" || smoke1 == "dunhill" || smoke2 == "dunhill";
}

static bool hintEleven_b(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& pet = assignedValues[0];
    const std::string& smoke = assignedValues[1];
    return pet != "horses" || smoke == "dunhill";
}

static bool hintTwelve(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& smoke = assignedValues[0];
    const std::string& drink = assignedValues[1];
    return smoke != "bluemaster" || drink == "beer";
}

static bool hintThirteen(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& nationality = assignedValues[0];
    const std::string& smoke = assignedValues[1];
    return nationality != "german" || smoke == "prince";
}

static bool hintFourteen_a(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 3)
    {
        return true;
    }
    const std::string& nationality = assignedValues[0];
    const std::string& color1 = assignedValues[1];
    const std::string& color2 = assignedValues[2];
    return nationality != "norwegian" || color1 == "blue" || color2 == "blue";
}

static bool hintFourteen_b(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& nationality = assignedValues[0];
    const std::string& color = assignedValues[1];
    return nationality != "norwegian" || color == "blue";
}

static bool hintFifteen_a(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 3)
    {
        return true;
    }
    const std::string& smoke = assignedValues[0];
    const std::string& drink1 = assignedValues[1];
    const std::string& drink2 = assignedValues[2];
    return smoke != "blends" || drink1 == "water" || drink2 == "water";
}

static bool hintFifteen_b(const std::vector<std::string>& assignedValues)
{
    if (assignedValues.size() < 2)
    {
        return true;
    }
    const std::string& smoke = assignedValues[0];
    const std::string& drink = assignedValues[1];
    return smoke != "blends" || drink == "water";
}

csp::ConstraintProblem<std::string> constructEinsteinRiddleProblem(std::vector<csp::Variable<std::string>>& variables,
	std::vector<csp::Constraint<std::string>>& constraints, VarRefsMaps& varRefsMaps)
	/*
	 ////////////////////////////////////////// einstein's five house riddle //////////////////////////////////////////
     ///// The situation: /////
     1. There are 5 houses in five different colors.
     2. In each house lives a person with a different nationality.
     3. These five owners drink a certain type of beverage, smoke a certain brand of cigar and keep a certain pet.
     4. No owners have the same pet, smoke the same brand of cigar or drink the same beverage.

     - THE QUESTION IS: WHO OWNS THE FISH?

     /// Hints: ///
     1. the brit lives in the red house
     2. the swede keeps dogs as pets
     3. the dane drinks tea
     4. the green house is on the left of the white house
     5. the green house's owner drinks coffee
     6. the person who smokes pallmall rears birds
     7. the owner of the yellow house smokes dunhill
     8. the man living in the center house drinks milk
     9. the norwegian lives in the first house
     10. the man who smokes blends lives next to the one who keeps cats
     11. the man who keeps horses lives next to the man who smokes dunhill
     12. the owner who smokes bluemaster drinks beer
     13. the german smokes prince
     14. the norwegian lives next to the blue house
     15. the man who smokes blend has a neighbor who drinks water
	*/
{
    variables.reserve(25);
    constraints.reserve(80);

    std::unordered_set<std::string> colorsDomain          { "red", "white", "green", "yellow", "blue" };
    std::unordered_set<std::string> nationalitiesDomain   { "brit", "swede", "dane", "norwegian", "german" };
    std::unordered_set<std::string> drinksDomain          { "tea", "coffee", "milk", "beer", "water" };
    std::unordered_set<std::string> smokesDomain          { "pallmall", "dunhill", "blends", "bluemaster", "prince" };
    std::unordered_set<std::string> petsDomain            { "dogs", "birds", "cats", "horses", "fish" };

    for (unsigned int i = 1; i < 6; ++i)
    {
        variables.push_back(colorsDomain);
        varRefsMaps.colorsVarsMap.emplace(i, variables.back());

        variables.push_back(nationalitiesDomain);
        varRefsMaps.nationalitiesVarsMap.emplace(i, variables.back());

        variables.emplace_back(drinksDomain);
        varRefsMaps.drinksVarsMap.emplace(i, variables.back());

        variables.emplace_back(smokesDomain);
         varRefsMaps.smokesVarsMap.emplace(i, variables.back());

        variables.emplace_back(petsDomain);
        varRefsMaps.petsVarsMap.emplace(i, variables.back());
    }

    std::vector<std::reference_wrapper<csp::Variable<std::string>>> allDiffColorsConstraintVars
    { 
        varRefsMaps.colorsVarsMap.at(1), varRefsMaps.colorsVarsMap.at(2), varRefsMaps.colorsVarsMap.at(3), 
        varRefsMaps.colorsVarsMap.at(4), varRefsMaps.colorsVarsMap.at(5)
    };
    constraints.emplace_back(allDiffColorsConstraintVars, csp::allDiff<std::string>);

    std::vector<std::reference_wrapper<csp::Variable<std::string>>> allDiffNationalitiesConstraintVars
    {
        varRefsMaps.nationalitiesVarsMap.at(1), varRefsMaps.nationalitiesVarsMap.at(2), varRefsMaps.nationalitiesVarsMap.at(3), 
        varRefsMaps.nationalitiesVarsMap.at(4), varRefsMaps.nationalitiesVarsMap.at(5)
    };
    constraints.emplace_back(allDiffNationalitiesConstraintVars, csp::allDiff<std::string>);

    std::vector<std::reference_wrapper<csp::Variable<std::string>>> allDiffDrinksConstraintVars
    {
        varRefsMaps.drinksVarsMap.at(1), varRefsMaps.drinksVarsMap.at(2), varRefsMaps.drinksVarsMap.at(3), 
        varRefsMaps.drinksVarsMap.at(4), varRefsMaps.drinksVarsMap.at(5)
    };
    constraints.emplace_back(allDiffDrinksConstraintVars, csp::allDiff<std::string>);

    std::vector<std::reference_wrapper<csp::Variable<std::string>>> allDiffSmokesConstraintVars
    {
         varRefsMaps.smokesVarsMap.at(1),  varRefsMaps.smokesVarsMap.at(2),  varRefsMaps.smokesVarsMap.at(3),  
         varRefsMaps.smokesVarsMap.at(4),  varRefsMaps.smokesVarsMap.at(5)
    };
    constraints.emplace_back(allDiffSmokesConstraintVars, csp::allDiff<std::string>);

    std::vector<std::reference_wrapper<csp::Variable<std::string>>> allDiffPetsConstraintVars
    {
        varRefsMaps.petsVarsMap.at(1), varRefsMaps.petsVarsMap.at(2), varRefsMaps.petsVarsMap.at(3), 
        varRefsMaps.petsVarsMap.at(4), varRefsMaps.petsVarsMap.at(5)
    };
    constraints.emplace_back(allDiffPetsConstraintVars, csp::allDiff<std::string>);


    for (unsigned int i = 1; i < 6; ++i)
    {
        std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintOneConstraintVars
        {
            varRefsMaps.nationalitiesVarsMap.at(i), varRefsMaps.colorsVarsMap.at(i)
        };
        constraints.emplace_back(hintOneConstraintVars, hintOne);

        std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintTwoConstraintVars
        {
            varRefsMaps.nationalitiesVarsMap.at(i), varRefsMaps.petsVarsMap.at(i) 
        };
        constraints.emplace_back(hintTwoConstraintVars, hintTwo);

        std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintThreeConstraintVars
        {
            varRefsMaps.nationalitiesVarsMap.at(i), varRefsMaps.drinksVarsMap.at(i)
        };
        constraints.emplace_back(hintThreeConstraintVars, hintThree);

        if (i < 5)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintFourAConstraintVars
            {
                varRefsMaps.colorsVarsMap.at(i), varRefsMaps.colorsVarsMap.at(i + 1)
            };
            constraints.emplace_back(hintFourAConstraintVars, hintFour_a);
        }
        else
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintFourBConstraintVars
            {
                varRefsMaps.colorsVarsMap.at(i)
            };
            constraints.emplace_back(hintFourBConstraintVars, hintFour_b);
        }

        std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintFiveConstraintVars
        {
            varRefsMaps.colorsVarsMap.at(i), varRefsMaps.drinksVarsMap.at(i)
        };
        constraints.emplace_back(hintFiveConstraintVars, hintFive);

        std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintSixConstraintVars
        {
             varRefsMaps.smokesVarsMap.at(i), varRefsMaps.petsVarsMap.at(i)
        };
        constraints.emplace_back(hintSixConstraintVars, hintSix);

        std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintSevenConstraintVars
        {
            varRefsMaps.colorsVarsMap.at(i),  varRefsMaps.smokesVarsMap.at(i)
        };
        constraints.emplace_back(hintSevenConstraintVars, hintSeven);

        if (i == 3)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintEightConstraintVars
            {
                varRefsMaps.drinksVarsMap.at(i)
            };
            constraints.emplace_back(hintEightConstraintVars, hintEight);
        }

        if (i == 1)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintNineConstraintVars
            {
                varRefsMaps.nationalitiesVarsMap.at(i)
            };
            constraints.emplace_back(hintNineConstraintVars, hintNine);
        }

        if (1 < i && i < 5)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintTenAConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(i), varRefsMaps.petsVarsMap.at(i - 1), varRefsMaps.petsVarsMap.at(i + 1)
            };
            constraints.emplace_back(hintTenAConstraintVars, hintTen_a);
        }
        else if (i == 1)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> firstHintTenBConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(i), varRefsMaps.petsVarsMap.at(2)
            };
            constraints.emplace_back(firstHintTenBConstraintVars, hintTen_b);

            std::vector<std::reference_wrapper<csp::Variable<std::string>>> secondHintTenBConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(2), varRefsMaps.petsVarsMap.at(i)
            };
            constraints.emplace_back(secondHintTenBConstraintVars, hintTen_b);
        }
        else
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> thirdtHintTenBConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(i), varRefsMaps.petsVarsMap.at(4)
            };
            constraints.emplace_back(thirdtHintTenBConstraintVars, hintTen_b);

            std::vector<std::reference_wrapper<csp::Variable<std::string>>> fourthHintTenBConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(4), varRefsMaps.petsVarsMap.at(i)
            };
            constraints.emplace_back(fourthHintTenBConstraintVars, hintTen_b);
        }

        if (1 < i && i < 5)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> firstHintElevenAConstraintVars
            {
                varRefsMaps.petsVarsMap.at(i),  varRefsMaps.smokesVarsMap.at(i - 1),  varRefsMaps.smokesVarsMap.at(i + 1)
            };
            constraints.emplace_back(firstHintElevenAConstraintVars, hintEleven_a);
        }
        else if (i == 1)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> secondHintElevenAConstraintVars
            {
                varRefsMaps.petsVarsMap.at(i),  varRefsMaps.smokesVarsMap.at(2)
            };
            constraints.emplace_back(secondHintElevenAConstraintVars, hintEleven_a);

            std::vector<std::reference_wrapper<csp::Variable<std::string>>> thirdHintElevenAConstraintVars
            {
                varRefsMaps.petsVarsMap.at(2),  varRefsMaps.smokesVarsMap.at(i)
            };
            constraints.emplace_back(thirdHintElevenAConstraintVars, hintEleven_a);
        }
        else
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> fourthHintElevenAConstraintVars
            {
                varRefsMaps.petsVarsMap.at(i),  varRefsMaps.smokesVarsMap.at(4)
            };
            constraints.emplace_back(fourthHintElevenAConstraintVars, hintEleven_a);

            std::vector<std::reference_wrapper<csp::Variable<std::string>>> fifthHintElevenAConstraintVars
            {
                varRefsMaps.petsVarsMap.at(4),  varRefsMaps.smokesVarsMap.at(i)
            };
            constraints.emplace_back(fifthHintElevenAConstraintVars, hintEleven_a);
        }

        std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintTwelveConstraintVars
        {
             varRefsMaps.smokesVarsMap.at(i), varRefsMaps.drinksVarsMap.at(i)
        };
        constraints.emplace_back(hintTwelveConstraintVars, hintTwelve);

        std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintThirteenConstraintVars
        {
            varRefsMaps.nationalitiesVarsMap.at(i),  varRefsMaps.smokesVarsMap.at(i)
        };
        constraints.emplace_back(hintThirteenConstraintVars, hintThirteen);

        if (1 < i && i < 5)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintFourteenAConstraintVars
            {
                varRefsMaps.nationalitiesVarsMap.at(i), varRefsMaps.colorsVarsMap.at(i - 1), varRefsMaps.colorsVarsMap.at(i + 1)
            };
            constraints.emplace_back(hintFourteenAConstraintVars, hintFourteen_a);
        }
        else if (i == 1)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> firstHintFourteenBConstraintVars
            {
                varRefsMaps.nationalitiesVarsMap.at(i), varRefsMaps.colorsVarsMap.at(2)
            };
            constraints.emplace_back(firstHintFourteenBConstraintVars, hintFourteen_b);

            std::vector<std::reference_wrapper<csp::Variable<std::string>>> secondHintFourteenBConstraintVars
            {
                varRefsMaps.nationalitiesVarsMap.at(2), varRefsMaps.colorsVarsMap.at(i)
            };
            constraints.emplace_back(secondHintFourteenBConstraintVars, hintFourteen_b);
        }
        else
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> thirdHintFourteenBConstraintVars
            {
                varRefsMaps.nationalitiesVarsMap.at(i), varRefsMaps.colorsVarsMap.at(4)
            };
            constraints.emplace_back(thirdHintFourteenBConstraintVars, hintFourteen_b);

            std::vector<std::reference_wrapper<csp::Variable<std::string>>> fourthHintFourteenBConstraintVars
            {
                varRefsMaps.nationalitiesVarsMap.at(4), varRefsMaps.colorsVarsMap.at(i)
            };
            constraints.emplace_back(fourthHintFourteenBConstraintVars, hintFourteen_b);
        }

        if (1 < i && i < 5)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> hintFifteenAConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(i), varRefsMaps.drinksVarsMap.at(i - 1), varRefsMaps.drinksVarsMap.at(i + 1)
            };
            constraints.emplace_back(hintFifteenAConstraintVars, hintFifteen_a);
        }
        else if (i == 1)
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> firstHintFifteenBConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(i), varRefsMaps.drinksVarsMap.at(2)
            };
            constraints.emplace_back(firstHintFifteenBConstraintVars, hintFifteen_b);

            std::vector<std::reference_wrapper<csp::Variable<std::string>>> secondHintFifteenBConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(2), varRefsMaps.drinksVarsMap.at(i)
            };
            constraints.emplace_back(secondHintFifteenBConstraintVars, hintFifteen_b);
        }
        else
        {
            std::vector<std::reference_wrapper<csp::Variable<std::string>>> thirdHintFifteenBConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(i), varRefsMaps.drinksVarsMap.at(4)
            };
            constraints.emplace_back(thirdHintFifteenBConstraintVars, hintFifteen_b);

            std::vector<std::reference_wrapper<csp::Variable<std::string>>> fourthHintFifteenBConstraintVars
            {
                 varRefsMaps.smokesVarsMap.at(4), varRefsMaps.drinksVarsMap.at(i)
            };
            constraints.emplace_back(fourthHintFifteenBConstraintVars, hintFifteen_b);
        }
    }

    std::vector<std::reference_wrapper<csp::Constraint<std::string>>> constraintsRefs{ constraints.begin(), constraints.end() };
    csp::ConstraintProblem<std::string> einsteinRiddleProblem{ constraintsRefs };
    return einsteinRiddleProblem;
}