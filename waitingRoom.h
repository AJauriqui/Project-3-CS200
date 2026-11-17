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

	// These vectors own the objects; the B-trees only store pointers to them.
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
		// Repeat shifts 1, 2, and 3 every 24 hours.
		return ((h % (SHIFT_LENGTH * SHIFTS_PER_DAY)) / SHIFT_LENGTH) + 1;
	}

	bool loadPatients(const std::string& path) {
		std::ifstream in(path.c_str());
		if (!in) return false;
		std::string line;
		std::getline(in, line); // skip header
		// Store patient records for later admission into the simulation.
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
				// Keep the condition within the range supported by Patient.
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
				// Add the doctor to the priority tree using the owned object's address.
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
		// The initial patients arrive at hour zero.
		for (int i = 0; i < INITIAL_PATIENTS; ++i) {
			admit(0);
		}
	}

	void tick() {
		// Advance treatment, assign waiting patients, update conditions, and admit new arrivals.
		++hour;
		const int currentShift = shiftAt(hour);

		std::vector<Doctor*> allDocs = doctors.toVector();

		// 1. Busy doctors treat their patient for an hour
		for (size_t i = 0; i < allDocs.size(); ++i) {
			Doctor* doc = allDocs[i];
			if (doc->isAvailable()) continue;
			Patient* p = doc->getPatient();
			doc->hourOfTreatment();
			if (p->getCondition() <= 0) {
				p->setDischargeTime(hour);
				doc->released();
				// The doctor can be assigned another patient on a later tick.
				++dischargeCount;
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
			// Count waiting time when treatment begins.
			totalWaitTime += (hour - p->getArrivalTime());
		}

		// 3. Everyone still waiting gets worse. Conditions change, so the tree is rebuilt to re-sort it.
		std::vector<Patient*> w = waitingRoom.toVector();
		waitingRoom.clear();
		// Reinsert patients because their updated conditions may change their priority.
		for (size_t i = 0; i < w.size(); ++i) {
			w[i]->updateCondition(hour);
			waitingRoom.insert(w[i]);
		}

		// 4. New arrivals
		for (int i = 0; i < ADMISSIONS_PER_HOUR; ++i) {
			admit(hour);
		}
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
		// Shuffle the patient pool again when the current draw order is exhausted.
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
		// Keep the new patient alive while the waiting tree stores its pointer.
		Patient* p = allPatients.back().get();
		waitingRoom.insert(p);
		++totalArrivals;
	}

	static std::vector<std::string> split(const std::string& line) {
		std::vector<std::string> out;
		std::stringstream ss(line);
		std::string field;
		while (std::getline(ss, field, ',')) {
			// Remove surrounding spaces so parsed values are clean.
			size_t a = field.find_first_not_of(" \t\r\n");
			size_t b = field.find_last_not_of(" \t\r\n");
			out.push_back((a == std::string::npos) ? "" : field.substr(a, b - a + 1));
		}
		return out;
	}

};

#endif // WAITINGROOM_H```