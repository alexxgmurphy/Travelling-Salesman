#include <iostream>
#include <random>
#include "TSP.h"
#include <fstream>
#include <algorithm>
#include <cstddef>
#include <string>
#include <iterator>
#include <numeric>

std::vector<Location> parseInput(std::ifstream& inFile)
{
	std::string line = "";
	std::vector<Location> retVal;
	while (!inFile.eof())
	{
		std::getline(inFile, line);
		std::vector<std::string> tempVec;
		std::string tempStr = line;
		size_t index = tempStr.find_first_of(',');
		while (index != std::string::npos)
		{
			tempVec.push_back(tempStr.substr(0, index));
			tempStr = tempStr.substr(index + 1);
			index = tempStr.find_first_of(',');
		}
		tempVec.push_back(tempStr);
		Location tempLoc;
		tempLoc.mName = tempVec[0];
		tempLoc.mLatitude = std::stod(tempVec[1]);
		tempLoc.mLongitude = std::stod(tempVec[2]);
		retVal.push_back(tempLoc);
		tempVec.clear();
		tempStr = "";
		line = "";
	}
	return retVal;
}

std::vector<int> genShuffle(std::mt19937 &rand, const int &numLocs)
{
	std::vector<int> toFill(numLocs);
	int i = 0;
	std::generate(toFill.begin(), toFill.end(), [&i, &toFill]() {
		return i++;
	});
	std::shuffle(toFill.begin()+1, toFill.end(), rand);
	return toFill;
}

std::vector<std::vector<int>> initPop(std::mt19937 &rand, const int popSize, const int numLocs)
{
	std::vector<std::vector<int>> retVal(popSize);
	std::generate(retVal.begin(), retVal.end(), [&rand, &numLocs](){
		return genShuffle(rand, numLocs);
	});
	return retVal;
}

void outputPop(const std::vector<std::vector<int>>& pop, std::ofstream &outFile)
{
	for (auto &i : pop)
	{
		for (auto &v : i)
		{
			outFile << v;
			if (v != i[i.size() - 1])
			{
				outFile << ",";
			}
		}
		outFile << "\n";
	}
}

double haversineDistance(const std::pair<double, double> &start, const std::pair<double, double> &end)
{
	double dLon = (end.second * 0.0174533) - (start.second * 0.0174533);
	double dLat = (end.first * 0.0174533) - (start.first * 0.0174533);
	double a = std::pow((sin(dLat / 2)), 2) + cos(start.first * 0.0174533) * cos(end.first * 0.0174533) * std::pow(sin(dLon / 2), 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	double dist = 3961 * c;
	return dist;
}

std::vector<std::pair<double, double>> longLatGen(const std::vector<int> &indexes, const std::vector<Location> &locations)
{
	std::vector<std::pair<double, double>> longLats(indexes.size());
	int i = -1;
	std::generate(longLats.begin(), longLats.end(), [&i, &indexes, &locations]() {
		i++;
		return std::make_pair(locations[indexes[i]].mLatitude, locations[indexes[i]].mLongitude);
	});
	return longLats;
}
std::pair<int, double> individualFitness(const std::vector<int>& individual, const int& index, const std::vector<Location> &locations)
{
	std::vector<int> copyPlus = individual;
	copyPlus.push_back(0);
	std::vector<std::pair<double, double>> longLats = longLatGen(individual, locations);
	std::vector<double> distances(longLats.size());
	int i = -1;
	std::generate(distances.begin(), distances.end()-1, [&longLats, &i]() {
		i++;
		return haversineDistance(longLats[i], longLats[i+1]);
	});
	double sum = haversineDistance(longLats[longLats.size() - 1], longLats[0]);
	sum = std::accumulate(distances.begin(), distances.end(), sum, [&sum](const double& a, const double& b) {
		return a + b;
	});

	return std::make_pair(int(index), sum);
}

std::vector<std::pair<int, double>> popFitness(const std::vector<std::vector<int>>& population, const std::vector<Location> &locations)
{
	std::vector<std::pair<int, double>> fitness(population.size());
	int i = -1;
	std::generate(fitness.begin(), fitness.end(), [&i, &locations, &population]() {
		i++;
		return individualFitness(population[i], i, locations);
	});
	return fitness;
}

void outputFitness(const std::vector<std::pair<int, double>> &population, std::ofstream &out)
{
	out << "FITNESS:\n";
	for (auto &i : population)
	{
		out << i.first << ":" << i.second << std::endl;
	}
}

bool fitBinary(const std::pair<int, double> &a, const std::pair<int, double> &b)
{
	return a.second < b.second;
}

std::vector<std::pair<int, double>> sortFitness(const std::vector<std::pair<int, double>> &fitness)
{
	auto sorted = fitness;
	std::sort(sorted.begin(), sorted.end(), fitBinary);
	return sorted;
}

std::vector<std::pair<int, double>> constructProbs(const std::vector<std::vector<int>> &pop, const std::vector<std::pair<int, double>> &sortedFitness)
{
	int i = -1;
	double popSize = pop.size();
	std::vector<std::pair<int, double>> probs(sortedFitness.size());
	std::generate(probs.begin(), probs.end(), [&i, &popSize, &sortedFitness]() {
		i++;
		double prob = 1 / popSize;
		if (i == 0 || i == 1)
		{
			prob *= 6;
		}
		else if (i < popSize / 2)
		{
			prob *= 3;
		}
		return std::make_pair(sortedFitness[i].first, prob);
	});
	double adjustment = 0;
	adjustment = std::accumulate(probs.begin(), probs.end(), adjustment, [](const double &a, const std::pair<int, double> &b) {
		return a + b.second;
	});
	adjustment = 1 / adjustment;
	std::vector<std::pair<int, double>> adjustedProbs(probs.size());
	std::transform(probs.begin(), probs.end(), adjustedProbs.begin(), [&adjustment](const std::pair<int, double> &prob) {
		return std::make_pair(prob.first, prob.second * adjustment);
	});
	return adjustedProbs;
}

std::pair<int, int> chooseParents(const std::vector<std::pair<int, double>> &fitness, const std::vector<std::vector<int>> &pop, std::mt19937 &rand)
{
	auto sortedFitness = sortFitness(fitness);
	auto probabilities = constructProbs(pop, sortedFitness);
	std::sort(probabilities.begin(), probabilities.end(), [](const std::pair<int, double> &a, const std::pair<int, double> &b) {
		return a.first < b.first;
	});
	std::uniform_real_distribution<double> myDist(0, 1);
	double picker = myDist(rand);
	double accumulated = 0;
	bool picked = false;
	std::pair<int, int> parents = std::make_pair(-1, -1);
	accumulated = std::accumulate(probabilities.begin(), probabilities.end(), accumulated, [&parents, &picker, &picked](double &accumulated, const std::pair<int, double> &poss) {
		accumulated += poss.second;
		if (!picked && accumulated >= picker)
		{
			picked = true;
			parents.first = poss.first;
		}
		return accumulated;
	});

	picker = myDist(rand);
	picked = false;
	accumulated = 0;
	accumulated = std::accumulate(probabilities.begin(), probabilities.end(), accumulated, [&parents, &picker, &picked](double &accumulated, const std::pair<int, double> &poss) {
		accumulated += poss.second;
		if (!picked && accumulated >= picker)
		{
			picked = true;
			parents.second = poss.first;
		}
		return accumulated;
	});
	return parents;
}

std::vector<std::pair<int, int>> makeParentVec(const std::vector<std::pair<int, double>> &fitness, const std::vector<std::vector<int>> &pop, std::mt19937 &rand)
{
	std::vector<std::pair<int, int>> retVal(pop.size());
	std::generate(retVal.begin(), retVal.end(), [&fitness, &pop, &rand](){
		return chooseParents(fitness, pop, rand);
	});
	return retVal;
}

std::vector<int> crossover(const std::vector<int> &parent1, const std::vector<int> &parent2, const int &crossIndex, static int &parent)
{
	std::vector<int> child;
	if (parent == 1)
	{
		std::copy_n(parent1.begin(), crossIndex+1, std::back_inserter(child));
		std::copy_if(parent2.begin(), parent2.end(), std::back_inserter(child), [&child, &parent2](const int currElement) {
			bool copy;
			if (std::find(child.begin(), child.end(), currElement) == child.end())
			{
				copy = true;
			}
			else
			{
				copy = false;
			}
			return copy;
		});
	}
	else
	{
		std::copy_n(parent2.begin(), crossIndex+1, std::back_inserter(child));
		std::copy_if(parent1.begin(), parent1.end(), std::back_inserter(child), [&child, &parent1](const int currElement) {
			bool copy;
			if (std::find(child.begin(), child.end(), currElement) == child.end())
			{
				copy = true;
			}
			else
			{
				copy = false;
			}
			return copy;
		});
	}
	return child;
}

std::vector<int> mutate(const std::vector<int> &original, std::mt19937 &rand)
{
	std::uniform_int_distribution<int> pickIndex1(1, original.size() - 1);
	int firstIndex = pickIndex1(rand);
	std::uniform_int_distribution<int> pickIndex2(1, original.size() - 1);
	int secondIndex = pickIndex2(rand);
	std::vector<int> retVal(original.size());
	std::copy(original.begin(), original.end(), retVal.begin());
	std::swap(retVal[firstIndex], retVal[secondIndex]);
	return retVal;
}

std::vector<std::vector<int>> makeGen(const std::vector<std::pair<int, int>> &parents, const std::vector<std::vector<int>> &routes, std::mt19937 &rand, const double &mutationArg)
{
	std::vector<std::vector<int>> nextGen(parents.size());
	int i = 0;
	std::uniform_int_distribution<int> toCross(1, routes[parents[0].first].size() - 2);
	std::uniform_int_distribution<int> pickParent(0, 1);
	std::uniform_real_distribution<double> mutationRange(0, 1);
	std::generate(nextGen.begin(), nextGen.end(), [&routes, &parents, &i, &rand, &toCross, &pickParent, &mutationArg, &mutationRange]() {
		int crossIndex = toCross(rand);
		int parent = pickParent(rand);
		std::vector<int> child = crossover(routes[parents[i].first], routes[parents[i].second], crossIndex, parent);
		i++;
		double mutationChance = mutationRange(rand);
		if (mutationChance <= mutationArg)
		{
			child = mutate(child, rand);
		}
		return child;
	});

	return nextGen;
}

int main(int argc, const char* argv[])
{
	//parse command line input
	std::string inFileName = argv[1];
	int popSize = std::stoi(argv[2]);
	int generations = std::stoi(argv[3]);
	double mutationChance = std::stod(argv[4]);
	mutationChance = mutationChance / 100;
	int seed = std::stoi(argv[5]);

	//random number constructor
	static std::mt19937 randGen(seed);

	//input file stream
	std::ifstream inFile;
	inFile.open(inFileName);
	if (!inFile.is_open())
	{
		std::cout << "Error opening input file; quitting" << std::endl;
		return 0;
	}

	//generate initial population
	std::vector<Location> locations = parseInput(inFile);

	//generate initial population
	std::vector<std::vector<int>> population = initPop(randGen, popSize, locations.size());

	//output initial population
	std::ofstream outFile;
	outFile.open("log.txt");
	if (!outFile.is_open())
	{
		std::cout << "Error opening output file; quitting" << std::endl;
	}
	outFile << "INITIAL POPULATION:" << std::endl;
	outputPop(population, outFile);

	//calculate initial population fitness
	std::vector<std::pair<int, double>> fitness = popFitness(population, locations);

	//output initial population fitness
	outputFitness(fitness, outFile);
	outFile << "SELECTED PAIRS:" << std::endl;

	fitness = sortFitness(fitness);

	//generate vector of parent pairs
	std::vector<std::pair<int, int>> parentVec = makeParentVec(fitness, population, randGen);

	//output chosen parents
	for (auto &i : parentVec)
	{
		outFile << "(" << i.first << "," << i.second << ")" << std::endl;
	}

	//create specified number of generations and output them
	int count = 0;
	while (count < generations)
	{
		count++;
		outFile << "GENERATION: " << count << std::endl;
		std::vector<std::vector<int>> childPop = makeGen(parentVec, population, randGen, mutationChance);
		outputPop(childPop, outFile);

		auto fitVec = popFitness(childPop, locations);
		outputFitness(fitVec, outFile);
		fitVec = sortFitness(fitVec);
		if (count != generations)
		{
			outFile << "SELECTED PAIRS:" << std::endl;

			parentVec = makeParentVec(fitVec, childPop, randGen);
			for (auto &i : parentVec)
			{
				outFile << "(" << i.first << "," << i.second << ")" << std::endl;
			}
		}
		else
		{
			outFile << "SOLUTION:" << std::endl;
			for (auto& i : childPop[0])
			{
				outFile << locations[i].mName << std::endl;
			}
			outFile << "LAX Airport" << std::endl;
			outFile << "DISTANCE: " << fitVec[0].second << " miles";
		}
		population = childPop;
	}

	return 0;
}
