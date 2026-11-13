#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "waitingRoom.h"
#include "visual.h"

int main(int argc, char* argv[]) {
    std::string patientsPath = (argc > 1) ? argv[1] : "patients.csv";
    std::string doctorsPath = (argc > 2) ? argv[2] : "doctors.csv";
    unsigned seed = (argc > 3) ? (unsigned)std::stoul(argv[3]) : 42;

    Hospital hospital(seed);

    if (!hospital.loadPatients(patientsPath)) {
        std::cerr << "Warning: Could not load " << patientsPath << ".\n";
    }
    if (!hospital.loadDoctors(doctorsPath)) {
        std::cerr << "Warning: Could not load " << doctorsPath << ".\n";
    }

    hospital.init();

    sf::RenderWindow window(sf::VideoMode(1200, 800), "Hospital Waiting Room Simulation");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
        if (!font.loadFromFile("/System/Library/Fonts/Helvetica.ttc")) {
            std::cerr << "Warning: Could not load system font.\n";
        }
    }

    bool autoRun = false;
    sf::Clock clock;
    sf::Time elapsedTime = sf::Time::Zero;

    std::vector<bool> prevDoctorBusy(4, false);
    std::vector<PatientAnimation> activeAnimations;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::Space || event.key.code == sf::Keyboard::T) {
                    hospital.tick();
                }
                else if (event.key.code == sf::Keyboard::R) {
                    autoRun = !autoRun;
                }
            }

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);
                if (mousePos.x >= 950 && mousePos.x <= 1160 && mousePos.y >= 25 && mousePos.y <= 65) {
                    hospital.tick();
                }
            }
        }

        if (autoRun) {
            sf::Time elapsed = clock.restart();
            elapsedTime += elapsed;
            if (elapsedTime >= sf::seconds(1.0f)) {
                hospital.tick();
                elapsedTime = sf::Time::Zero;
            }
        }

        window.clear(sf::Color(240, 244, 248));

        // --- TITLE & HEADER ---
        sf::Text titleText;
        titleText.setFont(font);
        titleText.setCharacterSize(22);
        titleText.setFillColor(sf::Color(30, 41, 59));
        titleText.setStyle(sf::Text::Bold);
        titleText.setString("Hospital Simulation Dashboard");
        titleText.setPosition(40, 20);
        window.draw(titleText);

        sf::Text subText;
        subText.setFont(font);
        subText.setCharacterSize(13);
        subText.setFillColor(sf::Color(100, 116, 139));
        subText.setString("[SPACE] Step Hour  |  [R] Auto-Run (1h/sec)  |  [ESC] Quit");
        subText.setPosition(40, 48);
        window.draw(subText);

        // --- CLOCK & ADVANCE BUTTON ---
        sf::RectangleShape btnBox(sf::Vector2f(210, 40));
        btnBox.setPosition(950, 25);
        btnBox.setFillColor(sf::Color(37, 99, 235));
        window.draw(btnBox);

        sf::Text btnText;
        btnText.setFont(font);
        btnText.setCharacterSize(14);
        btnText.setFillColor(sf::Color::White);
        btnText.setStyle(sf::Text::Bold);
        btnText.setString("Advance Hour (+1hr)");
        btnText.setPosition(965, 36);
        window.draw(btnText);

        sf::RectangleShape clockBox(sf::Vector2f(210, 40));
        clockBox.setPosition(950, 75);
        clockBox.setFillColor(sf::Color::White);
        clockBox.setOutlineColor(sf::Color(203, 213, 225));
        clockBox.setOutlineThickness(1.f);
        window.draw(clockBox);

        sf::Text clockText;
        clockText.setFont(font);
        clockText.setCharacterSize(15);
        clockText.setFillColor(sf::Color(15, 23, 42));
        clockText.setStyle(sf::Text::Bold);
        int currentHour = hospital.getHour();
        int day = (currentHour / 24) + 1;
        int hrOfDay = currentHour % 24;
        clockText.setString("Day " + std::to_string(day) + " - " + (hrOfDay < 10 ? "0" : "") + std::to_string(hrOfDay) + ":00 hrs");
        clockText.setPosition(965, 86);
        window.draw(clockText);

        // --- LEFT HALF: WAITING ROOM ---
        sf::RectangleShape waitingBox(sf::Vector2f(560, 640));
        waitingBox.setPosition(40, 130);
        waitingBox.setFillColor(sf::Color::White);
        waitingBox.setOutlineColor(sf::Color(203, 213, 225));
        waitingBox.setOutlineThickness(2.f);
        window.draw(waitingBox);

        sf::Text waitingHeader;
        waitingHeader.setFont(font);
        waitingHeader.setCharacterSize(18);
        waitingHeader.setFillColor(sf::Color(15, 23, 42));
        waitingHeader.setStyle(sf::Text::Bold);
        waitingHeader.setString("Waiting Room");
        waitingHeader.setPosition(60, 145);
        window.draw(waitingHeader);

        // Using hospital's waitingRoom tree converted to vector via available methods
        std::vector<Patient*> waitingPatients = hospital.getWaitingPatients();
        float startX = 70.f;
        float startY = 195.f;
        float xOff = 0.f;
        float yOff = 0.f;

        for (size_t i = 0; i < waitingPatients.size(); ++i) {
            Patient* p = waitingPatients[i];

            sf::CircleShape node(20.f);
            node.setPosition(startX + xOff, startY + yOff);
            if (p->getIsCritical()) {
                node.setFillColor(sf::Color(239, 68, 68));
            } else if (p->getCondition() >= 7) {
                node.setFillColor(sf::Color(249, 115, 22));
            } else {
                node.setFillColor(sf::Color(59, 130, 246));
            }
            node.setOutlineColor(sf::Color::Black);
            node.setOutlineThickness(1.f);
            window.draw(node);

            sf::Text lbl;
            lbl.setFont(font);
            lbl.setCharacterSize(10);
            lbl.setFillColor(sf::Color::Black);
            lbl.setString(p->getName() + "\nC:" + std::to_string(p->getCondition()));
            lbl.setPosition(startX + xOff - 5, startY + yOff + 42);
            window.draw(lbl);

            xOff += 90.f;
            if (xOff >= 450.f) {
                xOff = 0.f;
                yOff += 80.f;
            }
        }

        // --- RIGHT HALF: 4 DOCTOR OFFICES ---
        std::vector<Doctor*> doctorsList = hospital.getAllDoctors();
        float officeX = 620.f;
        float officeY = 130.f;
        float officeHeight = 152.f;
        float officeWidth = 540.f;

        for (int i = 0; i < 4; ++i) {
            sf::RectangleShape officeBox(sf::Vector2f(officeWidth, officeHeight));
            officeBox.setPosition(officeX, officeY);
            officeBox.setFillColor(sf::Color::White);
            officeBox.setOutlineColor(sf::Color(203, 213, 225));
            officeBox.setOutlineThickness(2.f);
            window.draw(officeBox);

            if (i < doctorsList.size()) {
                Doctor* doc = doctorsList[i];
                bool isBusy = !doc->isAvailable();

                if (isBusy && !prevDoctorBusy[i]) {
                    PatientAnimation anim;
                    anim.patientName = doc->getPatient()->getName();
                    anim.startPos = sf::Vector2f(300.f, 400.f);
                    anim.targetPos = sf::Vector2f(officeX + 400.f, officeY + 70.f);
                    anim.currentPos = anim.startPos;
                    anim.progress = 0.f;
                    anim.active = true;
                    activeAnimations.push_back(anim);
                }
                prevDoctorBusy[i] = isBusy;

                sf::Text docTxt;
                docTxt.setFont(font);
                docTxt.setCharacterSize(13);
                docTxt.setFillColor(sf::Color::Black);
                std::string pName = isBusy ? doc->getPatient()->getName() : "None";
                int pCond = isBusy ? doc->getPatient()->getCondition() : 0;
                
                docTxt.setString("Doctor " + std::to_string(i+1) + ": Dr. " + doc->getName() + 
                                 "  (Skill: " + std::to_string(doc->getSkill()) + ", Shift: " + std::to_string(doc->getShift()) + ")\n" +
                                 "Status: " + (isBusy ? "Busy with Patient -> " + pName + " (Cond: " + std::to_string(pCond) + ")" : "Available"));
                docTxt.setPosition(officeX + 20, officeY + 20);
                window.draw(docTxt);

                sf::CircleShape docSprite(16.f);
                docSprite.setPosition(officeX + 40, officeY + 90);
                docSprite.setFillColor(sf::Color(16, 185, 129));
                window.draw(docSprite);

                sf::Text dLbl;
                dLbl.setFont(font);
                dLbl.setCharacterSize(10);
                dLbl.setFillColor(sf::Color::Black);
                dLbl.setString("Dr." + doc->getName());
                dLbl.setPosition(officeX + 25, officeY + 128);
                window.draw(dLbl);

                if (isBusy) {
                    sf::CircleShape patSprite(16.f);
                    patSprite.setPosition(officeX + 110, officeY + 90);
                    patSprite.setFillColor(sf::Color(59, 130, 246));
                    window.draw(patSprite);

                    sf::Text pLbl;
                    pLbl.setFont(font);
                    pLbl.setCharacterSize(10);
                    pLbl.setFillColor(sf::Color::Black);
                    pLbl.setString(pName);
                    pLbl.setPosition(officeX + 100, officeY + 128);
                    window.draw(pLbl);
                }
            } else {
                sf::Text emptyTxt;
                emptyTxt.setFont(font);
                emptyTxt.setCharacterSize(14);
                emptyTxt.setFillColor(sf::Color(148, 163, 184));
                emptyTxt.setString("Doctor Office " + std::to_string(i+1) + " (Unassigned)");
                emptyTxt.setPosition(officeX + 20, officeY + 20);
                window.draw(emptyTxt);
            }

            officeY += 162.f;
        }

        // --- ANIMATIONS ---
        for (auto it = activeAnimations.begin(); it != activeAnimations.end(); ) {
            it->progress += 0.05f;
            if (it->progress >= 1.0f) {
                it->progress = 1.0f;
                it->active = false;
            }
            it->currentPos.x = it->startPos.x + (it->targetPos.x - it->startPos.x) * it->progress;
            it->currentPos.y = it->startPos.y + (it->targetPos.y - it->startPos.y) * it->progress;

            sf::CircleShape movingNode(16.f);
            movingNode.setPosition(it->currentPos);
            movingNode.setFillColor(sf::Color(234, 179, 8));
            window.draw(movingNode);

            sf::Text moveLbl;
            moveLbl.setFont(font);
            moveLbl.setCharacterSize(11);
            moveLbl.setFillColor(sf::Color::Black);
            moveLbl.setString(it->patientName);
            moveLbl.setPosition(it->currentPos.x - 5, it->currentPos.y - 20);
            window.draw(moveLbl);

            if (!it->active) {
                it = activeAnimations.erase(it);
            } else {
                ++it;
            }
        }

        window.display();
    }

    return 0;
}