#ifndef GENETIC_TETRIS_MENU_CONTROLLER_HPP
#define GENETIC_TETRIS_MENU_CONTROLLER_HPP

#include "controller.hpp"

namespace gentetris {

class MenuController : public Controller {
public:
    MenuController(GUI& gui): Controller(gui) {}
    void update() override {}
    void handleSfmlEvent([[maybe_unused]] const sf::Event& e) override {}
    void handleCustomEvent([[maybe_unused]] EventType e) override {}
    void start() override {}
    void reset() override {}
    void finish() override {}
};

}  // namespace gentetris

#endif  // GENETIC_TETRIS_MENU_CONTROLLER_HPP
