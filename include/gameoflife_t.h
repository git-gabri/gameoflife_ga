#ifndef GAMEOFLIFE_T_H
#define GAMEOFLIFE_T_H

#include <iostream>
#include <vector>

using namespace std;

class gameoflife_t
{
    private:
        vector<vector<bool>> board;
        size_t sizeX;
        size_t sizeY;
        bool wrap_edges;
        size_t live_cells;

    public:
        gameoflife_t(size_t _sizeX = 100, size_t _sizeY = 100, bool _wrap_edges = false);
        //
        const decltype(board)& get_board() {return board;}
        size_t get_sizeX() {return sizeX;}
        size_t get_sizeY() {return sizeY;}
        size_t get_live_cells_count() {return live_cells;}
        //
        int set_cell(size_t posX = 0, size_t posY = 0, bool state = 0);
        int set_board(const vector<vector<bool>>& ref_board);
        void random_fill(float percentage = 30.0f);
        void clear_board() {board = vector<vector<bool>>(sizeY, vector<bool>(sizeX, false)); live_cells = 0;}
        //
        int count_neighbours(size_t posX, size_t posY);
        //
        int step_simulation(unsigned int num_steps = 1);
        void print_board(ostream &os = cout);
        //
        virtual ~gameoflife_t();
};

#endif // GAMEOFLIFE_T_H
