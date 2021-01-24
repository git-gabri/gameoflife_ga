# Conway's Game of Life and genetic algorithms
This program uses genetic algorithms to evolve starting structures in the [Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life), so that they are:
* as simple as possible
* they step through as many iterations as possible without becoming periodic

To do this, a `vector` of `player`s is created.  Each `player` can manipulate all the cells in a starting area, which by default is 8x8 in size, and all of the simulations are done in a 40x40 board.  
At the start of the simulation, this 8x8 starting area is copied in the middle of the 40x40 grid, and the simulation steps through time until periodicity in the board is detected.  
The players learn to take advantage of all the starting area at their disposal to produce a starting structure that gets them the highest score (more on how this is calculated later).  
The program stops once a certain amount of generations of players have been simulated.

**Note:** the size of the grid isn't automatically adjusted to simulate cells that tend to escape from the origin of the grid in structures such as gliders.

## Building
`gcc` compiler is required. Run `build.sh`. You should now have an executable called `gameoflife_ga`.

## How does it work
This section might be worth reading to better understand what's being referred to in `Usage`.  

At the start of the program, a `vector` of `player`s is created, and all of their starting areas get randomly filled with dead and live cells.  
Then the program goes into the following loop while the number of simulated generations is lower than a specified value:
* simulate every `player` and assign to it a score, which means:
	1. clear its simulation board.
	2. place its starting area in the middle of the board.
	3. subtract from the score `number_of_live_cells * cost_per_starting_cell`.
	4. check periodicity of the board, if periodic break from current loop
	5. single step simulation
	6. player's score gets incremented by `reward_per_step_completed`.
	7. player's score gets incremented by `number_of_live_cells * reward_per_alive_cell_per_step`.
	8. go to step iv.
* print some info to the user, along the starting board of the best 10% of players.
* mutate the worst 90% of players based on the best 10%, which means:
	1. select one of the best players from the 10% as a reference.
	2. there's a small chance for the player that's being mutated to have its starting board completely randomly redefined. If this happen, stop the mutations for this player.
	3. there's a small chance for the player that's being mutated to have big mutations happening.
	4. clear the starting board of the mutated player.
	5. copy the starting board of the reference player cell by cell, but for each copied cell, there's a small chance for it to change state or a small chance for it to be copied in a different location.
	6. if we made a clone of the reference player, go to step iv.
* increment the generation counter.

## Usage
To start the program, `cd` to the folder where the executable is placed and simply launch it with `./gameoflife_ga` in any terminal of your choice.  

The program comes with a variety of different options specified in a list of `#define` at the start of `main.cpp`, so every time something is changed, for these changes to have effect the program has to be recompiled (I didn't feel like implementing command line arguments).  
Here's a list of them and what they represent (some of them are pretty self explanatory):
* **POPULATION_SIZE**: amount of players that get simulated every generation.
* **GEN_TO_SIM**: number of generations to simulate until the program stops.
* **FALLBACK_NUM_THREADS**. This program is multi threaded by default. It attempts to automatically detect the number of threads available on the host machine and uses all of them to simulate the players. If this detection fails, the number of threads utilized for the simulations will be the number specified after this `define`.
* **GAMEBOARD_SIZEX**: x size of the simulated board.
* **GAMEBOARD_SIZEY**: y size of the simulated board.
* **STARTINGBOARD_SIZEX**: x size of the starting board that the player can manipulate.
* **STARTINGBOARD_SIZEY**: y size of the starting board that the player can manipulate.
* **WRAP_EDGES**. If this is set to true, the game board will be mapped on the surface of a torus. This means for example that a glider exiting on the right side of the board will reappear on the left side, and one exiting downwards will reappear on the upper side. This flag **shouldn't be touched**, because setting it to true might make the program might fail to detect periodicity. For example, if a glider spawns and doesn't crash into any structures in the game board, it will probably travel forever, and make its simulation run forever.
* **RANDOMFILL_PERCENTAGE**: percentage at which the starting boards of players of generation 0 get filled at.
* **COST_PER_STARTING_CELL**. This number gets subtracted from the score of the player at the start of the simulation. The more starting live cells it choses to use, the more it gets penalized.
* **REWARD_PER_STEP_COMPLETED**. This number gets added to the score of the player after every step of the simulation gets completed successfully, i.e. no periodicity detected.
* **REWARD_PER_ALIVE_CELL_PER_STEP**. This number, times the number of live cells in the current board state, gets added to the score of the player at every step.
* **PROB_CELL_CHANGE_STATE**: percentage representing the probability for a cell of a player, while it's being mutated, to chance state.
* **PROB_CELL_RELOCATE**: percentage representing the probability for a cell of a player, while it's being mutated, to move in a different position.
* **PROB_BIG_MUTATIONS**: percentage representing the probability for a player to have big mutations happening, i.e. `prob_cell_chance_state` and `prob_cell_relocate` are multiplied by 5.
* **PROB_COMPLETE_MUTATION**: percentage representing the probability for a player to have its starting board completely randomly redefined.

The program by default simulates each `player` until some kind of periodicity is detected, but by changing the line in `main.cpp` that says `return p.step_simulation_until_periodic();` to `return p.step_simulation_until_wall_is_hit_or_periodic();` the program will stop the simulation if periodicity is detected or if a live cells is spawned next to a wall.

The class `gameoflife_t` is completely standalone and can be used outside of this program too.  
The thread pool implementation is taken from [here](https://github.com/progschj/ThreadPool).