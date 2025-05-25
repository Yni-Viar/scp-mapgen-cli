#include "thirdparty/astar.hpp"
#include <vector>

class MapGenCore{
protected:

//# Works only if there are large endrooms, to prevent endless loop if cannot spawn
	const int NUMBER_OF_TRIES_TO_SPAWN = 4;

//# For performance reasons. Correct the code to increase the limit
	const int MAX_ROOMS_SPAWN = 512;


//# Rooms that will be used
	//MapGenZone[] rooms;

//# Zone size (values before 8 NOT recommended, may lead to unexpected behavior)
	int zone_size = 8;

//# Amount of zones by X coordinate
	int map_size_x = 0;

//# Amount of zones by Y coordinate
	int map_size_y = 0;

//# Room in grid size
	double grid_size = 64;

//# Large rooms support
	bool large_rooms = false;

//# How much the map will be filled with rooms
	double room_amount = 0.75;

// Sets the door generation. Not recommended to disable, if your map uses SCP:SL 14.0-like door frames!
//@export var enable_door_generation: bool = true
//# Better zone generation.
//# Sometimes, the generation will return "dull" path(e.g where there are only 3 ways to go)
//# This fixes these generations, at a little cost of generation time
	bool better_zone_generation = true;

//# How many additional rooms should spawn map generator
//# /!\ WARNING! Higher value may hang the game.
	int better_zone_generation_min_amount = 4;

//# Enable checkpoint rooms.
//# /!\ WARNING! The checkpoint room behaves differently, than SCP-CB checkpoints,
//# the "checkpoint" have 2 rooms, not one (as in SCP-CB).
	bool checkpoints_enabled = false;

//# Prints map seed
	bool debug_print = true;

	int size_x;
	int size_y;

	int endroom_amount = 0;

public:

	enum RoomTypes { EMPTY, ROOM1, ROOM2, ROOM2C, ROOM3, ROOM4 };

	struct Room {
		// north, east, west and south check the connection between rooms.
		bool exist;
		bool north;
		bool south;
		bool east;
		bool west;
		RoomTypes room_type;
		double angle;
		bool large;
		//Ref<MapGenRoom> resource;
		std::string room_name;
		bool checkpoint;
	};

	struct Vec2 {
		float x;
		float y;

		Vec2 operator - (Vec2 other)
		{
			Vec2 temp;
			temp.x -= other.x;
			temp.y -= other.y;
			return temp;
		}

		bool operator == (Vec2 other)
		{
			return (x == other.x && y == other.y);
		}
	};

	struct Vec2i {
		int x;
		int y;

		Vec2i operator - (Vec2i other)
		{
			Vec2i temp;
			temp.x -= other.x;
			temp.y -= other.y;
			return temp;
		}

		bool operator == (Vec2i other)
		{
			return (x == other.x && y == other.y);
		}
	};

	unsigned int rng_seed = 0;

	std::vector<std::vector<Room>> mapgen;

	//# Cells, where a room will never spawn due to large room overriden
	std::vector<Vec2i> disabled_points;

//# Prepares room generation
	void start_generation();

//# Main function, that generate the zones. Rewritten in 7.0
	void prepare_generation();
//# type: 0 - room1, 1 - room2, 2 - room2C, 3 - room3
	void generate_zone_astar();

//# Main walker function, using AStarGrid2D
	bool check_room_dimensions(int x, int y, int type);

//# Places information about rooms
	void walk_astar(Vec2 from, Vec2 to);
	void place_room_positions();
	void clear();
};