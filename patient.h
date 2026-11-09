#ifndef PATIENT_H
#define PATIENT_H

#include <string>

class Patient {
public:
    static const int WORST_CONDITION = 10;

private:
    std::string name;
    std::string birthday;
    int age;
    int condition;
    int arrivalTime; // hour admitted
    int criticalTime; // hours before patient becomes critical 
    int treatmentTime;
    int startTime; // hour treatment began 
	int dischargeTime; // hour treatment ended
	bool isCritical; // true if patient is critical and needs immediate attention
    
public:
    /*Creates a new patient with information, inputs are patient's name, birthday, age, condition, arrival time, and critical time
    Output is initializing a Patient object with the values*/
	Patient(const std::string& name, const std::string& birthday, int age, int condition, int arrivalTime, int criticalTime):
		name(name), birthday(birthday), age(age), condition(condition), arrivalTime(arrivalTime), criticalTime(criticalTime), treatmentTime(0), startTime(-1), dischargeTime(-1), isCritical(false) {
	}
    /*Returns the patient's name, no input, output is patient's name as a string*/
	std::string getName() const { return name; }
    /*Returns the patient's birthday, no input, output is patient's birthday as a string*/
    std::string getBirthday() const { return birthday; }
    /*Returns the patient's age, no input, output is patient's age as an int*/
    int getAge() const { return age; }
    /*Returns the patient's condition, no input, output is patient's condition as an int*/
    int getCondition() const { return condition; }
    /*Returns the patient's arrival time, no input, output is patient's arrival time as an int*/
    int getArrivalTime() const { return arrivalTime; }
    /*Returns the patient's critical time, no input, output is patient's critical time as an int*/
    int getCriticalTime() const { return criticalTime; }
    /*Returns the patient's treatment time, no input, output is patient's treatment time as an int*/
    int getTreatmentTime() const { return treatmentTime; }
    /*Returns the patient's start time, no input, output is patient's start time as an int*/
    int getStartTime() const { return startTime; }
    /*Returns the patient's discharge time, no input, output is patient's discharge time as an int*/
    int getDischargeTime() const { return dischargeTime; }
    /*Returns true if patient is critical, false otherwise, no input, output is true or false depending*/
    bool getIsCritical() const { return isCritical; }

    /*Updates the patients condition, input is c, the patient's new condition, no output*/
    void setCondition(int c) {
		if (c < 0) c = 0;
		if (c > WORST_CONDITION) c = WORST_CONDITION;
		condition = c;
	};
    /*Updates the patients treatment time, input is h, the number of treatment hours, no output*/
    void setTreatmentTime(int h) {
        treatmentTime = h;
    }
    /*Updates the patients start time, input is h, the hour treatment began, no output*/
    void setStartTime(int h) {
        startTime = h;
    }
    /*Updates the patients discharge time, input is h, the hour treatment ended, no output*/
    void setDischargeTime(int h) {
        dischargeTime = h;
    }
    /*Updates the patients critical status, input is critical, true if patient is critical, false otherwise, no output*/
    void setIsCritical(bool critical) {
        isCritical = critical;
    }

    /*Worsens the patients condition as time goes and checks if patient has reached their critical waiting time
    Input is currentTime, the current hour of the simulation, output is true if the patient just became critical, false otherwise*/
    bool updateCondition(int currentTime) {
		setCondition(condition + 1); // condition worsens by 1 each hour
		if (!isCritical && (currentTime - arrivalTime) >= criticalTime) {//checks if patient has waited long enough to become critical
			isCritical = true;
            return true;//patient has just become critical
		}
		return false;
    }
};
/*Compares patients based on condition, arrival time, and birthday to determine their priority
Inputs are x and y, pointers to two patient objects
Output is true if x comes before y, false otherwise*/
struct PatientPrio {
    /*Compares 2 patients to determine their order, inputs are x and y, pointers to the patients being compared,
    output is true if x has higher priority*/
	bool operator()(const Patient* x, const Patient* y) const {
		if (x->getCondition() != y->getCondition()) return x->getCondition() > y->getCondition();//compare conditions first
		if (x->getArrivalTime() != y->getArrivalTime()) return x->getArrivalTime() < y->getArrivalTime();//if conditions match, compare arrival times
		return x->getBirthday() < y->getBirthday();//if both match use birthday
	}
};

#endif // PATIENT_H