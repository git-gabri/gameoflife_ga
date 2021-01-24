#include "player.h"

#include <stdexcept>
#include <ctime>
#include <vector>
#include <array>
#include <string>

using namespace std;

//Constructor
player::player(size_t _game_board_sizeX, size_t _game_board_sizeY, size_t _starting_board_sizeX, size_t _starting_board_sizeY, bool _wrap_edges) {
    game = gameoflife_t(_game_board_sizeX, _game_board_sizeY, _wrap_edges);

    if(_starting_board_sizeX < 1 || _starting_board_sizeY < 1)
        throw runtime_error("size of the starting board too small");
    if(_starting_board_sizeX > _game_board_sizeX || _starting_board_sizeY > _game_board_sizeY)
        throw runtime_error("size of the starting board too big");

    starting_board = vector<vector<bool>>(_starting_board_sizeY, vector<bool>(_starting_board_sizeX, false));
    starting_board_sizeX = _starting_board_sizeX;
    starting_board_sizeY = _starting_board_sizeY;
}

//Set single cells in the starting board, i.e. the board that the player can manipulate (it can be smaller than the main board)
int player::set_starting_board_cell(size_t posX, size_t posY, bool state){
    if(posX >= starting_board_sizeX || posY >= starting_board_sizeY)
        throw runtime_error("setting non-existent starting board cell at (" + to_string(posX) + ", " + to_string(posY) + ")");

    starting_board[posY][posX] = state;
    return 0;
}

//Randomly fill starting board
void player::random_fill_starting_board(float percentage){
    for(size_t i = 0; i < starting_board_sizeY; ++i){
        for(size_t j = 0; j < starting_board_sizeX; ++j){
            if(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/100)) <= percentage){
                set_starting_board_cell(j, i, 1);
            }
        }
    }
}

//Print starting board to ostream os
void player::print_starting_board(ostream &os){
    //For every row
    for(size_t i = 0; i < starting_board_sizeY; ++i){
        //Print upped border if I still have to print the first row
        if(i == 0){
            os << "+";
            for(size_t tmp = 0; tmp < starting_board_sizeX; ++tmp) os << "--";
            os << "+";
            os << endl;
        }

        //Print left border before I start to print all the cells in the current row
        os << "|";
        //Print all the cells in the current row
        for(size_t j = 0; j < starting_board_sizeX; ++j){
            if(starting_board[i][j]) os << "[]";
            else                     os << "  ";
        }

        //Print right border and newline
        os << "|" << endl;

        //If I've just finished printing the last line, print the lower border
        if(i == starting_board_sizeY - 1){
            os << "+";
            for(size_t tmp = 0; tmp < starting_board_sizeX; ++tmp) os << "--";
            os << "+";
            os << endl;
        }
    }
}

//Initialize everything required for the "simulation", i.e. setting the initial conditions of Game of Life, resetting the score etc...
int player::init_simulation(size_t x_offset, size_t y_offset, double _cost_per_starting_cell, double _reward_per_step_completed, double _reward_per_alive_cell_every_step){
    //Check if offsets make sense
    if(x_offset + starting_board_sizeX > game.get_sizeX() || y_offset + starting_board_sizeY > game.get_sizeY())
        throw runtime_error("starting board offsets are too high");

    //Reset variables
    score = 0;
    cost_per_starting_cell = _cost_per_starting_cell;
    reward_per_step_completed = _reward_per_step_completed;
    reward_per_alive_cell_every_step = _reward_per_alive_cell_every_step;

    //Reset board to starting conditions
    game.clear_board();
    for(size_t i = 0; i < starting_board_sizeY; ++i){
        for(size_t j = 0; j < starting_board_sizeX; ++j){
            game.set_cell(j + x_offset, i + y_offset, starting_board[i][j]);
        }
    }

    //Clear board history
    board_history.clear();

    //Set initial score to -1 * live_cells * cost_per_starting_cell
    score -= game.get_live_cells_count() * cost_per_starting_cell;

    return 0;
}

//Function to single-step the simulation of the Game of Life
int player::step_simulation(size_t num_steps){
    game.step_simulation(num_steps);

    score += reward_per_step_completed;
    score += game.get_live_cells_count() * reward_per_alive_cell_every_step;

    board_history.push_back(game.get_board());

    return 0;
}

//Function to step the simulation of the Game of Life until periodicity is detected
size_t player::step_simulation_until_periodic(){
    size_t steps_counter = 0;

    while(!detect_periodicity(board_history)){
        step_simulation();

        if(board_history.size() > MAX_HISTORY_SIZE)
            board_history.erase(board_history.begin());

        ++steps_counter;
    }

    return steps_counter;
}

//Step simulation until some cell hits a wall
size_t player::step_simulation_until_wall_is_hit_or_periodic(){
    size_t steps_counter = 0;

    //while(lambda function that checks for cells around the borders return true)
    while([&]{
        bool wall_hit = false;
        const auto gameboard = game.get_board();

        //Check top and bottom borders
        for(size_t j = 0; j < game.get_sizeX() && !wall_hit; ++j){
            if(gameboard[0][j])                     wall_hit = true;
            if(gameboard[game.get_sizeY() - 1][j])  wall_hit = true;
        }
        //Check left and right borders
        for(size_t i = 0; i < game.get_sizeY() && !wall_hit; ++i){
            if(gameboard[i][0])                     wall_hit = true;
            if(gameboard[i][game.get_sizeX() - 1])  wall_hit = true;
        }

        return !wall_hit;
    }() && !detect_periodicity(board_history)){
        //step simulation
        step_simulation();

        //check if the history size is too big
        if(board_history.size() > MAX_HISTORY_SIZE)
            board_history.erase(board_history.begin());

        //increase step counter
        ++steps_counter;
    }

    return steps_counter;
}

//Function to detect periodicity based on the board_history of the Game of Life
int player::detect_periodicity(const vector<vector<vector<bool>>>& boards){
    //Check if history size is too small. If so, just return no periodicity detected
    if(boards.size() < 2)
        return 0;

    //Check last board against all the others
    auto last_board = boards.back();

    //Start from the second to last one and go until we reach board 0
    for(size_t i = boards.size() - 2; i+1 != 0; --i){
        bool identical_flag = true;

        for(size_t j = 0; j < boards[i].size() && identical_flag; ++j){
            for(size_t k = 0; k < boards[i][j].size() && identical_flag; ++k){
                if(last_board[j][k] != boards[i][j][k]){
                    identical_flag = false;
                }
            }
        }

        if(identical_flag)
            return 1;
    }

    return 0;
}

//Function to mutate the current player' starting board, based on another player' starting board
int player::mutate( const decltype(starting_board)& ref_starting_board,
                    float probability_cell_change_state,   float probability_cell_relocate,
                    const float probability_big_mutations, const float probability_complete_mutation, const float random_fill_percentage){

    //Check for big or complete mutations
    //complete mutation
    if(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/100)) <= probability_complete_mutation){
        random_fill_starting_board(random_fill_percentage);
        return 2;
    }
    //big mutations
    bool big_mutations = false;
    if(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/100)) <= probability_big_mutations){
        probability_cell_change_state *= 5;
        probability_cell_relocate *= 5;

        big_mutations = true;
    }

    //Flag used to keep track if some mutations happened. Since I don't like clones, and they're not very useful,
    //this variable is used so that if no mutations happen, we simply try again
    bool mutations_happened = false;

    do{
        //Clear starting board and make a copy of it
        clear_starting_board();

        //Iterate on each cell of the starting board and check if by chance the cell will change state or it will have to be relocated (i.e. move alive cell into dead cell)
        for(size_t i = 0; i < starting_board_sizeY; ++i){
            for(size_t j = 0; j < starting_board_sizeX; ++j){
                //Check if cell will change state
                if(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/100)) <= probability_cell_change_state){
                    starting_board[i][j] = !ref_starting_board[i][j];
                    mutations_happened = true;
                } else

                //Check if cell is alive and it will be relocated
                if(ref_starting_board[i][j] == true && static_cast<float>(rand()) / (static_cast<float>(RAND_MAX/100)) <= probability_cell_relocate){
                    size_t new_random_posX = 0;
                    size_t new_random_posY = 0;
                    int attempts = 0;
                    do{
                        new_random_posX = rand() % starting_board_sizeX;
                        new_random_posY = rand() % starting_board_sizeY;
                        ++attempts;
                    }while(ref_starting_board[new_random_posY][new_random_posX] == true && attempts < 20);

                    starting_board[new_random_posY][new_random_posX] = true;
                    mutations_happened = true;
                } else {
                    starting_board[i][j] = ref_starting_board[i][j];
                }
            }
        }
    }while(!mutations_happened);

    if(big_mutations)
        return 1;
    else
        return 0;
}

player::~player() {
    //dtor
}
