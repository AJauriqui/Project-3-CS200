#ifndef DOCTOR_H
#define DOCTOR_H

#include <string>
#include "patient.h"

class Doctor {
	
	std::string dob; // This is the date of birth of the doctor
	std::string name; // This is the name of the Doctor
	// Skill is rate of condition improvement per hour of treatment
	int skill;
	// Doctors have shifts, the same time every day
	int shift; // This is an 8 hour shift, so shift number is 1, 2, or 3
	Patient* currentPatient;

public:
	/*Creates a new doctor with given information, inputs are doctors name, DOB, skill and shift
	Output is initializing a Doctor object with the values*/
	Doctor(const std::string& name, const std::string& dob, int skill, int shift)
		: name(name), dob(dob), skill(skill), shift(shift), currentPatient(nullptr) {}
	std::string getName() const { return name; }//returns the doctor's name, no input, output is the doctor's name as a string
	std::string getDOB() const { return dob; }//returns the doctor's date of birth, no input, output is the doctor's date of birth as a string
	int getSkill() const { return skill; }//returns the doctor's skill level, no input, output is the doctor's skill as an int
	int getShift() const { return shift; }//returns the doctor's shift number , no input, output is the shift as an int
	Patient* getPatient() const { return currentPatient; }//returns the current patient assigned to the doctor, no input, output is a pointer to the current patient or nullptr if none assigned
	bool isAvailable() const { return currentPatient == nullptr; }//checks if doctor is available, no input, returns true if the doctor is available, false otherwise

	/*Assigns a patient to the doctor and records when treatment starts, inputs are patient pointer and hour, the current hour
	Output is none*/
	void takePatient(Patient* patient, int hour) {
		currentPatient = patient;
		patient->setStartTime(hour);
	}
	/*Treats the current patient for one hour, no input, output is none*/
	void hourOfTreatment() {
		if (!currentPatient) return;//no patient assigned, so no treatment
		currentPatient->setCondition(currentPatient->getCondition() - skill);//improves patients conditions based on doctor's skill
		currentPatient->setTreatmentTime(currentPatient->getTreatmentTime() + 1);//increases treatment time by 1 hour
	}
	/*Releases the current patient from the doctor's care and makes doctor available again, no input, output is a pointer to the released patient or nullptr if no patient*/
	Patient* released() {
		Patient* p = currentPatient;
		currentPatient = nullptr; // remove patient from doctor's care/pointer
		return p;
	}
};


/*Compares doctors by shift, skill, name, and DOB
Inputs are pointers to two Doctor objects, output is determined by the comparison rules*/ 
struct DoctorPrio {
	/*Compares 2 doctors to determine order, inputs are x and y, pointers to the doctors being compared
	Output is true if x comes before y, false otherwise*/
	bool operator()(const Doctor* x, const Doctor* y) const {
		if (x->getShift() != y->getShift()) return x->getShift() < y->getShift();//compare shifts first
		if (x->getSkill() != y->getSkill()) return x->getSkill() > y->getSkill();//higher skill first
		if (x->getName() != y->getName()) return x->getName() < y->getName();//compare names if skills match
		return x->getDOB() < y->getDOB();//compare DOBs if all else matches
	}
};

#endif // DOCTOR_H