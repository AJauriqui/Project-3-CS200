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
	Patient(const std::string& name, const std::string& birthday, int age, int condition, int arrivalTime, int criticalTime) :
		name(name), birthday(birthday), age(age), condition(condition), arrivalTime(arrivalTime), criticalTime(criticalTime), treatmentTime(0), startTime(-1), dischargeTime(-1), isCritical(false) {
	}
	std::string getName() const { return name; }
    std::string getBirthday() const { return birthday; }
    int getAge() const { return age; }
    int getCondition() const { return condition; }
    int getArrivalTime() const { return arrivalTime; }
    int getCriticalTime() const { return criticalTime; }
    int getTreatmentTime() const { return treatmentTime; }
    int getStartTime() const { return startTime; }
    int getDischargeTime() const { return dischargeTime; }
    bool getIsCritical() const { return isCritical; }

    void setCondition(int c) {
		if (c < 0) c = 0;
		if (c > WORST_CONDITION) c = WORST_CONDITION;
		condition = c;
	};
    void setTreatmentTime(int h) {
        treatmentTime = h;
    }
    void setStartTime(int h) {
        startTime = h;
    }
    void setDischargeTime(int h) {
        dischargeTime = h;
    }
    void setIsCritical(bool critical) {
        isCritical = critical;
    }
    void updateCondition(int currentTime) {
		setCondition(condition + 1); // condition worsens by 1 each hour
		if (!isCritical && (currentTime - arrivalTime) >= criticalTime) {
			isCritical = true;
            return true;
		}
		return false;
    }
};

struct PatientPrio {
	bool operator()(const Patient* x, const Patient* y) const {
		if (x->getCondition() != y->getCondition()) return x->getCondition() > y->getCondition();
		if (x->getArrivalTime() != y->getArrivalTime()) return x->getArrivalTime() < y->getArrivalTime();
		return x->getBirthday() < y->getBirthday();
	}
};

#endif // PATIENT_H