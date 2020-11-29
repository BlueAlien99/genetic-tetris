#include "tetris.hpp"

#include "tetromino.hpp"
#include "tetromino_generator.hpp"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

using Square = std::pair<int, int>;
using Squares = std::vector<Square>;

Tetris::Tetris(){
    for(int i = 0; i < GRID_HEIGHT; ++i){
        std::vector<Tetromino::Color> gridLine(GRID_WIDTH, Tetromino::Color::EMPTY);
        grid_.push_back(gridLine);
    }
    generateTetromino();
}

bool Tetris::tick(){
    --tetrominoPosition_.second;
    if(isValidPosition()){
        return false;
    }
    ++tetrominoPosition_.second;
    for(const Square& square : tetromino_.getSquares()){
        int x = tetrominoPosition_.first + square.first;
        int y = tetrominoPosition_.second + square.second;
        grid_[y][x] = tetromino_.getColor();
    }
    for(int i = 0; i < GRID_HEIGHT; ++i){
        bool fullLine = true;
        for(int j = 0; j < GRID_WIDTH; ++j){
            if(grid_[i][j] == Tetromino::Color::EMPTY){
                fullLine = false;
            }
        }
        if(fullLine){
            grid_.erase(grid_.begin()+i);
            std::vector<Tetromino::Color> gridLine(GRID_WIDTH, Tetromino::Color::EMPTY);
            grid_.push_back(gridLine);
        }
    }
    generateTetromino();
    return true;
}

void Tetris::shiftLeft(){
    --tetrominoPosition_.first;
    if(!isValidPosition()){
        ++tetrominoPosition_.first;
    }
}

void Tetris::shiftRight(){
    ++tetrominoPosition_.first;
    if(!isValidPosition()){
        --tetrominoPosition_.first;
    }
}

void Tetris::hardDrop(){
    while(!tick());
}

void Tetris::rotateCW(){
    tetromino_.rotateCW();
    while(!isValidPosition()){
        tetromino_.rotateCW();
    }
}

void Tetris::rotateCCW(){
    tetromino_.rotateCCW();
    while(!isValidPosition()){
        tetromino_.rotateCCW();
    }
}

const std::vector<std::vector<Tetromino::Color> > Tetris::getGrid() const{
    std::vector<std::vector<Tetromino::Color> > staticGrid;
    for(int i = 0; i < GRID_HEIGHT; ++i){
        std::vector<Tetromino::Color> line(grid_[i]);
        staticGrid.push_back(line);
    }
    for(const Square& square : tetromino_.getSquares()){
        int x = tetrominoPosition_.first + square.first;
        int y = tetrominoPosition_.second + square.second;
        if(y < GRID_HEIGHT){
            staticGrid[y][x] = tetromino_.getColor();
        }
    }
    return staticGrid;
}

std::string Tetris::toString() const{
    std::vector<std::vector<Tetromino::Color> > staticGrid = getGrid();
    std::string str = "";
    for(int i = GRID_HEIGHT-1; i >= 0; --i){
        for(int j = 0; j < GRID_WIDTH; ++j){
            if(staticGrid[i][j] == Tetromino::Color::EMPTY){
                str += "_";
            } else{
                str += "#";
            }
        }
        str += "\n";
    }
    return str;
}

bool Tetris::isValidPosition() const{
    for(const Square& square : tetromino_.getSquares()){
        int x = tetrominoPosition_.first + square.first;
        int y = tetrominoPosition_.second + square.second;
        if(x < 0 || x > GRID_WIDTH-1 || y < 0){
            return false;
        }
        if(x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT && grid_[y][x] != Tetromino::Color::EMPTY){
            return false;
        }
    }
    return true;
}

void Tetris::generateTetromino(){
    tetromino_ = generator_.getTetromino();
    tetrominoPosition_ = std::pair((GRID_WIDTH/2)-2, GRID_HEIGHT+1);
}