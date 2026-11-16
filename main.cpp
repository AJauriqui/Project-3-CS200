#include <iostream>
#include <string>
#include "visual.h"
#include "waitingRoom.h"

int main(int argc, char* argv[]) {
    // Input paths and seed configuration for testing/headless execution
    std::string patientsPath = (argc > 1) ? argv[1] : "patients.csv";
    std::string doctorsPath = (argc > 2) ? argv[2] : "doctors.csv";
    unsigned seed = (argc > 3) ? (unsigned)std::stoul(argv[3]) : 42; // Fixed seed for reproducible test runs

    Hospital hospital(seed);

    if (!hospital.loadPatients(patientsPath)) {
        std::cerr << "Could not read patients from '" << patientsPath << "'.\n";
        return 1;
    }
    if (!hospital.loadDoctors(doctorsPath)) {
        std::cerr << "Could not read doctors from '" << doctorsPath << "'.\n";
        return 1;
    }

    // Initialize the hospital simulation
    hospital.init();

    // Run a fixed number of automated simulation ticks to verify proper testing 
    // without requiring an interactive terminal prompt loop.
    const int simulationHours = 24; // Simulate a full 24-hour cycle
    for (int h = 0; h < simulationHours; ++h) {
        hospital.tick();
    }

    // Launch the visual interface as requested
    runVisualization(hospital);

    return 0;
}