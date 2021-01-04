#ifndef TETRIS_HPP
#define TETRIS_HPP

#include <string>
#include <utility>
#include <vector>

#include "observer.hpp"
#include "tetromino.hpp"
#include "tetromino_generator.hpp"

class Tetris {
public:
    using Position = std::pair<int, int>;
    using Grid = std::vector<std::vector<Tetromino::Color> >;

    static const int GRID_WIDTH = 10;
    static const int GRID_VISIBLE_HEIGHT = 20;
    static const int GRID_FULL_HEIGHT = 40;
    static constexpr Position TETROMINO_INITIAL_POS = {(GRID_WIDTH / 2) - 2,
                                                       (GRID_FULL_HEIGHT / 2) - 1};

    static const int MAX_LEVEL = 15;
    static const int LINES_PER_LEVEL = 10;

    Tetris();
    bool tick(bool isSoftDrop = false);
    void shiftLeft();
    void shiftRight();
    void hardDrop();
    void softDrop();
    void rotateCW();
    void rotateCCW();
    Grid getRawGrid() const;
    Grid getDisplayGrid() const;
    std::string toString() const;
    bool isFinished() const;
    unsigned int getScore() const;
    unsigned int getLevel() const;
    unsigned int getLevelProgress() const;
    double getLevelSpeed() const;

protected:
    bool is_finished_;
    virtual void generateTetromino();

private:
    bool isValidPosition(Position tetromino_position) const;
    Position getHardDropPosition() const;
    unsigned int clearLines();
    void calculateLevelSpeed();

    TetrominoGenerator generator_;
    Tetromino tetromino_;
    Position tetromino_position_;
    Grid grid_;

    unsigned int score_;
    unsigned int level_;
    unsigned int level_progress_;
    double level_speed_;
};

class ObservableTetris : public Tetris, public Subject {
public:
    ObservableTetris() : Tetris() {}

private:
    void generateTetromino() override;
};

#endif