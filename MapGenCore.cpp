#include "MapGenCore.hpp"
#include "thirdparty/astar.hpp"
#include <iostream>
#include <vector>
#include <cmath>

int main()
{
	unsigned int seed;
	std::cout << "Input seed for map generator..." << std::endl;
	std::cin >> seed;
	MapGenCore core = MapGenCore();
	core.rng_seed = seed;
	core.start_generation();
	return 0;
}

void MapGenCore::start_generation()
{
	clear();
	srand(rng_seed);
	prepare_generation();

	// Determines, if the generation is too large to stop it.
	// You can change the limit in MAX_ROOM_SPAWN const.
	int all_rooms_count = size_x * size_y * room_amount;
	if(all_rooms_count > MAX_ROOMS_SPAWN)
	{
		std::cout << "The limit of " << MAX_ROOMS_SPAWN << " rooms for all zones reached. Stopping the program..." << std::endl;
		std::cout << "If you want to increase the limit, set MAX_ROOM_SPAWN constant to higher value, althrough it is not recommended." << std::endl;
	}
	else
	{
		generate_zone_astar();
		place_room_positions();
	}
}

void MapGenCore::prepare_generation()
{
	if(debug_print)
	{
		std::cout << "Preparing generation..." << std::endl;
	}
	size_x = zone_size * (map_size_x + 1);
	size_y = zone_size * (map_size_y + 1);

	// Fill mapgen with zeros
	for(int g=0; g<size_x; g+=1)
	{
		mapgen.push_back({});
		for(int h=0; h<size_y; h+=1)
		{
			mapgen[g].push_back(Room());
			mapgen[g][h].exist = false;
			mapgen[g][h].north = false;
			mapgen[g][h].south = false;
			mapgen[g][h].east = false;
			mapgen[g][h].west = false;
			mapgen[g][h].room_type = RoomTypes::EMPTY;
			mapgen[g][h].angle =  - 1;
			mapgen[g][h].large = false;
			mapgen[g][h].checkpoint = false;
		}
	}
}

void MapGenCore::generate_zone_astar()
{
	if(debug_print)
	{
		std::cout << "Generating the map..." << std::endl;
	}

	// Zone counter. Used for determining a center of the map.
	MapGenCore::Vec2i zone_counter = MapGenCore::Vec2i(0, 0);

	// Zone index. Used for iterating zone resources.
	int zone_index = 0;

	// Zone index for Y coordinate.
	int zone_index_default = 0;

	// Zone center for the first quadrant.
	double zone_center = zone_size / 2;
	for(int i=0; i<map_size_x + 1; i+=1)
	{
		zone_counter.x = i;
		for(int j=0; j<map_size_y + 1; j+=1)
		{

			// Large room amount (when checkpoints enabled, there are fewer rooms)
			int large_room_amount = ( !checkpoints_enabled ? zone_size / 6 : (zone_size - 2) / 6 );
			zone_counter.y = j;
			zone_index += j;
			int number_of_rooms = zone_size * room_amount;

			// to deal with zero-sized zone_counter, there is a simple formula - if is not odd - 
			// add value to be not null
			int tmp_x = 0;
			int tmp_y = 0;

			MapGenCore::Vec2 current_zone_center = MapGenCore::Vec2(zone_center + (zone_size * zone_counter.x), zone_center + (zone_size * zone_counter.y));
			mapgen[std::round(current_zone_center.x)][std::round(current_zone_center.y)].exist = true;
			if(number_of_rooms > (zone_size - 1) * 4 - 4 - large_room_amount * 6)
			{
				std::cout << "Too many rooms, map won't spawn" << std::endl;
				return ;
			}
			else if(number_of_rooms < 1)
			{
				std::cout << "Too few rooms, map won't spawn" << std::endl;
				return ;
			}

			// Available room position (for AStar walk)
			std::vector<MapGenCore::Vec2> available_room_position = 
				std::vector { MapGenCore::Vec2(size_x / (map_size_x + 1) * zone_counter.x, size_x / (map_size_x + 1) * (zone_counter.x + 1) - 1), 
				MapGenCore::Vec2(size_y / (map_size_y + 1) * zone_counter.y, size_y / (map_size_y + 1) * (zone_counter.y + 1) - 1),  };

			// Random room position. If large rooms enabled, also used for large room coordinates
			MapGenCore::Vec2 random_room;
			/*
			//# Reworked large rooms module
			if(large_rooms && rooms[zone_index]->endrooms_single_large.size() > 0)
			{
				for(int k=0; k<large_room_amount; k+=1)
				{
					for(int l=0; l<NUMBER_OF_TRIES_TO_SPAWN; l+=1)
					{
						random_room = MapGenCore::Vec2(rand() % (available_room_position[0].y - available_room_position[0].x + 1) + available_room_position[0].x, 
							rand() % (available_room_position[1].y - available_room_position[1].x + 1) + available_room_position[1].x);
						if(check_room_dimensions(random_room.x, random_room.y, 0))
						{
							walk_astar(MapGenCore::Vec2(std::round(current_zone_center.x), std::round(current_zone_center.y)), random_room);
							mapgen[random_room.x][random_room.y].large = true;
							break;
						}
					}
				}
			}
			*/
			//# Walk before need-to-spawn rooms runs out
			while(number_of_rooms > 0)
			{
				if(checkpoints_enabled)
				{

					//# If checkpoints enabled, disable non-checkpoint rooms in generic hallway generation
					random_room = MapGenCore::Vec2(rand() % (int)(((available_room_position[0].y - 1) - (available_room_position[0].x + 1) + 1) + (available_room_position[0].x + 1)),
						rand() % (int)(((available_room_position[1].y - 1) - (available_room_position[1].x + 1) + 1) + (available_room_position[1].x + 1)));
				}
				else
				{
					//# Do as it was in 7.x, except reverted old better zone generation
					random_room = MapGenCore::Vec2(rand() % (int)((available_room_position[0].y - available_room_position[0].x + 1) + available_room_position[0].x),
						rand() % (int)((available_room_position[1].y - available_room_position[1].x + 1) + available_room_position[1].x));
				}
				if (better_zone_generation && mapgen[random_room.x][random_room.y].exist && endroom_amount < better_zone_generation_min_amount)
				{
					continue;
				}
				walk_astar(MapGenCore::Vec2(std::round(current_zone_center.x), std::round(current_zone_center.y)), random_room);
				number_of_rooms -= 1;
			}

			//# Connect two zones
			if(zone_counter.x < map_size_x)
			{
				int zone_center_x = std::round(zone_center + (zone_size * (zone_counter.x + 1)));
				walk_astar(MapGenCore::Vec2(std::round(current_zone_center.x), std::round(current_zone_center.y)), MapGenCore::Vec2(zone_center_x, std::round(current_zone_center.y)));
			}
			if(zone_counter.y < map_size_y)
			{
				int zone_center_y = std::round(zone_center + (zone_size * (zone_counter.y + 1)));
				walk_astar(MapGenCore::Vec2(std::round(current_zone_center.x), std::round(current_zone_center.y)), MapGenCore::Vec2(std::round(current_zone_center.x), zone_center_y));
			}
		}
		zone_index_default += map_size_y;
		zone_counter.y = 0;
	}
}
//# Checks spawn places for large rooms in given coordinates
bool MapGenCore::check_room_dimensions(int x, int y, int type)
{
	switch(type)
	{
		case 0:
			//# ROOM1 - endroom
			if(x == 0 && y == 0)
			{
				if(!mapgen[x + 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else if(!mapgen[x][y + 1].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					return true;
				}
				else
				{
					return false;
				}
			}
			else if(x == size_x - 1 && y == size_y - 1)
			{
				if(!mapgen[x - 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					return true;
				}
				else if(!mapgen[x][y - 1].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else
				{
					return false;
				}
			}
			else if(x == 0 && y == size_y - 1)
			{
				if(!mapgen[x + 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else if(!mapgen[x][y - 1].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else
				{
					return false;
				}
			}
			else if(x == size_x - 1 && y == 0)
			{
				if(!mapgen[x - 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					return true;
				}
				else if(!mapgen[x][y + 1].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					return true;
				}
				else
				{
					return false;
				}
			}
			//# |[x][x]  |        |[x]
			//# |[o][x]  |[o][x]  |[o]
			//# |        |[x][x]  |[x]
			else if(x == 0)
			{
				if(!mapgen[x][y + 1].exist && !mapgen[x + 1][y + 1].exist && !mapgen[x + 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else if (!mapgen[x][y - 1].exist && !mapgen[x + 1][y - 1].exist && !mapgen[x + 1][y].exist)\
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else if (!mapgen[x][y + 1].exist && !mapgen[x][y - 1].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else
				{
					return false;
				}
			}
			//# [x][x]|        |  [x]|
			//# [x][o]|  [x][o]|  [o]|
			//#       |  [x][x]|  [x]|
			else if (x == size_x - 1)
			{
				if (!mapgen[x][y + 1].exist && !mapgen[x - 1][y + 1].exist && !mapgen[x - 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					return true;
				}
				else if (!mapgen[x][y - 1].exist && !mapgen[x - 1][y - 1].exist && !mapgen[x - 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					return true;

				}
				else if (!mapgen[x][y + 1].exist && !mapgen[x][y - 1].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else
				{
					return false;
				}
			}
			
			//# [x][x]   [x][x]   
			//# [o][x]   [x][o]   [x][o][x]
			//# ------   ------   ---------
			else if (y == 0)
			{
				if (!mapgen[x][y + 1].exist && !mapgen[x + 1][y + 1].exist && !mapgen[x + 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else if(!mapgen[x - 1][y].exist && !mapgen[x - 1][y + 1].exist && !mapgen[x][y + 1].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					return true;
				}
				else if(!mapgen[x + 1][y].exist && !mapgen[x - 1][y].exist)
				{

					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else
				{
						
					return false;
				}
			}
			
			//# ------   ------   ---------  
			//# [o][x]   [x][o]   [x][o][x]
			//# [x][x]   [x][x]
			else if (y == size_y - 1)
			{
				if (!mapgen[x + 1][y].exist && !mapgen[x + 1][y - 1].exist && !mapgen[x][y - 1].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else if (!mapgen[x - 1][y].exist && !mapgen[x - 1][y - 1].exist && !mapgen[x][y - 1].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else if (!mapgen[x + 1][y].exist && !mapgen[x - 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else;
				{
					return false;
				}
			}
			//# [x][x]   [x][x]   [x][x][x]
			//# [x][o]   [o][x]   [x][o][x]   [x][o][x]
			//# [x][x]   [x][x]               [x][x][x]
			else
			{
				if (!mapgen[x][y + 1].exist && !mapgen[x - 1][y + 1].exist && !mapgen[x - 1][y - 1].exist && !mapgen[x][y - 1].exist && !mapgen[x - 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					return true;
				}
				else if (!mapgen[x][y + 1].exist && !mapgen[x + 1][y + 1].exist && !mapgen[x + 1][y - 1].exist && !mapgen[x][y - 1].exist && !mapgen[x + 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else if (!mapgen[x - 1][y].exist && !mapgen[x - 1][y + 1].exist && !mapgen[x][y + 1].exist && !mapgen[x + 1][y + 1].exist && !mapgen[x + 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else if (!mapgen[x - 1][y].exist && !mapgen[x - 1][y - 1].exist && !mapgen[x][y - 1].exist && !mapgen[x + 1][y - 1].exist && !mapgen[x + 1][y].exist)
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else
				{
					return false;
				}
			}
		case 1:
			//# ROOM2 - hallway
			//# |[o][x]  
			if(x == 0)
			{
				if (!mapgen[x + 1][y].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x + 1, y)) != disabled_points.end())// disabled_points.has(MapGenCore::Vec2i(x + 1, y)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else
				{
					return false;
				}
			}

			//# [x][o]|
			else if (x == size_x - 1)
			{
				if (!mapgen[x - 1][y].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x - 1, y)) != disabled_points.end())//!disabled_points.has(MapGenCore::Vec2i(x - 1, y)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					return true;
				}
				else
				{
					return false;
				}
			}
			//# [x]
			//# [o]
			//# ---
			else if (y == 0)
			{
				if (!mapgen[x][y + 1].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y + 1)) != disabled_points.end())// !disabled_points.has(MapGenCore::Vec2i(x, y + 1)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					return true;
				}
				else
				{
					return false;
				}
			}
			//# ---
			//# [o]
			//# [x]
			else if (y == size_y - 1)
			{
				if (!mapgen[x][y - 1].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y - 1)) != disabled_points.end())//!disabled_points.has(MapGenCore::Vec2i(x, y - 1)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else
				{
					return false;
				}
			}
			//#             [x]
			//# [x][o][x]   [o]
			//#             [x]
			else
			{
				if (!mapgen[x][y + 1].exist && !mapgen[x][y - 1].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y + 1)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y - 1)) != disabled_points.end()) //!disabled_points.has(MapGenCore::Vec2i(x, y + 1)) && !disabled_points.has(MapGenCore::Vec2i(x, y - 1)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else if (!mapgen[x + 1][y].exist && !mapgen[x - 1][y].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x + 1, y)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x - 1, y)) != disabled_points.end())//!disabled_points.has(MapGenCore::Vec2i(x + 1, y)) && !disabled_points.has(MapGenCore::Vec2i(x - 1, y)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					return true;
				}
				else
				{
					return false;
				}
			}
		case 2:
			//# ROOM2C - corner
			if ((x == 0 && y == 0) || (x == size_x - 1 && y == size_y - 1) || (x == 0 && y == size_y - 1) || (x == size_x - 1 && y == 0))
			{
				return true;
			}
			//# |[x]  [x]|
			//# |[o]  [o]|
			//# |[x]  [x]|
			else if(x == 0 || x == size_x - 1)
			{
				if(!mapgen[x][y + 1].exist && !mapgen[x][y - 1].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y + 1)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y - 1)) != disabled_points.end())//!disabled_points.has(MapGenCore::Vec2i(x, y + 1)) && !disabled_points.has(MapGenCore::Vec2i(x, y - 1)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else
				{
					return false;
				}
			}
			//# ---------
			//# [x][o][x]   [x][o][x]
			//#             ---------
			else if (y == 0 || y == size_y - 1)
			{
				if (!mapgen[x + 1][y].exist && !mapgen[x - 1][y].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x - 1, y)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x + 1, y)) != disabled_points.end())//!disabled_points.has(MapGenCore::Vec2i(x - 1, y)) && !disabled_points.has(MapGenCore::Vec2i(x + 1, y)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else
				{
					return false;
				}
			}
			//#                   [x][x]    [x][x]    
			//# [x][o]   [o][x]   [x][o]    [o][x]
			//# [x][x]   [x][x]
			else
			{
				if (!mapgen[x - 1][y].exist && !mapgen[x - 1][y - 1].exist && 
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y - 1)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x - 1, y - 1)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y - 1)) != disabled_points.end())// !mapgen[x][y - 1].exist && !disabled_points.has(MapGenCore::Vec2i(x - 1, y - 1)) && !disabled_points.has(MapGenCore::Vec2i(x, y - 1)) && !disabled_points.has(MapGenCore::Vec2i(x - 1, y)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					return true;
				}
				else if (!mapgen[x][y - 1].exist && !mapgen[x + 1][y - 1].exist && 
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x + 1, y)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x + 1, y - 1)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x + 1, y)) != disabled_points.end())// !mapgen[x + 1][y].exist && !disabled_points.has(MapGenCore::Vec2i(x + 1, y - 1)) && !disabled_points.has(MapGenCore::Vec2i(x, y - 1)) && !disabled_points.has(MapGenCore::Vec2i(x + 1, y)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else if (!mapgen[x - 1][y].exist && !mapgen[x - 1][y + 1].exist &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y + 1)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x - 1, y + 1)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x - 1, y)) != disabled_points.end())// !mapgen[x][y + 1].exist && !disabled_points.has(MapGenCore::Vec2i(x - 1, y)) && !disabled_points.has(MapGenCore::Vec2i(x - 1, y + 1)) && !disabled_points.has(MapGenCore::Vec2i(x, y + 1)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					return true;
				}
				else if (!mapgen[x][y + 1].exist && !mapgen[x + 1][y + 1].exist && !mapgen[x + 1][y].exist &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y + 1)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x + 1, y + 1)) != disabled_points.end() &&
					std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x + 1, y)) != disabled_points.end())// !disabled_points.has(MapGenCore::Vec2i(x, y + 1)) && !disabled_points.has(MapGenCore::Vec2i(x + 1, y + 1)) && !disabled_points.has(MapGenCore::Vec2i(x + 1, y)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y + 1));
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else
				{
					return false;
				}
			}
		case 3:
			//# ROOM3 - TWay
			//# |[o][x]  
			if(x == 0 || y == 0 || x == size_x - 1 || y == size_y - 1)
			{
				return true;
			}
			//#                  [x]
			//# [x][o]  [o][x]   [o]  [o]
			//#                       [x]
			else
			{
				if(!mapgen[x][y + 1].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y + 1)) != disabled_points.end())//!disabled_points.has(MapGenCore::Vec2i(x, y + 1)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y + 1));
					return true;
				}
				else if (!mapgen[x][y - 1].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x, y - 1)) != disabled_points.end())//!disabled_points.has(MapGenCore::Vec2i(x, y - 1)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x, y - 1));
					return true;
				}
				else if (!mapgen[x + 1][y].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x + 1, y)) != disabled_points.end())//!disabled_points.has(MapGenCore::Vec2i(x + 1, y)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x + 1, y));
					return true;
				}
				else if (!mapgen[x - 1][y].exist && std::find(disabled_points.begin(), disabled_points.end(), MapGenCore::Vec2i(x - 1, y)) != disabled_points.end())//!disabled_points.has(MapGenCore::Vec2i(x - 1, y)))
				{
					disabled_points.push_back(MapGenCore::Vec2i(x - 1, y));
					return true;
				}
				else
				{
					return false;
				}
			}
		default:
			//# ROOM4 and unknown types are not supported
			return false;
	}
}

void MapGenCore::walk_astar(MapGenCore::Vec2 from, MapGenCore::Vec2 to)
{
	// Initialization
	AStar::AStar astar_grid;
	astar_grid.setWorldSize({ size_x, size_y });
	astar_grid.setHeuristic(AStar::Heuristic::manhattan);
	for(MapGenCore::Vec2i obstacle : disabled_points)
	{
		astar_grid.addObstacle(AStar::Vec2i(obstacle.x, obstacle.y));
	}
	MapGenCore::Vec2i previous_map = MapGenCore::Vec2i((int)from.x, (int)from.y);

	// Walk
	for(AStar::Vec2i map : astar_grid.findPath(AStar::Vec2i((int)from.x, (int)from.y), AStar::Vec2i((int)to.x, (int)to.y)))
	{
		MapGenCore::Vec2i map_converted = MapGenCore::Vec2i(map.x, map.y);
		// Get difference between previous and now position.
		// This is necessary for determining room connections
		MapGenCore::Vec2i dir = map_converted - previous_map;
		previous_map = map_converted;
		mapgen[map.x][map.y].exist = true;

		if(dir == MapGenCore::Vec2i(1, 0))
		{
			if(mapgen[map.x - 1][map.y].exist)
			{
				mapgen[map.x - 1][map.y].east = true;
				mapgen[map.x][map.y].west = true;
			}
		}
		if(dir == MapGenCore::Vec2i( - 1, 0))
		{
			if(mapgen[map.x + 1][map.y].exist)
			{
				mapgen[map.x + 1][map.y].west = true;
				mapgen[map.x][map.y].east = true;
			}
		}
		if(dir == MapGenCore::Vec2i(0, 1))
		{
			if(mapgen[map.x][map.y - 1].exist)
			{
				mapgen[map.x][map.y - 1].north = true;
				mapgen[map.x][map.y].south = true;
			}
		}
		if(dir == MapGenCore::Vec2i(0, - 1))
		{
			if(mapgen[map.x][map.y + 1].exist)
			{
				mapgen[map.x][map.y + 1].south = true;
				mapgen[map.x][map.y].north = true;
			}
		}
	}
	endroom_amount += 1;
}

void MapGenCore::place_room_positions()
{
	if(debug_print)
	{
		std::cout << "Map generated:" << std::endl;
		for(int j=0; j<size_x; j+=1)
		{
			for(int k=0; k<size_y; k+=1)
			{
				std::cout << ((int)mapgen[j][k].exist);
			}
			std::cout << std::endl;
		}
		std::cout << "Connecting rooms..." << std::endl;
	}

	// Regular rooms amount (needed for better generation)
	std::vector<int> room1_amount = std::vector<int>();
	std::vector<int> room2_amount = std::vector<int>();
	std::vector<int> room2c_amount = std::vector<int>();
	std::vector<int> room3_amount = std::vector<int>();
	std::vector<int> room4_amount = std::vector<int>();

	// Large rooms amount
	std::vector<int> room2l_amount = std::vector<int>();
	std::vector<int> room2cl_amount = std::vector<int>();
	std::vector<int> room3l_amount = std::vector<int>();

	MapGenCore::Vec2i zone_counter = MapGenCore::Vec2i(0, 0);
	int room_index = 0;
	int room_index_default = 0;

	// Initialize values
	room1_amount.push_back(0);
	room2_amount.push_back(0);
	room2c_amount.push_back(0);
	room3_amount.push_back(0);
	room4_amount.push_back(0);

	room2l_amount.push_back(0);
	room2cl_amount.push_back(0);
	room3l_amount.push_back(0);
	for(int l=0; l<size_x; l+=1)
	{

		//append zone horizontal
		if(l >= size_x / (map_size_x + 1) * (zone_counter.x + 1))
		{
			zone_counter.x += 1;
			room1_amount.push_back(0);
			room2_amount.push_back(0);
			room2c_amount.push_back(0);
			room3_amount.push_back(0);
			room4_amount.push_back(0);

			room2l_amount.push_back(0);
			room2cl_amount.push_back(0);
			room3l_amount.push_back(0);
			room_index_default += 1;
		}
		for(int m=0; m<size_y; m+=1)
		{

			//append zone vertical
			if(m >= size_y / (map_size_y + 1) * (zone_counter.y + 1))
			{
				zone_counter.y += 1;
				room1_amount.push_back(0);
				room2_amount.push_back(0);
				room2c_amount.push_back(0);
				room3_amount.push_back(0);
				room4_amount.push_back(0);

				room2l_amount.push_back(0);
				room2cl_amount.push_back(0);
				room3l_amount.push_back(0);
				room_index += 1;
			}
			bool north;
			bool east;
			bool south;
			bool west;
			if(mapgen[l][m].exist)
			{
				west = mapgen[l][m].west;
				east = mapgen[l][m].east;
				north = mapgen[l][m].north;
				south = mapgen[l][m].south;
				if(north && south)
				{
					if(east && west)
					{

						//room4
						int room_angle[] {0, 90, 180, 270};
						mapgen[l][m].room_type = RoomTypes::ROOM4;
						mapgen[l][m].angle = room_angle[rand() % 4];
						room4_amount[room_index] += 1;
					}
					else if(east && !west)
					{
						//room3, pointing east
						mapgen[l][m].room_type = RoomTypes::ROOM3;
						mapgen[l][m].angle = 90;
						if(large_rooms)
						{
							if(check_room_dimensions(l, m, 3) && room3l_amount[room_index] < zone_size / 6)
							{
								mapgen[l][m].large = true;
								room3l_amount[room_index] += 1;
							}
						}
						room3_amount[room_index] += 1;
					}
					else if (!east && west)
					{
						//room3, pointing west
						mapgen[l][m].room_type = RoomTypes::ROOM3;
						mapgen[l][m].angle = 270;
						if(large_rooms)
						{
							if(check_room_dimensions(l, m, 3) && room3l_amount[room_index] < zone_size / 6)
							{
								mapgen[l][m].large = true;
								room3l_amount[room_index] += 1;
							}
						}
						room3_amount[room_index] += 1;
					}
					else //room2
					{
						if(m < size_y - 1 && m > 0)
						{
							//upper checkpoint room2
							if(m == size_y / (map_size_y + 1) * zone_counter.y && mapgen[l][m - 1].exist && checkpoints_enabled)
							{
								mapgen[l][m].checkpoint = true;
								mapgen[l][m].angle = 180;
							}
							//lower checkpoint room2
							else if(m == size_y / (map_size_y + 1) * (zone_counter.y + 1) - 1 && mapgen[l][m + 1].exist && checkpoints_enabled)
							{
								mapgen[l][m].checkpoint = true;
								mapgen[l][m].angle = 0;
							}
							else
							{
								//generic vertical room2
								int room_angle[] {0, 180};
								mapgen[l][m].angle = room_angle[rand() % 2];
							}
						}
						else
						{
							//generic vertical room2
							int room_angle[] { 0, 180 };
							mapgen[l][m].angle = room_angle[rand() % 2];
						}
						mapgen[l][m].room_type = RoomTypes::ROOM2;
						if(large_rooms)
						{
							if(check_room_dimensions(l, m, 1) && room2l_amount[room_index] < zone_size / 6)
							{
								mapgen[l][m].large = true;
								room2l_amount[room_index] += 1;
							}
						}
						room2_amount[room_index] += 1;
					}
				}
				else if (east && west)
				{
					if(north && !south)
					{
						//room3, pointing north
						mapgen[l][m].room_type = RoomTypes::ROOM3;
						mapgen[l][m].angle = 0;
						if(large_rooms)
						{
							if(check_room_dimensions(l, m, 3) && room3l_amount[room_index] < zone_size / 6)
							{
								mapgen[l][m].large = true;
								room3l_amount[room_index] += 1;
							}
						}
						room3_amount[room_index] += 1;
					}
					else if (!north && south)
					{
						//room3, pointing south
						mapgen[l][m].room_type = RoomTypes::ROOM3;
						mapgen[l][m].angle = 180;
						if (large_rooms)
						{
							if (check_room_dimensions(l, m, 3) && room3l_amount[room_index] < zone_size / 6)
							{
								mapgen[l][m].large = true;
								room3l_amount[room_index] += 1;
							}
							room3_amount[room_index] += 1;
						}
						else
						{
							//room2
							if (l < size_x - 1 && l > 0)
							{
								//right checkpoint room2
								if (l == size_x / (map_size_x + 1) * zone_counter.x && mapgen[l - 1][m].exist)
								{
									mapgen[l][m].checkpoint = true;
									mapgen[l][m].angle = 270;
								}

								//left checkpoint room2
								else if (l == size_x / (map_size_x + 1) * (zone_counter.x + 1) - 1 && mapgen[l + 1][m].exist)
								{
									mapgen[l][m].checkpoint = true;
									mapgen[l][m].angle = 90;
								}
								else
								{
									//generic horizontal room2
									int room_angle[]{ 90, 270 };
									mapgen[l][m].angle = room_angle[rand() % 2];
								}
							}
							else
							{
								//generic horizontal room2
								int room_angle[]{ 90, 270 };
								mapgen[l][m].angle = room_angle[rand() % 2];
							}

							mapgen[l][m].room_type = RoomTypes::ROOM2;

							if (large_rooms)
							{
								if (check_room_dimensions(l, m, 1) && room2l_amount[room_index] < zone_size / 6)
								{
									mapgen[l][m].large = true;
									room2l_amount[room_index] += 1;
								}
							}
							room2_amount[room_index] += 1;
						}
					}
					else if (north)
					{
						if (east)
						{
							//room2c, north-east
							mapgen[l][m].room_type = RoomTypes::ROOM2C;
							mapgen[l][m].angle = 0;
							if (large_rooms)
							{
								if (check_room_dimensions(l, m, 2) && room2cl_amount[room_index] < zone_size / 6)
								{
									mapgen[l][m].large = true;
									room2cl_amount[room_index] += 1;
								}
							}
							room2c_amount[room_index] += 1;
						}
						else if (west)
						{

							//room2c, north-west
							mapgen[l][m].room_type = RoomTypes::ROOM2C;
							mapgen[l][m].angle = 270;
							if (large_rooms)
							{
								if (check_room_dimensions(l, m, 2) && room2cl_amount[room_index] < zone_size / 6)
								{
									mapgen[l][m].large = true;
									room2cl_amount[room_index] += 1;
								}
							}
							room2c_amount[room_index] += 1;
						}
						else
						{
							//room1, north
							mapgen[l][m].room_type = RoomTypes::ROOM1;
							mapgen[l][m].angle = 0;
							room1_amount[room_index] += 1;
						}
					}
					else if (south)
					{
						if(east)
						{
							//room2c, south-east
							mapgen[l][m].room_type = RoomTypes::ROOM2C;
							mapgen[l][m].angle = 90;
							if(large_rooms)
							{
								if(check_room_dimensions(l, m, 2) && room2cl_amount[room_index] < zone_size / 6)
								{
									mapgen[l][m].large = true;
									room2cl_amount[room_index] += 1;
								}
							}
							room2c_amount[room_index] += 1;
						}
						else if(west)
						{
							//room2c, south-west
							mapgen[l][m].room_type = RoomTypes::ROOM2C;
							mapgen[l][m].angle = 180;
							if(large_rooms)
							{
								if(check_room_dimensions(l, m, 2) && room2cl_amount[room_index] < zone_size / 6)
								{
									mapgen[l][m].large = true;
									room2cl_amount[room_index] += 1;
								}
							}
							room2c_amount[room_index] += 1;
						}
						else
						{
							//room1, south
						mapgen[l][m].room_type = RoomTypes::ROOM1;
							mapgen[l][m].angle = 180;
							room1_amount[room_index] += 1;
						}
					}
					else if (east)
					{
						//room1, east
						mapgen[l][m].room_type = RoomTypes::ROOM1;
						mapgen[l][m].angle = 90;
						room1_amount[room_index] += 1;
					}
					else
					{
						//room1, west
						mapgen[l][m].room_type = RoomTypes::ROOM1;
						mapgen[l][m].angle = 270;
						room1_amount[room_index] += 1;
					}
					zone_counter.y = 0;
					//PANIC! <> unexpected at Token(type='TEXT', value='zone_counter', lineno=717, index=28271, end=28283)

					room_index = room_index_default;
				}//# Clears the map generation
			}
		}
	}
}

void MapGenCore::clear()
{
	if(debug_print)
	{
		std::cout << "Clearing the map..." << std::endl;
	}
	disabled_points.resize(0);
	mapgen.resize(0);
}
