#include <iostream>
#include "app.h"

int main() {
    iot::App app;
    if (app.initialize("config/config.json")) {
        app.run();
    }
    app.shutdown();
    return 0;
}