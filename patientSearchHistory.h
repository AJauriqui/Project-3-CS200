#ifndef PATIENTSEARCHHISTORY_H
#define PATIENTSEARCHHISTORY_H

#include <iostream>
#include <stack>
#include <string>

//stores the history of patient searches using a stack
class PatientSearchHistory {
private:
	std::stack<std::string> history;
public: 
    /*Adds patients information to the search history
    Input is a string of patients information, no output*/
    void pushSearch(const std::string& patientInfo){
        history.push(patientInfo);
    }
    /*Display's patient search history from newest to oldest
    No input, output is a list of patient information strings*/
    void printHistory() const{
        if(history.empty()){
            std::cout << "No search history available." << std::endl;
            return;
        }
        std::stack<std::string> tempHistory = history; // Create a copy to preserve the original stack
        std::cout << "Patient Search History:" << std::endl;
        int count=1;
        while(!tempHistory.empty()){
            std::cout << count << ". " << tempHistory.top() << std::endl;
            tempHistory.pop();
            count++;
        }
    }
};
#endif //PATIENTSEARCHHISTORY_H


	