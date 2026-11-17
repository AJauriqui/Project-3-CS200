#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "waitingRoom.h"
#include "visual.h"

sf::Color getPatientColor(Patient* p) {
    // Use a different color for patients with more serious conditions.
    if (!p) return sf::Color(59, 130, 246);
    if (p->getIsCritical()) {
        return sf::Color(239, 68, 68);
    } else if (p->getCondition() >= 7) {
        return sf::Color(249, 115, 22);
    } else {
        return sf::Color(59, 130, 246);
    }
}

void runVisualization(Hospital& hospital) {
    // Draw the simulation and handle user input until the window closes.
    sf::RenderWindow window(sf::VideoMode({1200, 800}), "Hospital Waiting Room Simulation (Visual Mode)");
    window.setFramerateLimit(60);

    sf::Font font;
    // Try another system font if the first one is unavailable.
    if (!font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
        if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
            std::cerr << "Warning: Could not load system font.\n";
        }
    }

    bool autoRun = false;
    sf::Clock clock;
    sf::Time elapsedTime = sf::Time::Zero;

    std::vector<Patient*> prevAssignedPatients(4, nullptr);
    std::vector<PatientAnimation> activeAnimations;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape || keyPressed->code == sf::Keyboard::Key::Q) {
                    window.close(); // Pressing ESC or Q will now instantly close the window
                }
                else if (keyPressed->code == sf::Keyboard::Key::Space || keyPressed->code == sf::Keyboard::Key::T) {
                    if (!autoRun) hospital.tick(); // Prevent manual ticks while auto-run is active
                }
                else if (keyPressed->code == sf::Keyboard::Key::R) {
                    autoRun = !autoRun; // Toggle auto-run on/off safely
                    // Start a fresh interval so time spent paused is not counted.
                    clock.restart();
                    elapsedTime = sf::Time::Zero;
                }
            }
            else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mousePressed->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mousePos(mousePressed->position.x, mousePressed->position.y);
                    // This rectangle matches the advance-hour button's position and size.
                    if (mousePos.x >= 950 && mousePos.x <= 1160 && mousePos.y >= 25 && mousePos.y <= 65) {
                        if (!autoRun) hospital.tick();
                    }
                }
            }
        }

        if (autoRun) {
            sf::Time elapsed = clock.restart();
            elapsedTime += elapsed;
            if (elapsedTime >= sf::seconds(1.0f)) { // 1 second per hour in auto-run
                hospital.tick();
                elapsedTime = sf::Time::Zero;
            }
        }

        window.clear(sf::Color(240, 244, 248));

        // --- TITLE & HEADER ---
        sf::Text titleText(font);
        titleText.setCharacterSize(22);
        titleText.setFillColor(sf::Color(30, 41, 59));
        titleText.setStyle(sf::Text::Bold);
        titleText.setString("Hospital Simulation Dashboard (Visual Mode)");
        titleText.setPosition({40.f, 20.f});
        window.draw(titleText);

        sf::Text subText(font);
        subText.setCharacterSize(13);
        subText.setFillColor(sf::Color(100, 116, 139));
        subText.setString("[SPACE] Step Hour  |  [R] Toggle Auto-Run  |  [Q / ESC] Return to Console");
        subText.setPosition({40.f, 48.f});
        window.draw(subText);

        // --- CLOCK & ADVANCE BUTTON ---
        sf::RectangleShape btnBox(sf::Vector2f(210.f, 40.f));
        btnBox.setPosition({950.f, 25.f});
        btnBox.setFillColor(sf::Color(37, 99, 235));
        window.draw(btnBox);

        sf::Text btnText(font);
        btnText.setCharacterSize(14);
        btnText.setFillColor(sf::Color::White);
        btnText.setStyle(sf::Text::Bold);
        btnText.setString("Advance Hour (+1hr)");
        btnText.setPosition({965.f, 36.f});
        window.draw(btnText);

        sf::RectangleShape clockBox(sf::Vector2f(210.f, 40.f));
        clockBox.setPosition({950.f, 75.f});
        clockBox.setFillColor(sf::Color::White);
        clockBox.setOutlineColor(sf::Color(203, 213, 225));
        clockBox.setOutlineThickness(1.f);
        window.draw(clockBox);

        sf::Text clockText(font);
        clockText.setCharacterSize(15);
        clockText.setFillColor(sf::Color(15, 23, 42));
        clockText.setStyle(sf::Text::Bold);
        int currentHour = hospital.getHour();
        // Convert the total elapsed hours into the displayed day and hour.
        int day = (currentHour / 24) + 1;
        int hrOfDay = currentHour % 24;
        int currentShift = Hospital::shiftAt(currentHour);
        clockText.setString("Day " + std::to_string(day) + " - " + (hrOfDay < 10 ? "0" : "") + std::to_string(hrOfDay) + ":00 (Shift " + std::to_string(currentShift) + ") " + (autoRun ? "[AUTO]" : ""));
        clockText.setPosition({960.f, 86.f});
        window.draw(clockText);

        // --- LEFT HALF: WAITING ROOM ---
        sf::RectangleShape waitingBox(sf::Vector2f(560.f, 640.f));
        waitingBox.setPosition({40.f, 130.f});
        waitingBox.setFillColor(sf::Color::White);
        waitingBox.setOutlineColor(sf::Color(203, 213, 225));
        waitingBox.setOutlineThickness(2.f);
        window.draw(waitingBox);

        sf::Text waitingHeader(font);
        waitingHeader.setCharacterSize(18);
        waitingHeader.setFillColor(sf::Color(15, 23, 42));
        waitingHeader.setStyle(sf::Text::Bold);
        waitingHeader.setString("Waiting Room");
        waitingHeader.setPosition({60.f, 145.f});
        window.draw(waitingHeader);

        std::vector<Patient*> waitingPatients = hospital.getWaitingPatients();
        // Place waiting patients in rows inside the waiting room panel.
        float startX = 70.f;
        float startY = 195.f;
        float xOff = 0.f;
        float yOff = 0.f;

        for (size_t i = 0; i < waitingPatients.size(); ++i) {
            Patient* p = waitingPatients[i];

            sf::CircleShape node(20.f);
            node.setPosition({startX + xOff, startY + yOff});
            node.setFillColor(getPatientColor(p));
            node.setOutlineColor(sf::Color::Black);
            node.setOutlineThickness(1.f);
            window.draw(node);

            sf::Text lbl(font);
            lbl.setCharacterSize(10);
            lbl.setFillColor(sf::Color::Black);
            lbl.setString(p->getName() + "\nC:" + std::to_string(p->getCondition()));
            lbl.setPosition({startX + xOff - 5.f, startY + yOff + 42.f});
            window.draw(lbl);

            xOff += 90.f;
            if (xOff >= 450.f) {
                // Reset the column and move down to the next row.
                xOff = 0.f;
                yOff += 80.f;
            }
        }

        // --- RIGHT HALF: 4 DOCTOR OFFICES ---
        std::vector<Doctor*> allDocs = hospital.getAllDoctors();
        std::vector<Doctor*> activeShiftDocs;
        // Only show doctors assigned to the current shift.
        for (auto* doc : allDocs) {
            if (doc->getShift() == currentShift) {
                activeShiftDocs.push_back(doc);
            }
        }

        float officeX = 620.f;
        float officeY = 130.f;
        float officeHeight = 152.f;
        float officeWidth = 540.f;

        for (int i = 0; i < 4; ++i) {
            sf::RectangleShape officeBox(sf::Vector2f(officeWidth, officeHeight));
            officeBox.setPosition({officeX, officeY});
            officeBox.setFillColor(sf::Color::White);
            officeBox.setOutlineColor(sf::Color(203, 213, 225));
            officeBox.setOutlineThickness(2.f);
            window.draw(officeBox);

            if (i < activeShiftDocs.size()) {
                Doctor* doc = activeShiftDocs[i];
                bool isBusy = !doc->isAvailable();
                Patient* currentPat = isBusy ? doc->getPatient() : nullptr;

                if (currentPat != prevAssignedPatients[i] && currentPat != nullptr) {
                    // Start an animation when a doctor receives a new patient.
                    PatientAnimation anim;
                    anim.patientName = currentPat->getName();
                    anim.startPos = sf::Vector2f(300.f, 400.f);
                    anim.targetPos = sf::Vector2f(officeX + 110.f, officeY + 90.f);
                    anim.currentPos = anim.startPos;
                    anim.progress = 0.f;
                    anim.active = true;
                    activeAnimations.push_back(anim);
                }
                // Remember the assignment so the animation only starts for a new patient.
                prevAssignedPatients[i] = currentPat;

                sf::Text docTxt(font);
                docTxt.setCharacterSize(13);
                docTxt.setFillColor(sf::Color::Black);
                std::string pName = isBusy ? currentPat->getName() : "None";
                int pCond = isBusy ? currentPat->getCondition() : 0;
                
                docTxt.setString("Office " + std::to_string(i+1) + ": Dr. " + doc->getName() + 
                                 "  (Skill: " + std::to_string(doc->getSkill()) + ", Shift: " + std::to_string(doc->getShift()) + ")\n" +
                                 "Status: " + (isBusy ? "Busy with Patient -> " + pName + " (Cond: " + std::to_string(pCond) + ")" : "Available"));
                docTxt.setPosition({officeX + 20.f, officeY + 20.f});
                window.draw(docTxt);

                sf::CircleShape docSprite(16.f);
                docSprite.setPosition({officeX + 40.f, officeY + 90.f});
                docSprite.setFillColor(sf::Color(16, 185, 129));
                window.draw(docSprite);

                sf::Text dLbl(font);
                dLbl.setCharacterSize(10);
                dLbl.setFillColor(sf::Color::Black);
                dLbl.setString("Dr." + doc->getName());
                dLbl.setPosition({officeX + 25.f, officeY + 128.f});
                window.draw(dLbl);

                bool isCurrentlyAnimating = false;
                // Avoid drawing a second patient sprite while its moving sprite is visible.
                for (const auto& anim : activeAnimations) {
                    if (anim.patientName == pName && anim.active) {
                        isCurrentlyAnimating = true;
                        break;
                    }
                }

                if (isBusy && !isCurrentlyAnimating) {
                    sf::CircleShape patSprite(16.f);
                    patSprite.setPosition({officeX + 110.f, officeY + 90.f});
                    patSprite.setFillColor(getPatientColor(currentPat));
                    patSprite.setOutlineColor(sf::Color::Black);
                    patSprite.setOutlineThickness(1.f);
                    window.draw(patSprite);

                    sf::Text pLbl(font);
                    pLbl.setCharacterSize(10);
                    pLbl.setFillColor(sf::Color::Black);
                    pLbl.setString(pName + "\n(C:" + std::to_string(pCond) + ")");
                    pLbl.setPosition({officeX + 95.f, officeY + 128.f});
                    window.draw(pLbl);
                }
            } else {
                sf::Text emptyTxt(font);
                emptyTxt.setCharacterSize(14);
                emptyTxt.setFillColor(sf::Color(148, 163, 184));
                emptyTxt.setString("Doctor Office " + std::to_string(i+1) + " (No Doctor on Shift)");
                emptyTxt.setPosition({officeX + 20.f, officeY + 20.f});
                window.draw(emptyTxt);
            }

            officeY += 162.f;
        }

        // --- ANIMATIONS ---
        // Move newly assigned patients toward their doctor's office.
        for (auto it = activeAnimations.begin(); it != activeAnimations.end(); ) {
            it->progress += 0.05f;
            if (it->progress >= 1.0f) {
                it->progress = 1.0f;
                it->active = false;
            }
            // Move the patient a little farther toward the office each frame.
            it->currentPos.x = it->startPos.x + (it->targetPos.x - it->startPos.x) * it->progress;
            it->currentPos.y = it->startPos.y + (it->targetPos.y - it->startPos.y) * it->progress;

            sf::Color movingColor(59, 130, 246);
            // Find the assigned patient to keep the moving sprite's color up to date.
            for (auto* doc : allDocs) {
                if (!doc->isAvailable() && doc->getPatient()->getName() == it->patientName) {
                    movingColor = getPatientColor(doc->getPatient());
                    break;
                }
            }

            sf::CircleShape movingNode(16.f);
            movingNode.setPosition(it->currentPos);
            movingNode.setFillColor(movingColor);
            movingNode.setOutlineColor(sf::Color::Black);
            movingNode.setOutlineThickness(1.f);
            window.draw(movingNode);

            sf::Text moveLbl(font);
            moveLbl.setCharacterSize(11);
            moveLbl.setFillColor(sf::Color::Black);
            moveLbl.setString(it->patientName);
            moveLbl.setPosition({it->currentPos.x - 5.f, it->currentPos.y - 20.f});
            window.draw(moveLbl);

            if (!it->active) {
                // erase returns the next valid iterator after removing this animation.
                it = activeAnimations.erase(it);
            } else {
                ++it;
            }
        }

        window.display();
    }
}