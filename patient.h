#ifndef PATIENT_H
#define PATIENT_H

#include <string>
using namespace std; 
class Patient {
private:
    string name;
    string birthday;
    int age;
    int condition;
    int arrivalTime;
    int criticalTime;
    int treatmentTime;
    
public:
    Patient(const string& name, const string& birthday, int age, int condition, int arrivalTime, int criticalTime);
    string getName() const;
    string getBirthday() const;
    int getAge() const;
    int getCondition() const;
    int getArrivalTime() const;
    int getCriticalTime() const;
    int getTreatmentTime() const;

    void setCondition(int condition);
    void setTreatmentTime(int treatmentTime);
    void updateCondition(int currentTime);
};

#endif // PATIENT_H