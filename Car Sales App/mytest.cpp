#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <algorithm>

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
			//the mean and standard deviation can change by 1ing new values to constructor 
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
		// the user program creates the vector param and 1es here
		// here we populate the vector using m_min and m_max
		for (int i = m_min; i <= m_max; i++) {
			array.push_back(i);
		}
		shuffle(array.begin(), array.end(), m_generator);
	}

	void getShuffle(int array[]) {
		// the param array must be of the size (m_max-m_min+1)
		// the user program creates the array and 1 it here
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
	bool testInsertion() {
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
				return 0;
			}
		}

		for (Car car : cars_inserted) {
			int index = hashCode(car.getModel()) % carDB.m_currentCap;
			if (!(carDB.m_currentTable[index] == car)) {
				return 0;
			}
		}

		// Check whether the data size changes correctly
		if (carDB.m_currentSize != 10) {
			return 0;
		}

		return 1;
	}
	bool testInsertionEmpty() {
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		for (int i = 0; i < 15; ++i) {
			Car car;
			if (carDB.insert(car)) {
				return 0; //shows error insertion
			}
		}
		if (carDB.m_currentSize == 0)
			return 1;
	}
	bool testInsertionDuplicate() {
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		Car car(carModels[rndCar.getRandNum()], rndQuantity.getRandNum(), rndID.getRandNum(), true);
		carDB.insert(car);

		for (int i = 0; i < 10; ++i) {
			if (carDB.insert(car)) {
				return 0; //shows error insertion
			}
		}
		if (carDB.m_currentSize == 1)
			return 1;
	}
	bool testInsertion_DOUBLEHASH_Colliding() {
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);

		// Insert four cars with intentionally colliding hash values
		Car car1("challenger", 1, 1001, true);  // hash("model1") % m_currentCap
		Car car2("challenger", 2, 1002, true);  // hash("model2") % m_currentCap collides with car1
		Car car3("gt500", 3, 1003, true);		// hash("model3") % m_currentCap
		Car car4("gt500", 4, 1004, true);		// hash("model4") % m_currentCap collides with car3

		// Insert the cars into the CarDB		
		carDB.insert(car1); //72
		carDB.insert(car2); //72 (already occupied), 81
		carDB.insert(car3); //97
		carDB.insert(car4); //97 (already occupied), 1

		// Check if the cars are inserted at the correct positions after collisions
		int index1 = hashCode(car1.getModel()) % carDB.getCurrentCap();
		int index2 = hashCode(car2.getModel()) % carDB.getCurrentCap();
		int index3 = hashCode(car3.getModel()) % carDB.getCurrentCap();
		int index4 = hashCode(car4.getModel()) % carDB.getCurrentCap();

		if (!carDB.m_currentTable[index1].getUsed() || !carDB.m_currentTable[81].getUsed() ||
			!carDB.m_currentTable[index3].getUsed() || !carDB.m_currentTable[1].getUsed()) {
			return false;
		}

		// Check if the cars are in the expected positions
		if (!(carDB.m_currentTable[index1] == car1) || !(carDB.m_currentTable[81] == car2) ||
			!(carDB.m_currentTable[index3] == car3) || !(carDB.m_currentTable[1] == car4)) {
			return false;
		}

		return true;
	}
	bool testInsertion_QUADRATIC_Colliding() {
		CarDB carDB(MINPRIME, hashCode, QUADRATIC);

		// Insert four cars with intentionally colliding hash values
		Car car1("challenger", 1, 1001, true);  // hash("model1") % m_currentCap
		Car car2("challenger", 2, 1002, true);  // hash("model2") % m_currentCap collides with car1
		Car car3("gt500", 3, 1003, true);		// hash("model3") % m_currentCap
		Car car4("gt500", 4, 1004, true);		// hash("model4") % m_currentCap collides with car3

		// Insert the cars into the CarDB		
		carDB.insert(car1); //72
		carDB.insert(car2); //72 (already occupied), 73
		carDB.insert(car3); //97
		carDB.insert(car4); //97 (already occupied), 98

		// Check if the cars are inserted at the correct positions after collisions
		int index1 = hashCode(car1.getModel()) % carDB.getCurrentCap();
		int index2 = hashCode(car2.getModel()) % carDB.getCurrentCap();
		int index3 = hashCode(car3.getModel()) % carDB.getCurrentCap();
		int index4 = hashCode(car4.getModel()) % carDB.getCurrentCap();

		if (!carDB.m_currentTable[index1].getUsed() || !carDB.m_currentTable[73].getUsed() ||
			!carDB.m_currentTable[index3].getUsed() || !carDB.m_currentTable[98].getUsed()) {
			return false;
		}

		// Check if the cars are in the expected positions
		if (!(carDB.m_currentTable[index1] == car1) || !(carDB.m_currentTable[73] == car2) ||
			!(carDB.m_currentTable[index3] == car3) || !(carDB.m_currentTable[98] == car4)) {
			return false;
		}

		return true;
	}

	bool testInsertion_NonColliding() {
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);

		// Insert four cars with intentionally colliding hash values
		Car car1("model1", 1, 1001, true);  // hash("model1") % m_currentCap
		Car car2("model2", 2, 1002, true);  // hash("model2") % m_currentCap collides with car1
		Car car3("model3", 3, 1003, true);  // hash("model3") % m_currentCap collides with car1 and car2
		Car car4("model4", 4, 1004, true);  // hash("model4") % m_currentCap collides with car1, car2, and car3

		// Insert the cars into the CarDB		
		carDB.insert(car1); //63
		carDB.insert(car2); //64
		carDB.insert(car3); //65
		carDB.insert(car4); //66

		// Check if the cars are inserted at the correct positions after collisions
		int index1 = hashCode(car1.getModel()) % carDB.getCurrentCap();
		int index2 = hashCode(car2.getModel()) % carDB.getCurrentCap();
		int index3 = hashCode(car3.getModel()) % carDB.getCurrentCap();
		int index4 = hashCode(car4.getModel()) % carDB.getCurrentCap();

		if (!carDB.m_currentTable[index1].getUsed() || !carDB.m_currentTable[index2].getUsed() ||
			!carDB.m_currentTable[index3].getUsed() || !carDB.m_currentTable[index4].getUsed()) {
			return false;
		}

		// Check if the cars are in the expected positions
		if (!(carDB.m_currentTable[index1] == car1) || !(carDB.m_currentTable[index2] == car2) ||
			!(carDB.m_currentTable[index3] == car3) || !(carDB.m_currentTable[index4] == car4)) {
			return false;
		}

		return true;
	}

	bool test_Removal_Normal() {
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		vector<Car> cars_inserted;		int number_of_cars_inserted = 0;

		// Insert data
		for (int i = 0; i < 50; ++i) {
			Car car(carModels[rndCar.getRandNum()], rndQuantity.getRandNum(), rndID.getRandNum(), true);
			if (carDB.insert(car)) {
				cars_inserted.push_back(car);
				number_of_cars_inserted++;
			}
		}
		// Remove data to reach the delete ratio threshold
		for (Car car : cars_inserted)
			carDB.remove(car);  //remove all cars


		for (int i = 0; i < number_of_cars_inserted; i++)
		{
			if (!(carDB.getCar(cars_inserted[i].getModel(), cars_inserted[i].getDealer()) == EMPTY))
				return 0;	//if any previous car found in the table, then fail
		}
		return 1;
	}
	bool test_Removal_Empty() {
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		for (int i = 0; i < 50; ++i) {
			Car car;
			if (carDB.remove(car))  //attempting to remove empty car
				return 0;
		}
		if (carDB.m_currNumDeleted > 0)
			return 0;
		else
			return 1;

	}
	bool test_Removal_NonExistant() {
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		vector<Car> cars_inserted;		int number_of_cars_inserted = 0;

		Car car(carModels[rndCar.getRandNum()], rndQuantity.getRandNum(), rndID.getRandNum(), true);
		if (carDB.remove(car)) {
			return 0;
		}
		return 1;	//since not exisiting hence cant be removed
	}
	bool testRemove_NonCollidingKeys() {
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
			return 1;
		}
		else {
			return 0;
		}
	}

	bool testRemove_DOUBLEHASH_CollidingKeys() {
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
			return 0;
		}

		return 1;
	}
	bool testRemove_QUADRATIC_CollidingKeys() {
		// Test the remove operation with a number of colliding keys without triggering a rehash
		CarDB carDB(MINPRIME, hashCode, QUADRATIC);
		Random rndID(MINID, MAXID);

		// Insert colliding keys
		Car car1("remove_collide", 0, rndID.getRandNum(), true);
		Car car2("remove_collide", 0, rndID.getRandNum(), true);
		carDB.insert(car1);
		carDB.insert(car2);

		// Remove one key and check if it's not found, and the other is found
		carDB.remove(car1);
		if (carDB.getCar("remove_collide", car1.getDealer()).getUsed() || car2 == carDB.getCar("remove_collide", car2.getDealer())) {
			return 0;
		}

		return 1;
	}

	bool testFind_Empty_Car() {
		// Test the find operation for an error case, the Car object does not exist in the database
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Car car;
		if ((carDB.getCar(car.getModel(), car.getDealer())) == EMPTY) {
			return 1;
		}
		else
			return 0;
	}
	bool testFind_NonExistingCar() {
		// Test the find operation for an error case, the Car object does not exist in the database
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		if (carDB.getCar("nonexistent", 0).getUsed()) {
			return 0;
		}
		return 1;
	}
	bool testFind_NonCollidingKeys() {
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
				return 0;
			}
		}

		return 1;
	}
	bool testFind_CollidingKeys() {
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
			return 0;
		}

		return 1;
	}

	bool testRehashDataInsertion_and_LoadFactor() {
		// Test rehashing is triggered after a descent number of data insertion
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		int Initial_Table_Cap = carDB.m_currentCap;
		int Final_Table_Cap; //?

		// Insert more data than the initial capacity to trigger rehashing
		for (int i = 0; i < 60; ++i) {
			Car car(carModels[rndCar.getRandNum()], rndQuantity.getRandNum(), rndID.getRandNum(), true);
			carDB.insert(car);
		}
		Final_Table_Cap = carDB.m_currentCap;
		// Check whether the current size is correct after rehashing
		if (carDB.m_currentSize != 60) {
			return 0;
		}
		if (Final_Table_Cap > Initial_Table_Cap) //shows the table is reharshed of new cap
			return 1;
	}

	bool testRehashDataRemoval_and_DeletionRatio() {
		// Test rehash completion after triggering rehash due to a descent number of data removal
		CarDB carDB(MINPRIME, hashCode, DOUBLEHASH);
		Random rndID(MINID, MAXID);
		Random rndCar(0, 4);
		Random rndQuantity(0, 50);

		vector<Car> cars_inserted;

		// Insert data
		for (int i = 0; i < 50; ++i) {
			Car car(carModels[rndCar.getRandNum()], rndQuantity.getRandNum(), rndID.getRandNum(), true);
			if (carDB.insert(car))
				cars_inserted.push_back(car);
		}
		// Remove data to reach the delete ratio threshold

		for (int i = 0; i < 40; ++i) {
			carDB.remove(cars_inserted[i]);  //this tiggers 80% deletion and reharsh
		}

		// Check whether the current size is correct after rehashing
		if (carDB.deletedRatio() > 0.8) {
			return 0;
		}

		return 1;
	}

	void runAllTests() {
		cout << "Test Insertion Normal : " << (testInsertion() ? "Passed" : "Failed") << endl;
		cout << "Test Insertion Empty Car : " << (testInsertionEmpty() ? "Passed" : "Failed") << endl;
		cout << "Test Insertion Duplicate Car : " << (testInsertionDuplicate() ? "Passed" : "Failed") << endl;
		cout << "Test Insertion DOUBLEHASH Colliding keys : " << (testInsertion_DOUBLEHASH_Colliding() ? "Passed" : "Failed") << endl;
		cout << "Test Insertion QUADRATIC Colliding keys : " << (testInsertion_QUADRATIC_Colliding() ? "Passed" : "Failed") << endl;
		cout << "Test Insertion NonColliding keys : " << (testInsertion_NonColliding() ? "Passed" : "Failed") << endl;
		cout << "\nTest Removal Normal: " << (test_Removal_Normal() ? "Passed" : "Failed") << endl;
		cout << "Test Removal Empty Car : " << (test_Removal_Empty() ? "Passed" : "Failed") << endl;
		cout << "Test Removal NonExistant Car : " << (test_Removal_NonExistant() ? "Passed" : "Failed") << endl;
		cout << "Test Remove NonColliding Keys : " << (testRemove_NonCollidingKeys() ? "Passed" : "Failed") << endl;
		cout << "Test Remove DOUBLEHASH Colliding Keys : " << (testRemove_DOUBLEHASH_CollidingKeys() ? "Passed" : "Failed") << endl;
		cout << "Test Remove QUADRATIC Colliding Keys : " << (testRemove_QUADRATIC_CollidingKeys() ? "Passed" : "Failed") << endl;
		cout << "\nTest Find Empty Car : " << (testFind_Empty_Car() ? "Passed" : "Failed") << endl;
		cout << "Test Find NonExisting Car : " << (testFind_NonExistingCar() ? "Passed" : "Failed") << endl;
		cout << "Test Find NonColliding Keys : " << (testFind_NonCollidingKeys() ? "Passed" : "Failed") << endl;
		cout << "Test Find Colliding Keys : " << (testFind_CollidingKeys() ? "Passed" : "Failed") << endl;
		cout << "\nTest Rehash Data Insertion : " << (testRehashDataInsertion_and_LoadFactor() ? "Passed" : "Failed") << endl;
		cout << "Test Rehash Data Removal : " << (testRehashDataRemoval_and_DeletionRatio() ? "Passed" : "Failed") << endl;

		std::cout << "\nAll tests ran successfully!" << std::endl;
	}
};

int main() {
	Tester mytestcases;
	mytestcases.runAllTests();

	return 0;
}