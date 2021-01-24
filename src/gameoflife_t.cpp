#include "gameoflife_t.h"

#include <iostream>
#include <stdexcept>
#include <ctime>
#include <vector>
#include <string>

using namespace std;

gameoflife_t::gameoflife_t(size_t _sizeX, size_t _sizeY, bool _wrap_edges) {
    if(_sizeX < 1 || _sizeY < 1)
        throw runtime_error("size of the board too small");

    board = vector<vector<bool>>(_sizeY, vector<bool>(_sizeX, false));
    sizeX = _sizeX;
    sizeY = _sizeY;
    wrap_edges = _wrap_edges;

    live_cells = 0;
}

int gameoflife_t::set_cell(size_t posX, size_t posY, bool state){
    if(posX >= sizeX || posY >= sizeY)
        throw runtime_error("setting non-existent board cell at (" + to_string(posX) + ", " + to_string(posY) + ")");

    if(board[posY][posX] == true && state == false)
        --live_cells;
    if(board[posY][posX] == false && state == true)
        ++live_cells;

    board[posY][posX] = state;
    return 0;
}

int gameoflife_t::set_board(const vector<vector<bool>>& ref_board){
    size_t _sizeY = ref_board.size();
    size_t _sizeX = ref_board[0].size();

    {
        bool are_all_rows_identical = true;
        for(size_t tmp = 0; tmp < ref_board.size(); ++tmp){
            if(ref_board[tmp].size() != _sizeX){
                are_all_rows_identical = false;
                break;
            }
        }

        if(_sizeY != sizeY || _sizeX != sizeX)
            throw runtime_error("can't set board from one of different size");
        if(!are_all_rows_identical)
            throw runtime_error("board provided is not rectangular");
    }

    live_cells = 0;
    for(size_t i = 0; i < sizeY; ++i){
        for(size_t j = 0; j < sizeX; ++j){
            set_cell(j, i, ref_board[i][j]);
        }
    }

    return 0;
}

void gameoflife_t::random_fill(float percentage){
    for(size_t i = 0; i < sizeY; ++i){
        for(size_t j = 0; j < sizeX; ++j){
            if(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/100)) <= percentage){
                set_cell(j, i, 1);
            }
        }
    }
}

int gameoflife_t::count_neighbours(size_t posX, size_t posY){
    if(posX >= sizeX || posY >= sizeY)
        throw runtime_error("tried to check neighbours in non-existent board cell");
    /*
    +----+----+----+
    | UL | U  | UR |
    |    |    |    |
    +----+----+----+
    | L  | XX | R  |
    |    | XX |    |
    +----+----+----+
    | DL | D  | DR |
    |    |    |    |
    +----+----+----+
    */
    if(wrap_edges){
        return  board[(posY == 0 ? sizeY - 1 : posY - 1)][(posX == 0 ? sizeX - 1 : posX - 1)] +     //UL
                board[(posY == 0 ? sizeY - 1 : posY - 1)][(posX)                            ] +     //U
                board[(posY == 0 ? sizeY - 1 : posY - 1)][(posX == sizeX - 1 ? 0 : posX + 1)] +     //UR
                board[(posY)                            ][(posX == 0 ? sizeX - 1 : posX - 1)] +     //L
                board[(posY)                            ][(posX == sizeX - 1 ? 0 : posX + 1)] +     //R
                board[(posY == sizeY - 1 ? 0 : posY + 1)][(posX == 0 ? sizeX - 1 : posX - 1)] +     //DL
                board[(posY == sizeY - 1 ? 0 : posY + 1)][(posX)                            ] +     //D
                board[(posY == sizeY - 1 ? 0 : posY + 1)][(posX == sizeX - 1 ? 0 : posX + 1)] ;     //DR
    } else {
        int num_neighbours = 0;
        if(posY > 0 && posX > 0)                    num_neighbours += board[(posY - 1)][(posX - 1)];    //UL
        if(posY > 0)                                num_neighbours += board[(posY - 1)][(posX)    ];    //U
        if(posY > 0 && posX < sizeX - 1)            num_neighbours += board[(posY - 1)][(posX + 1)];    //UR
        if(posX > 0)                                num_neighbours += board[(posY)    ][(posX - 1)];    //L
        if(posX < sizeX - 1)                        num_neighbours += board[(posY)    ][(posX + 1)];    //R
        if(posY < sizeY - 1 && posX > 0)            num_neighbours += board[(posY + 1)][(posX - 1)];    //DL
        if(posY < sizeY - 1)                        num_neighbours += board[(posY + 1)][(posX)    ];    //D
        if(posY < sizeY - 1 && posX < sizeX - 1)    num_neighbours += board[(posY + 1)][(posX + 1)];    //DR
        return num_neighbours;
    }
}

int gameoflife_t::step_simulation(unsigned int num_steps){
    for(unsigned int s = 0; s < num_steps; ++s){
        auto new_board = board;
        for(size_t i = 0; i < sizeY; ++i){
            for(size_t j = 0; j < sizeX; ++j){
                int num_neighbours = count_neighbours(j, i);

                if(num_neighbours < 2 || num_neighbours > 3){
                    //Set new board to updated value
                    new_board[i][j] = false;
                    //If in the old board the cell was alive, decrease the live cells counter
                    if(board[i][j] == true)
                        --live_cells;
                }
                if(num_neighbours == 3){
                    //Set new board to updated value
                    new_board[i][j] = true;
                    //If in the old board the cell was dead, increase the live cells counter
                    if(board[i][j] == false)
                        ++live_cells;
                }
            }
        }
        board = new_board;
    }

    return 0;
}

void gameoflife_t::print_board(ostream &os){
    //For every row
    for(size_t i = 0; i < sizeY; ++i){
        //Print upped border if I still have to print the first row
        if(i == 0){
            os << "+";
            for(size_t tmp = 0; tmp < sizeX; ++tmp) os << "--";
            os << "+";
            os << endl;
        }

        //Print left border before I start to print all the cells in the current row
        os << "|";
        //Print all the cells in the current row
        for(size_t j = 0; j < sizeX; ++j){
            if(board[i][j]) os << "[]";
            else            os << "  ";
        }

        //Print right border and newline
        os << "|" << endl;

        //If I've just finished printing the last line, print the lower border
        if(i == sizeY - 1){
            os << "+";
            for(size_t tmp = 0; tmp < sizeX; ++tmp) os << "--";
            os << "+";
            os << endl;
        }
    }
}

gameoflife_t::~gameoflife_t() {
    //dtor
}
