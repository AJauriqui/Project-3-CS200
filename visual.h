#ifndef VISUAL_H
#define VISUAL_H

#include "SFML/Graphics.hpp"
#include <string>

struct PatientAnimation {
    std::string patientName;
    sf::Vector2f startPos;
    sf::Vector2f targetPos;
    sf::Vector2f currentPos;
    float progress; // 0.0 to 1.0
    bool active;
};

class Hospital;
/*Runs the SFML visualization for the hospital simulation
Input is reference to the intitalized Hospital object, output displays the simulation window and updates*/
void runVisualization(Hospital& hospital);


#endif // VISUAL_H