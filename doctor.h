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
	Doctor(const std::string& name, const std::string& dob, int skill, int shift)
		: name(name), dob(dob), skill(skill), shift(shift), currentPatient(nullptr) {}
	std::string getName() const { return name; }
	std::string getDOB() const { return dob; }
	int getSkill() const { return skill; }
	int getShift() const { return shift; }
	Patient* getPatient() const { return currentPatient; }
	bool isAvailable() const { return currentPatient == nullptr; }

	void takePatient(Patient* patient, int hour) {
		currentPatient = patient;
		patient->setStartTime(hour);
	}

	void hourOfTreatment() {
		if (!currentPatient) return;
		currentPatient->setCondition(currentPatient->getCondition() - skill);
			currentPatient->setTreatmentTime(currentPatient->getTreatmentTime() + 1);
	}

	Patient* released() {
		Patient* p = currentPatient;
		currentPatient = nullptr; // remove patient from doctor's care/pointer
		return p;
	}
};


// This orders doctors in their B Tree by their shift, 
// then skill, then name, and then dob (which makes them unique)
// So when assigning doctors, the best per shift is taken first. 
struct DoctorPrio {
	bool operator()(const Doctor* x, const Doctor* y) const {
		if (x->getShift() != y->getShift()) return x->getShift() < y->getShift();
		if (x->getSkill() != y->getSkill()) return x->getSkill() > y->getSkill();
		if (x->getName() != y->getName()) return x->getName() < y->getName();
		return x->getDOB() < y->getDOB();
	}
};

#endif // DOCTOR_H