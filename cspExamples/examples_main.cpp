
#include "pch.h"
#include "australia_map_coloring_problem.h"
#include "n_queens_problem.h"
#include "magic_square_problem.h"
#include "car_assembly_problem.h"
#include "verbal_arithmetic_problem.h"
#include "einstein_five_house_riddle_problem.h"


int main()
{
	//std::vector<csp::Variable<std::string>> auMapColoringVars;
	//std::vector<csp::Constraint<std::string>> auMapColoringConstrs;
	//csp::ConstraintProblem<std::string> australiaMapColoringProb = constructAustraliaMapColoringProblem(auMapColoringVars, auMapColoringConstrs);
	//std::cout << australiaMapColoringProb;
	//std::cout << "\n----------------------------------------------------------------------------------------------------\n";
	//const csp::AssignmentHistory<std::string> australiaMapColoringProbAssignmentHistory = 
	//	csp::heuristicBacktrackingSolver<std::string>(australiaMapColoringProb,
	//	csp::minimumRemainingValues_primarySelector<std::string>,
	//	csp::degreeHeuristic_secondarySelector<std::string>,
	//	csp::leastConstrainingValue<std::string>);
	//australiaMapColoringProb.writeNameToAssignment(std::cout);

	//std::vector<csp::Variable<unsigned int>> nQueensVars;
	//std::vector<csp::Constraint<unsigned int>> nQueensConstrs;
	//csp::ConstraintProblem<unsigned int> nQueensProb = constructNQueensProblem(8, nQueensVars, nQueensConstrs);
	//std::cout << nQueensProb;
	//std::cout << "\n----------------------------------------------------------------------------------------------------\n";
	//const csp::AssignmentHistory<unsigned int> nQueensProbAssignmentHistory = 
	//	csp::heuristicBacktrackingSolver<unsigned int>(nQueensProb,
	//	csp::minimumRemainingValues_primarySelector<unsigned int>,
	//	csp::degreeHeuristic_secondarySelector<unsigned int>,
	//	csp::leastConstrainingValue<unsigned int>);
	//nQueensProb.writeNameToAssignment(std::cout);

	/*std::vector<csp::Variable<unsigned int>> magicSquareVars;
	std::vector<csp::Constraint<unsigned int>> magicSquareConstrs;
	csp::ConstraintProblem<unsigned int> magicSquareProb = constructMagicSquareProblem(3, magicSquareVars, magicSquareConstrs);
	std::cout << magicSquareProb;
	std::cout << "\n----------------------------------------------------------------------------------------------------\n";
	const csp::AssignmentHistory<unsigned int> magicSquareProbAssignmentHistory =
		csp::heuristicBacktrackingSolver<unsigned int>(magicSquareProb,
		csp::minimumRemainingValues_primarySelector<unsigned int>,
		csp::degreeHeuristic_secondarySelector<unsigned int>,
		csp::leastConstrainingValue<unsigned int>);
	magicSquareProb.writeNameToAssignment(std::cout);*/


	//std::vector<csp::Variable<unsigned int>> carAssemblyVars;
	//std::vector<csp::Constraint<unsigned int>> carAssemblyConstrs;
	//csp::ConstraintProblem<unsigned int> carAssemblyProb = constructCarAssemblyProblem(carAssemblyVars, carAssemblyConstrs);
	//std::cout << carAssemblyProb;
	//std::cout << "\n----------------------------------------------------------------------------------------------------\n";
	//const csp::AssignmentHistory<unsigned int> carAssemblyProbAssignmentHistory =
	//	csp::heuristicBacktrackingSolver<unsigned int>(carAssemblyProb,
	//	csp::minimumRemainingValues_primarySelector<unsigned int>,
	//	csp::degreeHeuristic_secondarySelector<unsigned int>,
	//	csp::leastConstrainingValue<unsigned int>);
	//carAssemblyProb.writeNameToAssignment(std::cout);


	//std::vector<csp::Variable<unsigned int>> verbalArithmeticVars;
	//std::vector<csp::Constraint<unsigned int>> verbalArithmeticConstraints;
	//csp::ConstraintProblem<unsigned int> verbalArithmeticProb = constructVerbalArithmeticProblem(verbalArithmeticVars, verbalArithmeticConstraints);
	//std::cout << verbalArithmeticProb;
	//std::cout << "\n----------------------------------------------------------------------------------------------------\n";
	//const csp::AssignmentHistory<unsigned int> verbalArithmeticProbAssignmentHistory =
	//	csp::heuristicBacktrackingSolver<unsigned int>(verbalArithmeticProb,
	//		csp::minimumRemainingValues_primarySelector<unsigned int>,
	//		csp::degreeHeuristic_secondarySelector<unsigned int>,
	//		csp::leastConstrainingValue<unsigned int>);
	//verbalArithmeticProb.writeNameToAssignment(std::cout);


	std::vector<csp::Variable<std::string>> einsteinRiddleVars;
	std::vector<csp::Constraint<std::string>> einsteinRiddleConstraints;
	VarRefsMaps varsRefMaps;
	csp::ConstraintProblem<std::string> einsteinRiddleProb = constructEinsteinRiddleProblem(einsteinRiddleVars, 
		einsteinRiddleConstraints, varsRefMaps);
	std::cout << einsteinRiddleProb;
	std::cout << "\n----------------------------------------------------------------------------------------------------\n";
	const csp::AssignmentHistory<std::string> einsteinRiddleProbAssignmentHistory =
		csp::heuristicBacktrackingSolver<std::string>(einsteinRiddleProb,
			csp::minimumRemainingValues_primarySelector<std::string>,
			csp::degreeHeuristic_secondarySelector<std::string>,
			csp::leastConstrainingValue<std::string>);
	for (unsigned int i = 1; i < 6; ++i)
	{
		std::cout << i << " : " << varsRefMaps.colorsVarsMap.at(i).get().getValue() << ' ';
		std::cout << varsRefMaps.nationalitiesVarsMap.at(i).get().getValue() << ' ';
		std::cout << varsRefMaps.drinksVarsMap.at(i).get().getValue() << ' ';
		std::cout << varsRefMaps.smokesVarsMap.at(i).get().getValue() << ' ';
		std::cout << varsRefMaps.petsVarsMap.at(i).get().getValue() << '\n';
	}
}