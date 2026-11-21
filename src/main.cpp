#include <Geode/Geode.hpp>

using namespace geode::prelude;

class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        log::info("HawkPath loaded successfully!");

        return true;
    }
};