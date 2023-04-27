/* 

$Id

Copyright (C) 2009 Alexios Chouchoulas

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  

*/


#ifndef __ASTAR_ASTAR_H
#define __ASTAR_ASTAR_H


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include "astar_heap.h"


// The maximum number of directions
#define NUM_DIRS 8


// Directions are returned as an array of these values using the various DIR_x
// values above.
typedef BYTE direction_t;


/*
 * The A* data structure itself.
 *
 * 0------------------------------------------------------------+
 * | GAME MAP                                                   |
 * |                                                            |
 * | (origin_x, origin_y)                                       |
 * |       \<- - - - - -  w - - - - - ->                        |
 * |        0--------------------------+ ^                      |
 * |        | A* GRID                  | |                      |
 * |        |                          |                        |
 * |        |    o....                 | |                      |
 * |        |         ...              |                        |
 * |        |            .             | h                      |
 * |        |             .            |                        |
 * |        |              ......o     | |                      |
 * |        |                          | v                      |
 * |        +--------------------------+                        |
 * |                                                            |
 * +------------------------------------------------------------+
 *
 * The A* search works on a subplane of the entire map, sized w x h. The origin
 * of this subplane is stored within the structure for easy reference.
 * 
 * All other co-ordinates in the structure are translated so that they are in
 * the co-ordinate system of the A* grid, not the game map. This means that,
 * e.g., column range [0, w) |--> [origin_x, origin_x + w) and row range
 * [0, h) |--> [origin_y, origin_y + h).
 *
 * Note that this is an oversimplification. Since game maps can be
 * topologically torroidal (if wrap-around is used), the actual mapping may
 * vary (e.g. may be modulo the size of the map). This algorithm won't handle
 * this case internally. The caller should have massaged numbers
 * appropriately.
 *
 */

typedef struct {

	///////////////////////////////////////////////////////////////////////////////
	//
	// Parameters.
	//
	///////////////////////////////////////////////////////////////////////////////
	
	uint32    origin_x;   // X ordinate of the top-left corner.
	uint32    origin_y;   // Y ordinate of the top-left corner.

	uint32    x0, y0;     // Starting location.
	uint32    x1, y1;     // Destination location.

	uint32    w;          // Width (pitch) of the grid.
	uint32    h;		// Height of the grid.


	///////////////////////////////////////////////////////////////////////////////
	//
	// Configuration and costs.
	//
	///////////////////////////////////////////////////////////////////////////////

	// Stop calculating when the route incurs this much cost.

	uint32    max_cost;


	// Maximum search time in microseconds (1000000us=1s).

	uint32    timeout;

	// Arrays of 8 elements holding delta-x and delta-y pairs for the eight
	// directions.

	int dx[NUM_DIRS];
	int dy[NUM_DIRS];

	// An array of 8 elements holding the costs of moving in each of the
	// directions.

	int mc[NUM_DIRS];

	// The steering penalty is added to the cost of any move that includes a change in
	// direction.
	
	int steering_penalty;

	// The cost of a 'move' as returned by the heuristic. If slightly smaller than the
	// cost of moving along a cardinal dimension (e.g. mc[0]), the calculated route is
	// smoother.

	int heuristic_factor;

	///////////////////////////////////////////////////////////////////////////////
	//
	// User functions
	//
	///////////////////////////////////////////////////////////////////////////////

	// The heuristic function. Given the source (x0,y0) and destination (x1,y1),
	// calculate the  heuristic distance. Leave it as-is  (or set it to  NULL) and the
	// built-in Manhattan distance will be used. The result of the heuristic will be
	// multiplied by the heuristic_factor above.
	
	uint32  (* heuristic) (const uint32 x0, const uint32 y0,
				 const uint32 x1, const uint32 y1);

	// The map initialisation function. Given co-ordinates (x,y) on the game map
	// (i.e. adjusted for the map origin (origin_x,origin_y) such that the X range is
	// [origin_x, origin_x + w) and Y range is [origin_y, origin_y + h), this function
	// must return a cost in the range [0,255] to denote how easy it is to reach this
	// map square. A cost of 255 is impassable. Any lower cost is treated as a
	// relative difficulty. The cost is added to the cost of moving along the chosen
	// direction, as described in array mc in this structure. The search grid may be
	// initialised either by calling astar_grid_init(), passing a get() function like
	// this, or by setting get() and allowing the algorithm to only request the map
	// squares it needs. The latter is preferable for large maps.

	BYTE (*get) (const uint32 x, const uint32 y);

	///////////////////////////////////////////////////////////////////////////////
	//
	// Data needed to run the algorithm
	//
	///////////////////////////////////////////////////////////////////////////////

	uint32    ofs0;       // Starting location (as grid offset)
	uint32    ofs1;       // Destination (as grid offset)
	uint32    bestscore;  // Best H so far.
	asheap_t *  heap;	// The binary heap holds F |-> square_t mappings.
	square_t *  grid;	// The grid holds the actual square_t structs.

	// Bitfield holding search state.

	uint32  origin_set:1; // The origin has been set.
	uint32  must_reset:1; // A search has ran, must reset.
	uint32  grid_init:1;  // The grid has been initialised.
	uint32  grid_clean:1; // The grid is ready for use.
	uint32  have_route:1; // A (partial) route has been found.
	uint32  have_best:1;  // There's a compromise route.
    uint32  move_8way:1;   // Move along all 8 directions.
	
	uint32 t0;      // Algorithm start time.

	///////////////////////////////////////////////////////////////////////////////
	//
	// Results
	//
	///////////////////////////////////////////////////////////////////////////////

	uint32    steps;	// Number of moves in the route.
	uint32    score;	// Score of the route.
	uint32    result;	// Result code of the routing.
	char *      str_result; // Stringified result code.
	uint32    usecs;      // Search time in microseconds.
	uint32    loops;      // Number of search loops.
	uint32    gets;       // Number of times get() was called.
	uint32    updates;    // Keeps track of heap updates (they're expensive).
	uint32    open;       // Number of open positions.
	uint32    closed;     // Number of closed positions.

	uint32    bestofs;    // If a route wasn't found, the best offset we could reach.
	uint32    bestx;      // Likewise, the X ordinate of the best ending point.
	uint32    besty;      // Likewise, the X ordinate of the best ending point.
} astar_t;


// This denotes the cost of impassable blocks.
#define COST_BLOCKED 255


// A* Result Codes (as returned by astar_run and stored in astar_t.result).
#define ASTAR_FOUND                 0
#define ASTAR_NOTHING               1 // Nothing yet.
#define ASTAR_NOTFOUND              2 // Didn't reach destination (or reached the score limit)
#define ASTAR_TRIVIAL               3 // Nothing to do, already at destination.
#define ASTAR_TIMEOUT               4 // Reached the time limit.
#define ASTAR_GRID_NOT_INITIALISED  5 // The grid hasn't been initialised (and get() is unset)
#define ASTAR_GRID_NOT_INITIALIZED  ASTAR_GRID_NOT_INITIALISED // Merkin alias.
#define ASTAR_ORIGIN_NOT_SET        6 // astar_t.get() called, but the origin wasn't set.
#define ASTAR_EMBEDDED              7 // The origin is embedded in a blocked square, can't move.
#define ASTAR_AMONTILLADO           ASTAR_EMBEDDED // E. A. Poe alias.


// We use three bits to specify the direction of a square's 'parent'.
#define DIR_N  0
#define DIR_E  1
#define DIR_S  2
#define DIR_W  3
#define DIR_NE 4
#define DIR_SE 5
#define DIR_SW 6
#define DIR_NW 7

// Movement modes.
#define DIR_CARDINAL  0
#define DIR_8WAY      1


// This is only used in directions_t to signify the end of the directions (for
// added safety).
#define DIR_END 255


astar_t *
astar_new (const uint32 w, const uint32 h,
	   BYTE (*get) (const uint32, const uint32),
	   uint32  (*heuristic) (const uint32, const uint32,
				   const uint32, const uint32));
	   
void	astar_destroy (astar_t * as);

void astar_init_grid (astar_t * as,
		      uint32 origin_x, uint32 origin_y,
		      BYTE(*get)(const uint32, const uint32));

void astar_set_movement_mode (astar_t * as, int movement_mode);

uint32 astar_get_directions (astar_t *as, direction_t ** directions);

void astar_free_directions (direction_t * directions);

void astar_set_origin (astar_t * as, const uint32 x, const uint32 y);

void astar_set_max_cost (astar_t *as, const uint32 max_cost);

void astar_set_timeout (astar_t *as, const uint32 timeout);

void astar_set_dxy (astar_t *as, const BYTE dir, const int dx, const int dy);

void astar_set_cost (astar_t *as, const BYTE dir, const uint32 cost);

void astar_set_steering_penalty (astar_t *as, const uint32 steering_penalty);

void astar_set_heuristic_factor (astar_t *as, const uint32 heuristic_factor);

int astar_run (astar_t * as,
	       const uint32 x0, const uint32 y0,
	       const uint32 x1, const uint32 y1);

// Return the last A* result code.
#define astar_result(as) (as)->result

// Return the last A* result code (string version).
#define astar_str_result(as) (as)->str_result

// Return non-zero if the A* algorithm has a route. This is only a
// full route if ASTAR_FOUND is the result code.
#define astar_have_route(as) (as)->have_route

// Convert directions to dx, dy.
#define astar_get_dx(as,dir) ((as)->dx[dir])
#define astar_get_dy(as,dir) ((as)->dy[dir])


#ifdef ASTAR_DEBUG
void astar_print (astar_t * as);
#endif // ASTAR_DEBUG


// int astar_run (astar_t * as);



#ifdef __cplusplus
};
#endif // __cplusplus


#endif // _ASTAR_ASTAR_H

// End of file.
