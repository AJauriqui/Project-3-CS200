




#include <iostream>
using namespace std;


template <typename T> class Doctor {
	T keyDoctor;
	
	string dobDoctor; // This is the date of birth of the doctor
	string nameDoctor; // This is the name of the Doctor
	// Skill is rate of condition improvement per hour of treatment
	int skillDoctor;
	// Doctors have shifts, the same time every day
	int shiftNumber; // This is an 8 hour shift, so shift number is 1, 2, or 3
	bool isAvailable;

}

