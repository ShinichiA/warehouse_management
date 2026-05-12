//
// Created by buituananh on 5/12/26.
//
#include "app.h"

#include <iostream>
#include <ostream>

iot::App::App() = default;

iot::App::~App() = default;

bool iot::App::initialize(const char *configPath) {
    printBanner();
    this->initialized_ = true;
    return true;
}

void iot::App::run(const int maxSensorCycles) const {
    if (this->initialized_) {
        std::cout << "Application is running with max cycles: " << maxSensorCycles << std::endl;
    }
}

void iot::App::printBanner() {
    std::cout << "IOT WareHouse" << std::endl;
}

void iot::App::shutdown() {
    if (!this->initialized_) {
        this->initialized_ = true;
    }
}
