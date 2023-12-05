// CMSC 341 - Fall 2023 - Project 4
#include "dealer.h"
int CarDB::getCurrentCap() const { return m_currentCap; }
CarDB::CarDB(int size, hash_fn hash, prob_t probing) {
	m_hash = hash;
	m_newPolicy = NONE;

	// Set the current table size within the range [MINPRIME-MAXPRIME]
	m_currentCap = findNextPrime(size);
	m_currentTable = new Car[m_currentCap]();
	m_currentSize = 0;
	m_currNumDeleted = 0;
	m_currProbing = probing;

	// Initialize old table variables
	m_oldTable = nullptr;
	m_oldCap = 0;
	m_oldSize = 0;
	m_oldNumDeleted = 0;
	m_oldProbing = NONE;
}

CarDB::~CarDB() {
	delete[] m_currentTable;
	delete[] m_oldTable;
}

void CarDB::changeProbPolicy(prob_t policy) {
	m_newPolicy = policy;
}

bool CarDB::insert(Car car) {
	// Hash the car model to get the index
	if (car == EMPTY)
		return 0;
	int index = m_hash(car.getModel()) % m_currentCap;
	int i = 0;
	// Handle collisions using the current probing policy
	while (m_currentTable[index].getUsed()) {
		if (m_currentTable[index] == car) 
			return false; // Car already exists, cannot insert duplicates
		

		// Use quadratic or double-hash probing based on the policy
		if (m_currProbing == QUADRATIC) 
			index = (index + (i * i)) % m_currentCap;
		else if (m_currProbing == DOUBLEHASH) 
			index = (index + i * (11 - (m_hash(car.getModel()) % 11))) % m_currentCap;
		
		i++;
	}

	m_currentTable[index] = car;
	m_currentSize++;

	//Check for rehashing criteria
	if (lambda() > 0.5 && m_oldTable == NULL)
		Currenttable_to_oldtable();

	if (m_oldTable != NULL) 	//if true, mean increamental transfer is still in progress,
		increamental_Transfer();

	return true;
}

void CarDB::Currenttable_to_oldtable()
{
	m_oldTable = m_currentTable;
	m_oldCap = m_currentCap;
	m_oldSize = m_currentSize;
	m_oldNumDeleted = m_currNumDeleted;
	m_oldProbing = m_currProbing;

	m_currentCap = findNextPrime((m_currentSize - m_currNumDeleted) * 4);
	m_currentSize = 0;	m_currNumDeleted = 0;
	m_currentTable = new Car[m_currentCap];
}

bool CarDB::simple_insert(Car car)
{
	if (car == EMPTY)
		return false;
	// Hash the car model to get the index
	int index = m_hash(car.getModel()) % m_currentCap;
	int i = 0;

	// Handle collisions using the current probing policy
	while (m_currentTable[index].getUsed()) {
		if (m_currentTable[index] == car)
			return false; 			// Car already exists, cannot insert duplicates

		// Use quadratic or double-hash probing based on the policy
		if (m_currProbing == QUADRATIC)
			index = (index + (i * i)) % m_currentCap;

		else if (m_currProbing == DOUBLEHASH)
			index = (index + i * (11 - (m_hash(car.getModel()) % 11))) % m_currentCap;

		i++;
	}

	// Insert the car at the calculated index
	m_currentTable[index] = car;
	m_currentSize++;
	return true;
}

void CarDB::increamental_Transfer()
{
	if (m_oldNumDeleted == m_oldSize)
	{
		delete[] m_oldTable;
		m_oldTable = nullptr;
		m_oldCap = 0;
		m_oldSize = 0;
		m_oldNumDeleted = 0;
		return;
	}
	int numToTransfer = static_cast<int>(floor(0.25 * m_oldSize));
	while (numToTransfer > 0 && m_oldSize > 0) {
		for (int j = 0; j < m_oldCap && numToTransfer > 0; j++) {
			if (m_oldTable[j].getUsed() && !m_oldTable[j].getModel().empty()) {
				// Transfer live data and mark as deleted in the old table
				simple_insert(m_oldTable[j]); //to avoid recursion
				m_oldTable[j].setUsed(false);
				m_oldNumDeleted++;
				numToTransfer--;
				if (m_oldNumDeleted == m_oldSize) break;
			}
		}
		if (m_oldNumDeleted == m_oldSize) break;
	}
}

bool CarDB::remove(Car car) {
	// Implement the removal logic here
	// Hash the car model to get the index
	if (car == EMPTY)
		return false;
	int index = m_hash(car.getModel()) % m_currentCap;
	int i = 0;

	// Handle collisions using the current probing policy
	while (true) {
		// Check if the current bucket contains the target car
		if (m_currentTable[index] == car && m_currentTable[index].getUsed()) {
			// Car found, mark as deleted
			m_currentTable[index].setUsed(false);
			m_currNumDeleted++;

			// Check for rehashing criteria
			if (deletedRatio() > 0.8)
				Currenttable_to_oldtable(); // Convert to oldtable

			if (m_oldTable != NULL)
				increamental_Transfer(); // Continue incremental transfer

			return true;
		}

		// Move to the next bucket using the probing logic
		if (m_currProbing == QUADRATIC)
			index = (index + (i * i)) % m_currentCap;
		else if (m_currProbing == DOUBLEHASH)
			index = (index + i * (11 - (m_hash(car.getModel()) % 11))) % m_currentCap;

		// Check if we have iterated through all possible buckets
		if (i >= m_currentCap)
			break;

		i++;
	}

	// Car not found
	i = 0;
	if (m_oldTable != NULL) {
		while (true) {
			// Check if the current bucket contains the target car
			if (m_oldTable[index] == car && m_oldTable[index].getUsed()) {
				// Car found, mark as deleted
				m_oldTable[index].setUsed(false);
				m_oldNumDeleted++;

				return true;
			}

			// Move to the next bucket using the probing logic
			if (m_oldProbing == QUADRATIC)
				index = (index + (i * i)) % m_oldCap;
			else if (m_oldProbing == DOUBLEHASH)
				index = (index + i * (11 - (m_hash(car.getModel()) % 11))) % m_oldCap;

			// Check if we have iterated through all possible buckets
			if (i >= m_oldCap)
				break;

			i++;
		}
	}

	return false; // Car not found
}

Car CarDB::getCar(string model, int dealer) const {
	// Implement the search logic here
	// Hash the car model to get the index
	int index = m_hash(model) % m_currentCap;
	int i = 0;

	// Search in the current table
	while (m_currentTable[index].getUsed()) {
		if (m_currentTable[index].getModel() == model && m_currentTable[index].getDealer() == dealer) {
			// Car found in the current table
			return m_currentTable[index];
		}

		// Use quadratic or double-hash probing based on the current probing policy
		if (m_currProbing == QUADRATIC) {
			index = (index + (i * i)) % m_currentCap;
		}
		else if (m_currProbing == DOUBLEHASH) {
			index = (index + i * (11 - (m_hash(model) % 11))) % m_currentCap;
		}

		i++;
	}

	// Search in the old table if it exists
	if (m_oldTable != nullptr) {
		// Hash the car model to get the index
		index = m_hash(model) % m_oldCap;
		i = 0;

		while (m_oldTable[index].getUsed()) {
			if (m_oldTable[index].getModel() == model && m_oldTable[index].getDealer() == dealer) {
				// Car found in the old table
				return m_oldTable[index];
			}

			// Use quadratic or double-hash probing based on the old probing policy
			if (m_oldProbing == QUADRATIC) {
				index = (index + (i * i)) % m_oldCap;
			}
			else if (m_oldProbing == DOUBLEHASH) {
				index = (index + i * (11 - (m_hash(model) % 11))) % m_oldCap;
			}

			i++;
		}
	}

	// Car not found
	return EMPTY;
}

float CarDB::lambda() const {
	// Calculate and return the load factor of the current table
	float totalOccupied = m_currentSize + m_currNumDeleted;
	return totalOccupied / m_currentCap;
}

float CarDB::deletedRatio() const {
	// Calculate and return the ratio of deleted buckets to the total number of occupied buckets
	float totalOccupied = m_currentSize;
	if (totalOccupied == 0) {
		return 0.0; // Avoid division by zero
	}
	return static_cast<float>(m_currNumDeleted) / totalOccupied;
}

void CarDB::dump() const {
	cout << "Dump for the current table: " << endl;
	if (m_currentTable != nullptr)
		for (int i = 0; i < m_currentCap; i++) {
			cout << "[" << i << "] : " << m_currentTable[i] << endl;
		}
	cout << "Dump for the old table: " << endl;
	if (m_oldTable != nullptr)
		for (int i = 0; i < m_oldCap; i++) {
			cout << "[" << i << "] : " << m_oldTable[i] << endl;
		}
}

bool CarDB::updateQuantity(Car car, int quantity) {
	// Hash the car model to get the index
	int index = m_hash(car.getModel()) % m_currentCap;
	int i = 0;

	// Search in the current table
	while (m_currentTable[index].getUsed()) {
		if (m_currentTable[index] == car) {
			// Car found, update its quantity
			m_currentTable[index].setQuantity(quantity);

			// Check for rehashing criteria and perform incremental rehashing if needed

			return true;
		}

		// Use quadratic or double-hash probing based on the current probing policy
		if (m_currProbing == QUADRATIC) {
			index = (index + (i * i)) % m_currentCap;
		}
		else if (m_currProbing == DOUBLEHASH) {
			index = (index + i * (11 - (m_hash(car.getModel()) % 11))) % m_currentCap;
		}

		i++;
	}

	// Search in the old table if it exists
	if (m_oldTable != nullptr) {
		// Hash the car model to get the index
		index = m_hash(car.getModel()) % m_oldCap;
		i = 0;

		while (m_oldTable[index].getUsed()) {
			if (m_oldTable[index] == car) {
				// Car found in the old table, update its quantity
				m_oldTable[index].setQuantity(quantity);

				// Check for rehashing criteria and perform incremental rehashing if needed

				return true;
			}
			// Use quadratic or double-hash probing based on the old probing policy
			if (m_oldProbing == QUADRATIC) {
				index = (index + (i * i)) % m_oldCap;
			}
			else if (m_oldProbing == DOUBLEHASH) {
				index = (index + i * (11 - (m_hash(car.getModel()) % 11))) % m_oldCap;
			}

			i++;
		}
	}

	// Car not found
	return false;
}

bool CarDB::isPrime(int number) {
	bool result = true;
	for (int i = 2; i <= number / 2; ++i) {
		if (number % i == 0) {
			result = false;
			break;
		}
	}
	return result;
}

int CarDB::findNextPrime(int current) {
	//we always stay within the range [MINPRIME-MAXPRIME]
	//the smallest prime starts at MINPRIME
	if (current < MINPRIME) current = MINPRIME - 1;
	for (int i = current; i < MAXPRIME; i++) {
		for (int j = 2; j * j <= i; j++) {
			if (i % j == 0)
				break;
			else if (j + 1 > sqrt(i) && i != current) {
				return i;
			}
		}
	}
	//if a user tries to go over MAXPRIME
	return MAXPRIME;
}

ostream& operator<<(ostream& sout, const Car& car) {
	if (!car.m_model.empty())
		sout << car.m_model << " (" << car.m_dealer << "," << car.m_quantity << ")";
	else
		sout << "";
	return sout;
}

bool operator==(const Car& lhs, const Car& rhs) {
	// since the uniqueness of an object is defined by model and delaer
	// the equality operator considers only those two criteria
	return ((lhs.m_model == rhs.m_model) && (lhs.m_dealer == rhs.m_dealer));
}