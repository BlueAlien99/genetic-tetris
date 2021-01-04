#include "tetris/tetris.hpp"

#define _USE_MATH_DEFINES
#include <algorithm>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

#include "tetris/tetromino.hpp"
#include "tetris/tetromino_generator.hpp"

Tetris::Tetris() : score_(0), level_(1), level_progress_(0) {
    for (int i = 0; i < GRID_FULL_HEIGHT; ++i) {
        std::vector<Tetromino::Color> grid_line(GRID_WIDTH, Tetromino::Color::EMPTY);
        grid_.push_back(grid_line);
    }
    is_finished_ = true;

    generateTetromino();  // is_finished_ needs to be true before this statement otherwise notifying
                          // on tetromino change will break
    is_finished_ = false;
    calculateLevelSpeed();
}

bool Tetris::tick(bool isSoftDrop) {
    if (is_finished_) {
        return false;
    }
    --tetromino_position_.second;
    if (isValidPosition(tetromino_position_)) {
        return false;
    }
    ++tetromino_position_.second;
    for (const Tetromino::Square& square : tetromino_.getSquares()) {
        int x = tetromino_position_.first + square.first;
        int y = tetromino_position_.second + square.second;

        //      The condition below should never evaluate to true and is kinda pointless,
        //      but I'm leaving it in case somebody changed GRID_VISIBLE_HEIGHT
        //      and / or GRID_FULL_HEIGHT to some strange values.
        if (y >= GRID_FULL_HEIGHT) {
            is_finished_ = true;
            return false;
        }

        grid_[y][x] = tetromino_.getColor();
    }

    unsigned int cleared_lines = clearLines();
    level_progress_ += cleared_lines;
    if(level_progress_ >= LINES_PER_LEVEL){
        level_progress_ = 0;
        ++level_;
        calculateLevelSpeed();
    }

    generateTetromino();
    return true;
}

void Tetris::shiftLeft() {
    --tetromino_position_.first;
    if (!isValidPosition(tetromino_position_)) {
        ++tetromino_position_.first;
    }
}

void Tetris::shiftRight() {
    ++tetromino_position_.first;
    if (!isValidPosition(tetromino_position_)) {
        --tetromino_position_.first;
    }
}

void Tetris::hardDrop() {
    tetromino_position_ = getHardDropPosition();
    tick();
}

void Tetris::softDrop() {}

void Tetris::rotateCW() {
    tetromino_.rotateCW();
    while (!isValidPosition(tetromino_position_)) {
        tetromino_.rotateCW();
    }
}

void Tetris::rotateCCW() {
    tetromino_.rotateCCW();
    while (!isValidPosition(tetromino_position_)) {
        tetromino_.rotateCCW();
    }
}

Tetris::Grid Tetris::getRawGrid() const { return grid_; }

Tetris::Grid Tetris::getDisplayGrid() const {
    Grid static_grid;
    for (int i = 0; i < GRID_VISIBLE_HEIGHT; ++i) {
        std::vector<Tetromino::Color> line(grid_[i]);
        static_grid.push_back(line);
    }
    Position ghost_piece_pos = getHardDropPosition();
    for (const Tetromino::Square& square : tetromino_.getSquares()) {
        int x = ghost_piece_pos.first + square.first;
        int y = ghost_piece_pos.second + square.second;
        if (y < GRID_VISIBLE_HEIGHT) {
            static_grid[y][x] = Tetromino::Color::GHOST;
        }
    }
    for (const Tetromino::Square& square : tetromino_.getSquares()) {
        int x = tetromino_position_.first + square.first;
        int y = tetromino_position_.second + square.second;
        if (y < GRID_VISIBLE_HEIGHT) {
            static_grid[y][x] = tetromino_.getColor();
        }
    }
    return static_grid;
}

std::string Tetris::toString() const {
    Grid static_grid = getDisplayGrid();
    std::string str = "";
    for (int i = GRID_VISIBLE_HEIGHT - 1; i >= 0; --i) {
        for (int j = 0; j < GRID_WIDTH; ++j) {
            if (static_grid[i][j] == Tetromino::Color::EMPTY) {
                str += "_";
            } else {
                str += "#";
            }
        }
        str += "\n";
    }
    return str;
}

bool Tetris::isFinished() const { return is_finished_; }

unsigned int Tetris::getScore() const { return score_; }

unsigned int Tetris::getLevel() const { return level_; }

unsigned int Tetris::getLevelProgress() const { return level_progress_; }

double Tetris::getLevelSpeed() const { return level_speed_; }

bool Tetris::isValidPosition(Position tetromino_position) const {
    for (const Tetromino::Square& square : tetromino_.getSquares()) {
        int x = tetromino_position.first + square.first;
        int y = tetromino_position.second + square.second;
        if (x < 0 || x > GRID_WIDTH - 1 || y < 0) {
            return false;
        }
        if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_FULL_HEIGHT &&
            grid_[y][x] != Tetromino::Color::EMPTY) {
            return false;
        }
    }
    return true;
}

Tetris::Position Tetris::getHardDropPosition() const {
    Position drop_pos = tetromino_position_;
    do {
        --drop_pos.second;
    } while (isValidPosition(drop_pos));
    ++drop_pos.second;
    return drop_pos;
}

unsigned int Tetris::clearLines(){
    unsigned int cleared = 0;
    int i = 0;
    while (i < GRID_FULL_HEIGHT) {
        bool is_filled_line = true;
        for (int j = 0; j < GRID_WIDTH; ++j) {
            if (grid_[i][j] == Tetromino::Color::EMPTY) {
                is_filled_line = false;
                break;
            }
        }
        if (is_filled_line) {
            grid_.erase(grid_.begin() + i);
            std::vector<Tetromino::Color> grid_line(GRID_WIDTH, Tetromino::Color::EMPTY);
            grid_.push_back(grid_line);
            ++cleared;
        } else {
            ++i;
        }
    }
    return cleared;
}

/**
 * https://tetris.fandom.com/wiki/Tetris_Worlds#Gravity
 */
void Tetris::calculateLevelSpeed() {
    double speed = pow(0.8 - ((level_ - 1) * 0.007), level_ - 1);
    level_speed_ = speed;
}

void Tetris::generateTetromino() {
    tetromino_ = generator_.getNextTetromino();
    tetromino_position_ = TETROMINO_INITIAL_POS;
    if (tetromino_.getShape() == Tetromino::Shape::I) {
        --tetromino_position_.second;
    }
    if (!isValidPosition(tetromino_position_)) {
        is_finished_ = true;
        return;
    }
    if (isValidPosition({tetromino_position_.first, tetromino_position_.second - 1})) {
        --tetromino_position_.second;
    }
}

void ObservableTetris::generateTetromino() {
    Tetris::generateTetromino();
    if (!is_finished_) {  // don't notify at the start of the game
        notifyObservers(
            GenTetrisEvent::TETROMINO_DROPPED);  // notify genetic algorithm that a tetromino has
                                                 // changed (it should then execute hardDrop())
    }
}
