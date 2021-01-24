#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include <array>
#include <gameoflife_t.h>

#define MAX_HISTORY_SIZE 20

using namespace std;

class player
{
    private:
        //Another board, for the starting condition, so that the player will be able to manipulate a smaller area than the entire game board if required
        vector<vector<bool>> starting_board;
        //Starting board sizes
        size_t starting_board_sizeX;
        size_t starting_board_sizeY;
        //Game board history, to detect periodicity
        vector<vector<vector<bool>>> board_history;
        //Costs and rewards
        double cost_per_starting_cell = 20;
        double reward_per_step_completed = 1;
        double reward_per_alive_cell_every_step = 0;

    public:
        //---------------------------------------------------------------------------------------------------------------------------------------------------------
        //Class containing the game of life game
        gameoflife_t game;
        //Cumulative score achieved in the past simulations. Cleared when called the function init simulation
        double score;

        //---------------------------------------------------------------------------------------------------------------------------------------------------------
        //Constructor
        player(size_t _game_board_sizeX = 100, size_t _game_board_sizeY = 100, size_t _starting_board_sizeX = 8, size_t _starting_board_sizeY = 8, bool _wrap_edges = false);
        //
        int set_starting_board_cell(size_t posX = 0, size_t posY = 0, bool state = 0);
        void random_fill_starting_board(float percentage = 30.0f);
        void clear_game_board() {game.clear_board();}
        void clear_starting_board() {starting_board = vector<vector<bool>>(starting_board_sizeY, vector<bool>(starting_board_sizeX, false));}
        const decltype(starting_board)& get_starting_board() {return starting_board;}
        void print_starting_board(ostream &os = cout);
        //
        size_t get_starting_sizeX() {return starting_board_sizeX;}
        size_t get_starting_sizeY() {return starting_board_sizeY;}
        //
        int init_simulation(size_t offset_x, size_t offset_y, double _cost_per_starting_cell = 20, double _reward_per_step_completed = 1, double _reward_per_alive_cell_every_step = 0);
        int step_simulation(size_t num_steps = 1);
        size_t step_simulation_until_periodic();
        size_t step_simulation_until_wall_is_hit_or_periodic();
        int detect_periodicity(const vector<vector<vector<bool>>>& boards);
        //
        int mutate(const decltype(starting_board)& ref_starting_board,  float probability_cell_change_state,    float probability_cell_relocate,
                                                                        const float probability_big_mutations,  const float probability_complete_mutation, const float random_fill_percentage = 30);
        //
        virtual ~player();
};

#endif // PLAYER_H
