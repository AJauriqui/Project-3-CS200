#include <iostream>
#include <sstream>
#include <string>
#include "visual.h"
#include "waitingRoom.h"

static void printHelp() {
    std::cout << "\nCommands:\n"
        << "  t         advance the clock by 1 hour\n"
        << "  n <k>     advance the clock by k hours\n"
        << "  w         show the waiting room (patient B-tree in priority order)\n"
        << "  d         show the doctors and what they are doing\n"
        << "  b         draw both B-trees and check their invariants\n"
        << "  s         show statistics\n"
        << "  v         open the SFML visualization\n"
        << "  h         show this help\n"
        << "  q         quit\n";
}

int main(int argc, char* argv[]) {
    std::string patientsPath = (argc > 1) ? argv[1] : "patients.csv";
    std::string doctorsPath = (argc > 2) ? argv[2] : "doctors.csv";
    unsigned seed = (argc > 3) ? (unsigned)std::stoul(argv[3]) : std::random_device()();

    Hospital hospital(seed);

    if (!hospital.loadPatients(patientsPath)) {
        std::cerr << "Could not read patients from '" << patientsPath
            << "'. Put patients.csv next to the program (or pass its path as the first argument).\n";
        return 1;
    }
    if (!hospital.loadDoctors(doctorsPath)) {
        std::cerr << "Could not read doctors from '" << doctorsPath
            << "'. Put doctors.csv next to the program (or pass its path as the second argument).\n";
        return 1;
    }

    hospital.init();
    printHelp();

    std::string line;
    while (true) {
        std::cout << "\n[hour " << hospital.getHour() << "] > ";
        if (!std::getline(std::cin, line)) break;

        std::istringstream in(line);
        std::string cmd;
        in >> cmd;
        if (cmd.empty()) continue;

        if (cmd == "t") {
            hospital.tick();
        }
        else if (cmd == "n") {
            int k = 0;
            if (!(in >> k) || k < 1) {
                std::cout << "Usage: n <number of hours>\n";
                continue;
            }
            for (int i = 0; i < k; ++i) hospital.tick();
        }
        else if (cmd == "w") {
            hospital.printWaitingRoom();
        }
        else if (cmd == "d") {
            hospital.printDoctors();
        }
        else if (cmd == "b") {
            hospital.printTrees();
        }
        else if (cmd == "s") {
            hospital.printStats();
        }
        else if (cmd == "v") {
            runVisualization(hospital);
        }   
        else if (cmd == "h") {
            printHelp();
        }
        else if (cmd == "q") {
            break;
        }
        else {
            std::cout << "Unknown command '" << cmd << "'. Type h for help.\n";
        }
    }
    return 0;
}
