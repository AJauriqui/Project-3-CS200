#include <doctor.h>
#include <patient.h>
#include <iostream>
using namespace std;

struct Key {
	int condition;
	int arrival;
	int dateOfBirth;

	bool operator<(const Key& other) const {
		if (condition != other.condition) {
			return condition < other.condition;
		}
		if (arrival != other.arrival) {
			return arrival < other.arrival;
		}
		return dateOfBirth < other.dateOfBirth;
	}
	bool operator==(const Key& other) const {
		return condition == other.condition && arrival == other.arrival && dateOfBirth == other.dateOfBirth;
	}
};

struct Node {
	bool leaf;
	vector<Key>   keys;
	vector<Node*> kids;
	Node(bool leaf) : leaf(leaf) {}
};