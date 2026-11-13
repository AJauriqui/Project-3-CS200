#ifndef WAITINGROOM_H
#define WAITINGROOM_H

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <random>
#include "patient.h"
#include "doctor.h"
#include "BTree.h"

struct PatientData {
	std::string name;
	std::string dob;
	int age;
	int condition;
};

class Hospital {
public:
	static const int INITIAL_PATIENTS = 8;
	static const int ADMISSIONS_PER_HOUR = 2;
	static const int SHIFT_LENGTH = 8;
	static const int SHIFTS_PER_DAY = 3;
	static const int DEGREE = 3; // B-tree degree
	static const int CRITICAL_WAIT = 5; // hours a patient can wait before becoming critical

private:
	BTree<Patient*, PatientPrio> waitingRoom;
	BTree<Doctor*, DoctorPrio> doctors;

	std::vector<std::unique_ptr<Patient>> allPatients;
	std::vector<std::unique_ptr<Doctor>> allDoctors;

	std::vector<PatientData> patientPool;
	std::vector<size_t> drawOrder;
	size_t drawIndex;
	size_t lastAdmitIndex;

	std::mt19937 rng;
	int hour;

	int dischargeCount;
	long totalWaitTime;

	int started;
	int totalArrivals;

public:
	explicit Hospital(unsigned seed = std::random_device()()) : waitingRoom(DEGREE), doctors(DEGREE), drawIndex(0), lastAdmitIndex((size_t)-1), rng(seed), hour(0), dischargeCount(0), totalWaitTime(0), started(0), totalArrivals(0) {}

	int getHour() const { return hour; }

	// Used by the SFML front end
	std::vector<Patient*> getWaitingPatients() const { return waitingRoom.toVector(); }
	std::vector<Doctor*> getAllDoctors() const { return doctors.toVector(); }

	static int shiftAt(int h) {
		return ((h % (SHIFT_LENGTH * SHIFTS_PER_DAY)) / SHIFT_LENGTH) + 1;
	}

	bool loadPatients(const std::string& path) {
		std::ifstream in(path.c_str());
		if (!in) return false;
		std::string line;
		std::getline(in, line); // skip header
		while (std::getline(in, line)) {
			std::vector<std::string> f = split(line);
			if (f.size() < 4) continue;
			try {
				PatientData d;
				d.name = f[0];
				d.age = std::stoi(f[1]);
				d.dob = f[2];
				d.condition = std::stoi(f[3]);
				if (d.condition < 1) d.condition = 1;
				if (d.condition > Patient::WORST_CONDITION) d.condition = Patient::WORST_CONDITION;
				patientPool.push_back(d);
			}
			catch (...) {
				std::cerr << "Error parsing patient line: " << line << std::endl;
			}
		}
		return !patientPool.empty();
	}

	bool loadDoctors(const std::string& path) {
		std::ifstream in(path.c_str());
		if (!in) return false;
		std::string line;
		std::getline(in, line); // skip header
		while (std::getline(in, line)) {
			std::vector<std::string> f = split(line);
			if (f.size() < 4) continue;
			try {
				int skill = std::stoi(f[2]);
				int shift = std::stoi(f[3]);
				if (shift < 1 || shift > SHIFTS_PER_DAY || skill < 1) continue;
				allDoctors.push_back(std::unique_ptr<Doctor>(new Doctor(f[0], f[1], skill, shift)));
				doctors.insert(allDoctors.back().get());
			}
			catch (...) {
				std::cerr << "Error parsing doctor line: " << line << std::endl;
			}
		}
		return !allDoctors.empty();
	}

	void init() {
		std::cout << "Midnight: Hospital opens. Admitting initial " << INITIAL_PATIENTS << " patients\n";
		for (int i = 0; i < INITIAL_PATIENTS; ++i) {
			admit(0);
		}
		printSummary();
	}

	void tick() {
		++hour;
		const int currentShift = shiftAt(hour);
		std::cout << "\n[hour " << hour << "] Shift " << currentShift << " begins\n";

		std::vector<Doctor*> allDocs = doctors.toVector();

		// 1. Busy doctors treat their patient for an hour
		for (size_t i = 0; i < allDocs.size(); ++i) {
			Doctor* doc = allDocs[i];
			if (doc->isAvailable()) continue;
			Patient* p = doc->getPatient();
			int prevCondition = p->getCondition();
			doc->hourOfTreatment();
			std::cout << "Doctor " << doc->getName() << " skill " << doc->getSkill() << " treated patient " << p->getName() << " (condition: " << prevCondition << " -> " << p->getCondition() << ")";
			if (doc->getShift() != currentShift) std::cout << " [overtime]";
			std::cout << std::endl;
			if (p->getCondition() <= 0) {
				p->setDischargeTime(hour);
				doc->released();
				++dischargeCount;
				std::cout << "Patient " << p->getName() << " discharged (waited " << (p->getStartTime() - p->getArrivalTime()) << " hours, treated for " << p->getTreatmentTime() << " hours)";
				if (doc->getShift() != currentShift) std::cout << "; Dr. " << doc->getName() << " goes home";
				std::cout << std::endl;
			}
		}

		// 2. Free doctors on the current shift take the highest priority waiting patients
		for (size_t i = 0; i < allDocs.size(); ++i) {
			Doctor* doc = allDocs[i];
			if (!doc->isAvailable() || doc->getShift() != currentShift) continue;
			Patient* p = nullptr;
			if (!waitingRoom.popFirst(p)) break;
			doc->takePatient(p, hour);
			++started;
			totalWaitTime += (hour - p->getArrivalTime());
			std::cout << "Doctor " << doc->getName() << " skill " << doc->getSkill() << " assigned patient " << p->getName() << " (condition: " << p->getCondition() << ", waited " << (hour - p->getArrivalTime()) << " hours)\n";
		}

		// 3. Everyone still waiting gets worse. Conditions change, so the tree is rebuilt to re-sort it.
		std::vector<Patient*> w = waitingRoom.toVector();
		waitingRoom.clear();
		for (size_t i = 0; i < w.size(); ++i) {
			if (w[i]->updateCondition(hour)) {
				std::cout << "Patient " << w[i]->getName() << " has become critical (condition: " << w[i]->getCondition() << ", waited " << (hour - w[i]->getArrivalTime()) << " hours)\n";
			}
			waitingRoom.insert(w[i]);
		}

		// 4. New arrivals
		for (int i = 0; i < ADMISSIONS_PER_HOUR; ++i) {
			admit(hour);
		}

		printSummary();
	}

	void printSummary() const {
		std::cout << "Waiting room: " << waitingRoom.size() << " patients, " << busyDoctorCount() << "/" << allDoctors.size() << " doctors busy\n";
		std::cout << "Discharged: " << dischargeCount << " patients, average wait time: " << (started > 0 ? (double)totalWaitTime / started : 0.0) << " hours\n";
	}

	void printWaitingRoom() const {
		std::vector<Patient*> w = waitingRoom.toVector();
		std::cout << "Waiting room (priority order):\n";
		std::cout << std::left << std::setw(20) << "Name" << std::setw(12) << "Birthday" << std::setw(6) << "Age" << std::setw(10) << "Condition" << std::setw(10) << "Arrival" << std::setw(10) << "Critical" << "\n";
		for (size_t i = 0; i < w.size(); ++i) {
			std::cout << std::left << std::setw(20) << w[i]->getName() << std::setw(12) << w[i]->getBirthday() << std::setw(6) << w[i]->getAge() << std::setw(10) << w[i]->getCondition() << std::setw(10) << w[i]->getArrivalTime() << std::setw(10) << (w[i]->getIsCritical() ? "Yes" : "No") << "\n";
		}
	}

	void printDoctors() const {
		std::vector<Doctor*> d = doctors.toVector();
		const int currentShift = shiftAt(hour);
		std::cout << "Doctors (current shift: " << currentShift << "):\n";
		// It will only display the doctors' Name, shift, skill, status, and patient
		std::cout << std::left << std::setw(20) << "Name" << std::setw(6) << "Shift" << std::setw(6) << "Skill" << std::setw(10) << "Status" << std::setw(20) << "Patient" << "\n";
		for (size_t i = 0; i < d.size(); ++i) {
			std::cout << std::left << std::setw(20) << d[i]->getName() << std::setw(6) << d[i]->getShift() << std::setw(6) << d[i]->getSkill() << std::setw(10) << (d[i]->isAvailable() ? "Available" : "Busy") << std::setw(20) << (d[i]->isAvailable() ? "None" : d[i]->getPatient()->getName()) << "\n";
		}
	}

	void printTrees() const {
		std::cout << "Patient B-tree (t = " << DEGREE << ", height " << waitingRoom.height() << ", " << waitingRoom.size() << " keys)  label = Name:condition@arrival\n";
		waitingRoom.print(std::cout, [](const Patient* p) { return p->getName() + ":" + std::to_string(p->getCondition()) + "@" + std::to_string(p->getArrivalTime()); });
		std::cout << "Invariants: " << (waitingRoom.checkInvariants() ? "OK" : "VIOLATED") << "\n";
		std::cout << "Doctor B-tree (t = " << DEGREE << ", height " << doctors.height() << ", " << doctors.size() << " keys)  label = Name:skill@shift\n";
		doctors.print(std::cout, [](const Doctor* d) { return d->getName() + ":" + std::to_string(d->getSkill()) + "@" + std::to_string(d->getShift()); });
		std::cout << "Invariants: " << (doctors.checkInvariants() ? "OK" : "VIOLATED") << "\n";
	}

	void printStats() const {
		std::cout << "Hospital statistics at hour " << hour << ":\n";
		std::cout << "Patients admitted: " << totalArrivals << "\n";
		std::cout << "Patients currently waiting: " << waitingRoom.size() << "\n";
		std::cout << "Patients currently being treated: " << busyDoctorCount() << "\n";
		std::cout << "Patients discharged: " << dischargeCount << "\n";
		std::cout << "Average wait time before treatment: " << (started > 0 ? (double)totalWaitTime / started : 0.0) << " hours\n";
	}

private:

	int busyDoctorCount() const {
		int busy = 0;
		for (size_t i = 0; i < allDoctors.size(); ++i) {
			if (!allDoctors[i]->isAvailable()) ++busy;
		}
		return busy;
	}

	void admit(int arrivalHour) {
		if (drawIndex >= drawOrder.size()) {
			drawOrder.resize(patientPool.size());
			for (size_t i = 0; i < patientPool.size(); ++i) drawOrder[i] = i;
			std::shuffle(drawOrder.begin(), drawOrder.end(), rng);

			// avoid admitting the same patient twice in a row across a reshuffle
			if (patientPool.size() > 1 && lastAdmitIndex != (size_t)-1 && drawOrder[0] == lastAdmitIndex) {
				std::swap(drawOrder[0], drawOrder[1]);
			}
			drawIndex = 0;
		}

		lastAdmitIndex = drawOrder[drawIndex++];
		const PatientData& d = patientPool[lastAdmitIndex];
		allPatients.push_back(std::unique_ptr<Patient>(new Patient(d.name, d.dob, d.age, d.condition, arrivalHour, CRITICAL_WAIT)));
		Patient* p = allPatients.back().get();
		waitingRoom.insert(p);
		++totalArrivals;
		std::cout << "Patient " << p->getName() << " admitted (condition: " << p->getCondition() << ", arrival: " << p->getArrivalTime() << ", critical in: " << p->getCriticalTime() << " hours)\n";
	}

	static std::vector<std::string> split(const std::string& line) {
		std::vector<std::string> out;
		std::stringstream ss(line);
		std::string field;
		while (std::getline(ss, field, ',')) {
			size_t a = field.find_first_not_of(" \t\r\n");
			size_t b = field.find_last_not_of(" \t\r\n");
			out.push_back((a == std::string::npos) ? "" : field.substr(a, b - a + 1));
		}
		return out;
	}

};

#endif // WAITINGROOM_H