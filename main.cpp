#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <thread>

#include <player.h>
#include <gameoflife_t.h>
#include <ThreadPool.h>

#define POPULATION_SIZE 1000
#define GEN_TO_SIM 500
#define FALLBACK_NUM_THREADS 4

#define GAMEBOARD_SIZEX 40
#define GAMEBOARD_SIZEY 40
#define STARTINGBOARD_SIZEX 8
#define STARTINGBOARD_SIZEY 8
#define WRAP_EDGES false        //Setting it to true might make the function detect_periodicity fail, if gliders are spawned and don't crash into any structures
                                //in the game board. This implies that the method step_simulation_until_periodic, will go on forever.
#define RANDOMFILL_PERCENTAGE 30

#define COST_PER_STARTING_CELL 3
#define REWARD_PER_STEP_COMPLETED 1
#define REWARD_PER_ALIVE_CELL_PER_STEP 0.1

#define PROB_CELL_CHANGE_STATE 5.0
#define PROB_CELL_RELOCATE 2.0
#define PROB_BIG_MUTATIONS 7.0
#define PROB_COMPLETE_MUTATION 7.0

using namespace std;

int main() {
    srand(static_cast<unsigned>(time(0)));
    const unsigned int available_threads = (thread::hardware_concurrency() == 0 ? FALLBACK_NUM_THREADS : thread::hardware_concurrency());

    vector<player> population(POPULATION_SIZE, player(GAMEBOARD_SIZEX, GAMEBOARD_SIZEY, STARTINGBOARD_SIZEX, STARTINGBOARD_SIZEY, WRAP_EDGES));
    ThreadPool simplayer_pool(available_threads);

    //Skip the worst 90% of the players
    size_t worst_best_player_index = (9 * population.size()) / 10;

    double best_players_curr_gen_avg_score = 0;
    double best_players_prev_gen_avg_score = 0;
    double all_players_curr_gen_avg_score = 0;
    double all_players_prev_gen_avg_score = 0;

    for(player &p : population)
        p.random_fill_starting_board(RANDOMFILL_PERCENTAGE);

    for(int generation = 0; generation < GEN_TO_SIM; ++generation) {
        cout << "------------------------------------------------------------------------------------------------" << endl;
        cout << "Simulationg now generation " << generation << "/" << GEN_TO_SIM << endl;
        best_players_curr_gen_avg_score = 0;
        all_players_curr_gen_avg_score = 0;

        {
            vector<future<size_t>> dummy_stepscompleted;
            for(player& p : population){
                dummy_stepscompleted.emplace_back(
                    simplayer_pool.enqueue([&](){
                        p.init_simulation((GAMEBOARD_SIZEX - STARTINGBOARD_SIZEX) / 2, (GAMEBOARD_SIZEY - STARTINGBOARD_SIZEY) / 2, COST_PER_STARTING_CELL, REWARD_PER_STEP_COMPLETED, REWARD_PER_ALIVE_CELL_PER_STEP);
                        return p.step_simulation_until_periodic();
                    })
                );
            }

            //Block current thread until all players have been simulated
            for(auto&& dummy : dummy_stepscompleted) dummy.get();
        }

        for(player& p : population) {
            //p.init_simulation((GAMEBOARD_SIZEX - STARTINGBOARD_SIZEX) / 2, (GAMEBOARD_SIZEY - STARTINGBOARD_SIZEY) / 2, COST_PER_STARTING_CELL, REWARD_PER_STEP_COMPLETED, REWARD_PER_ALIVE_CELL_PER_STEP);
            //p.step_simulation_until_periodic();
            all_players_curr_gen_avg_score += p.score;
        }
        all_players_curr_gen_avg_score /= (double)POPULATION_SIZE;

        sort(population.begin(), population.end(), [](const player & p1, const player & p2) {return p1.score < p2.score;});

        for(auto i = worst_best_player_index; i < population.size(); ++i) {
            auto& p = population[i];
            cout << "(" << i << ") Score of the following board: " << p.score << endl;
            p.print_starting_board();
            best_players_curr_gen_avg_score += p.score;
        }
        best_players_curr_gen_avg_score /= (double)(population.size() - worst_best_player_index);
        //
        cout << "Current generation average  score: " << all_players_curr_gen_avg_score  << " (prev.: " << all_players_prev_gen_avg_score  << ")" << endl;
        cout << "Current generation best avg score: " << best_players_curr_gen_avg_score << " (prev.: " << best_players_prev_gen_avg_score << ")" << endl;
        //
        //
        for(size_t i = 0; i < worst_best_player_index; ++i) {
            auto& p = population[i];
            //This formula picks the best player in a round robin way with each cycle of this for loop (offset + (i % num_best_players))
            auto& ref_p = population[worst_best_player_index + (i % (population.size() - worst_best_player_index))];

            p.mutate(ref_p.get_starting_board(), PROB_CELL_CHANGE_STATE, PROB_CELL_RELOCATE, PROB_BIG_MUTATIONS, PROB_COMPLETE_MUTATION, RANDOMFILL_PERCENTAGE);
        }
        //
        //
        all_players_prev_gen_avg_score = all_players_curr_gen_avg_score;
        best_players_prev_gen_avg_score = best_players_curr_gen_avg_score;
    }

    return 0;
}
