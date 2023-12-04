#include <iostream>
#include <vector>
#include <cassert>
#include <random>

#include "dealer.h"  // Include the header file for your CarDB class

enum RANDOM { UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE };
class Random {
public:
	Random(int min, int max, RANDOM type = UNIFORMINT, int mean = 50, int stdev = 20) : m_min(min), m_max(max), m_type(type)
	{
		if (type == NORMAL) {
			//the case of NORMAL to generate integer numbers with normal distribution
			m_generator = std::mt19937(m_device());
			//the data set will have the mean of 50 (default) and standard deviation of 20 (default)
			//the mean and standard deviation can change by passing new values to constructor 
			m_normdist = std::normal_distribution<>(mean, stdev);
		}
		else if (type == UNIFORMINT) {
			//the case of UNIFORMINT to generate integer numbers
			// Using a fixed seed value generates always the same sequence
			// of pseudorandom numbers, e.g. reproducing scientific experiments
			// here it helps us with testing since the same sequence repeats
			m_generator = std::mt19937(10);// 10 is the fixed seed value
			m_unidist = std::uniform_int_distribution<>(min, max);
		}
		else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
			m_generator = std::mt19937(10);// 10 is the fixed seed value
			m_uniReal = std::uniform_real_distribution<double>((double)min, (double)max);
		}
		else { //the case of SHUFFLE to generate every number only once
			m_generator = std::mt19937(m_device());
		}
	}
	void setSeed(int seedNum) {
		// we have set a default value for seed in constructor
		// we can change the seed by calling this function after constructor call
		// this gives us more randomness
		m_generator = std::mt19937(seedNum);
	}

	void getShuffle(vector<int>& array) {
		// the user program creates the vector param and passes here
		// here we populate the vector using m_min and m_max
		for (int i = m_min; i <= m_max; i++) {
			array.push_back(i);
		}
		shuffle(array.begin(), array.end(), m_generator);
	}

	void getShuffle(int array[]) {
		// the param array must be of the size (m_max-m_min+1)
		// the user program creates the array and pass it here
		vector<int> temp;
		for (int i = m_min; i <= m_max; i++) {
			temp.push_back(i);
		}
		std::shuffle(temp.begin(), temp.end(), m_generator);
		vector<int>::iterator it;
		int i = 0;
		for (it = temp.begin(); it != temp.end(); it++) {
			array[i] = *it;
			i++;
		}
	}

	int getRandNum() {
		// this function returns integer numbers
		// the object must have been initialized to generate integers
		int result = 0;
		if (m_type == NORMAL) {
			//returns a random number in a set with normal distribution
			//we limit random numbers by the min and max values
			result = m_min - 1;
			while (result < m_min || result > m_max)
				result = m_normdist(m_generator);
		}
		else if (m_type == UNIFORMINT) {
			//this will generate a random number between min and max values
			result = m_unidist(m_generator);
		}
		return result;
	}

	double getRealRandNum() {
		// this function returns real numbers
		// the object must have been initialized to generate real numbers
		double result = m_uniReal(m_generator);
		// a trick to return numbers only with two deciaml points
		// for example if result is 15.0378, function returns 15.03
		// to round up we can use ceil function instead of floor
		result = std::floor(result * 100.0) / 100.0;
		return result;
	}

private:
	int m_min;
	int m_max;
	RANDOM m_type;
	std::random_device m_device;
	std::mt19937 m_generator;
	std::normal_distribution<> m_normdist;//normal distribution
	std::uniform_int_distribution<> m_unidist;//integer uniform distribution
	std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};

enum TestResult { PASS, FAIL };
unsigned int hashCode(const string str) {
	unsigned int val = 0;
	const unsigned int thirtyThree = 33;  // magic number from textbook
	for (unsigned int i = 0; i < str.length(); i++)
		val = val * thirtyThree + str[i];
	return val;
}
string carModels[5] = { "challenger", "stratos", "gt500", "miura", "x101" };
string dealers[5] = { "super car", "mega car", "car world", "car joint", "shack of cars" };

class Tester {
public:
	static TestResult testInsertion() {
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);
		vector <Car> cars_inserted;
		// Insert multiple non-colliding keys
		for (int i = 0; i < 10; ++i) {
			Car car(carModels[rndCar.getRandNum()], rndQuantity.getRandNum(), rndID.getRandNum(), true);
			if (!carDB.insert(car)) {
				cars_inserted.push_back(car);
				std::cerr << "Test Insertion Failed: Could not insert car: " << car << std::endl;
				return FAIL;
			}
		}

		for (Car car : cars_inserted) {
			//Car car(carModels[i], 0, 0, false);  // Placeholder for comparison
			int index = hashCode(car.getModel()) % carDB.m_currentCap;
			if (!(carDB.m_currentTable[index] == car)) {
				std::cerr << "Test Insertion Failed: Incorrect insertion at index " << index << std::endl;
				return FAIL;
			}
		}


		//// Check whether they are inserted in the correct bucket (correct index)
		//for (int i = 0; i < 10; ++i) {
		//    Car car(carModels[i], 0, 0, false);  // Placeholder for comparison
		//    int index = hashCode(car.getModel()) % carDB.m_currentCap;
		//    if (!(carDB.m_currentTable[index] == car)) {
		//        std::cerr << "Test Insertion Failed: Incorrect insertion at index " << index << std::endl;
		//        return FAIL;
		//    }
		//}

		// Check whether the data size changes correctly
		if (carDB.m_currentSize != 10) {
			std::cerr << "Test Insertion Failed: Incorrect data size after insertion" << std::endl;
			return FAIL;
		}

		return PASS;
	}

	static bool test_Removal() {
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);

		// Insert some data
		Car car1("Model1", 2022, 1, true);
		Car car2("Model2", 2022, 2, true);
		Car car3("Model3", 2022, 3, true);

		carDB.insert(car1);
		carDB.insert(car2);
		carDB.insert(car3);

		// Display the current state of the CarDB
		std::cout << "Before Deletion:" << std::endl;
		carDB.dump();

		// Test the remove operation
		bool result1 = carDB.remove(car2);  // Remove car2

		// Display the state of the CarDB after deletion
		std::cout << "\nAfter Deletion of Car2:" << std::endl;
		carDB.dump();

		// Check if the removal was successful
		if (result1) {
			std::cout << "\nDeletion of Car2 was successful." << std::endl;
			//return 1;
		}
		else {
			std::cerr << "\nFailed to delete Car2." << std::endl;
			//return 0;
		}

		// Test the find operation after deletion
		Car foundCar2 = carDB.getCar("Model2", 2);

		if (!foundCar2.getUsed()) {
			std::cout << "\nCar2 is not found after deletion (as expected)." << std::endl;
			return 1;
		}
		else {
			std::cerr << "\nError: Car2 is still found after deletion." << std::endl;
			return 0;
		}
	}

	static TestResult testFindNonExistingCar() {
		// Test the find operation for an error case, the Car object does not exist in the database
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		if (carDB.getCar("nonexistent", 0).getUsed()) {
			std::cerr << "Test Find Non-Existing Car Failed: Found nonexistent car" << std::endl;
			return FAIL;
		}
		return PASS;
	}

	static TestResult testFindNonCollidingKeys() {
		// Test the find operation with several non-colliding keys
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		std::vector<Car> cars;

		for (int i = 0; i < 5; ++i) {
			Car car(carModels[i], rndQuantity.getRandNum(), rndID.getRandNum(), true);
			cars.push_back(car);
			carDB.insert(car);
		}

		for (const auto& car : cars) {
			if (!(car == carDB.getCar(car.getModel(), car.getDealer()))) {
				std::cerr << "Test Find Non-Colliding Keys Failed: Incorrect data found for car: " << car << std::endl;
				return FAIL;
			}
		}

		return PASS;
	}

	static TestResult testFindCollidingKeys() {
		// Test the find operation with several colliding keys without triggering a rehash
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);

		// Insert colliding keys
		Car car1("collide", 0, rndID.getRandNum(), true);
		Car car2("collide", 0, rndID.getRandNum(), true);
		carDB.insert(car1);
		carDB.insert(car2);

		// Check whether both cars are found
		if (!(car1 == carDB.getCar("collide", car1.getDealer()) && car2 == carDB.getCar("collide", car2.getDealer()))) {
			std::cerr << "Test Find Colliding Keys Failed: Incorrect data found for colliding keys" << std::endl;
			return FAIL;
		}

		return PASS;
	}

	static TestResult testRemoveNonCollidingKeys() {
		// Test the remove operation with a few non-colliding keys
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);

		// Insert non-colliding keys
		Car car1("remove1", 0, rndID.getRandNum(), true);
		Car car2("remove2", 0, rndID.getRandNum(), true);
		carDB.insert(car1);
		carDB.insert(car2);

		// Remove one key and check if it's not found, and the other is found
		carDB.remove(car1);
		if (carDB.getCar("remove1", car1.getDealer()) == EMPTY && car2 == carDB.getCar("remove2", car2.getDealer())) {
			return PASS;
		}
		else {
			std::cerr << "Test Remove Non-Colliding Keys Failed: Incorrect data found after removal" << std::endl;
			return FAIL;
		}
	}

	static TestResult testRemoveCollidingKeys() {
		// Test the remove operation with a number of colliding keys without triggering a rehash
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);

		// Insert colliding keys
		Car car1("remove_collide", 0, rndID.getRandNum(), true);
		Car car2("remove_collide", 0, rndID.getRandNum(), true);
		carDB.insert(car1);
		carDB.insert(car2);

		// Remove one key and check if it's not found, and the other is found
		carDB.remove(car1);
		if (carDB.getCar("remove_collide", car1.getDealer()).getUsed() || car2 == carDB.getCar("remove_collide", car2.getDealer())) {
			std::cerr << "Test Remove Colliding Keys Failed: Incorrect data found after removal" << std::endl;
			return FAIL;
		}

		return PASS;
	}

	static TestResult testRehashDataInsertion() {
		// Test rehashing is triggered after a descent number of data insertion
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		// Insert more data than the initial capacity to trigger rehashing
		for (int i = 0; i < 50; ++i) {
			Car car(carModels[rndCar.getRandNum()], rndQuantity.getRandNum(), rndID.getRandNum(), true);
			carDB.insert(car);
		}

		// Check whether the current size is correct after rehashing
		if (carDB.m_currentSize != 50) {
			std::cerr << "Test Rehash Data Insertion Failed: Incorrect data size after rehashing" << std::endl;
			return FAIL;
		}

		return PASS;
	}

	static TestResult testRehashLoadFactor() {
		// Test rehash completion after triggering rehash due to load factor
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		// Insert data to reach the load factor threshold
		for (int i = 0; i < 60; ++i) {
			Car car(carModels[rndCar.getRandNum()], rndQuantity.getRandNum(), rndID.getRandNum(), true);
			carDB.insert(car);
		}

		// Check whether the current size is correct after rehashing
		if (carDB.m_currentSize != 60 && carDB.lambda() > 0.5) {
			std::cerr << "Test Rehash Load Factor Failed: Incorrect data size after rehashing" << std::endl;
			return FAIL;
		}

		return PASS;
	}

	static TestResult testRehashDataRemoval() {
		// Test rehash completion after triggering rehash due to a descent number of data removal
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		vector<Car> cars_inserted;

		// Insert data
		for (int i = 0; i < 10; ++i) {
			Car car(carModels[rndCar.getRandNum()], rndQuantity.getRandNum(), rndID.getRandNum(), true);
			if (!carDB.insert(car))
				cout << "didnt insert" << endl;
			else
				cars_inserted.push_back(car);
		}
		carDB.dump();
		// Remove data to reach the delete ratio threshold
		int delete_counter = 0;	bool test;

		for (int i = 0; i < 5; ++i) {
			if (carDB.remove(cars_inserted[i]) == false) {
				cout << "missing: " << cars_inserted[i].getModel() << " (" << cars_inserted[i].m_dealer << "," << cars_inserted[i].m_quantity << ")" << endl;
				carDB.remove(cars_inserted[i]);
			}
			else
				delete_counter++;
		}
		carDB.dump();

		cout << "delete_counter  " << delete_counter << endl;
		cout << "deletedRatio  " << carDB.deletedRatio() * 100 << endl;


		// Check whether the current size is correct after rehashing
		if (carDB.m_currentSize != 30) {
			std::cerr << "Test Rehash Data Removal Failed: Incorrect data size after rehashing" << std::endl;
			return FAIL;
		}

		return PASS;
	}

	static TestResult runAllTests() {
		test_Removal();
		if (
			testInsertion() == FAIL ||
			testFindNonExistingCar() == FAIL ||
			testFindNonCollidingKeys() == FAIL ||
			testFindCollidingKeys() == FAIL ||
			testRemoveNonCollidingKeys() == FAIL ||
			testRemoveCollidingKeys() == FAIL ||
			testRehashDataInsertion() == FAIL ||
			testRehashLoadFactor() == FAIL ||
			testRehashDataRemoval() == FAIL
			) {
			std::cerr << "Some tests failed." << std::endl;
			return FAIL;
		}

		std::cout << "All tests passed successfully!" << std::endl;
		return PASS;
	}
};

int main() {
	Tester::runAllTests();

	return 0;
}
