#include "src/IRManager.h"

IRManager manager;

void setup() {
    // Bootstrap the coordinator
    manager.begin();
}

void loop() {
    // Service ticks for all sub-managers
    manager.loop();
}
