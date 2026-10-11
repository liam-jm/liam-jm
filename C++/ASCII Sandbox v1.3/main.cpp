// 22/05/2026: Sandbox (by Liam660)
/* update logs:
   v1.0: 
      -rendering added
      -movement added
      -collisions added
   v1.1:
      -block placing/breaking added
   v1.2:
      -world saving/loading/resetting
   v1.3:
      -main menu screen (ui/ux)
      -death screen (ui/ux) (works, but death can't occur)
      -game states
      -inventory (scrapped)
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <conio.h>
#include <windows.h>
#include <cmath>
#include <fstream>
#include <filesystem>

const int WORLD_HEIGHT = 16;
const int WORLD_WIDTH = 65;

// structs, enums and functions

enum game_state {
   MENU,
   GAME,
   INVENTORY,
   DEATH,
   NONE
};
enum entity_type {
   player,
   pig,
   cow,
   chicken,
   zombie
};
enum direction {
   up,
   right,
   left,
   down
};
enum object_type {
   air,
   wall,
   non_collidable_object,
   tree,
   player_object,
   entity_object
};
struct coordinates {
   int x;
   int y;
};
struct object {
   object_type type;
   coordinates position;
   std::string name;
   bool collidable;
   float toughness;
   float time_to_break;
};
struct item {
   std::string name;
   bool stackable;
   int max_stack_size;
};
struct slot {
   int index; // starts at 0, not 1
   item item;
};
struct inventory {
   slot selected_slot;
   slot slot_1;
   slot slot_2;
   slot slot_3;
   slot slot_4;
   slot slot_5;
};
struct entity {
   entity_type type;
   bool natural_regen;
   int hp;
   int max_hp;
   float xp;
   int speed;
   char reference;
   char up_reference;
   char right_reference;
   char left_reference;
   char down_reference;
   std::string nametag;
   direction rotation;
   inventory inventory;
   coordinates position;
};

struct status_effect {
   // work in progress
};

void save_world(char world[WORLD_HEIGHT][WORLD_WIDTH], int world_index, entity& main_player) { // int world_index must be 1, 2 or 3
   std::ofstream save("saves/world_" + std::to_string(world_index) + ".txt");

   save << main_player.position.x << "\n";
   save << main_player.position.y << "\n";
   save << main_player.hp << "\n";

   std::string frame;

   for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int x = 0; x < (WORLD_WIDTH); x++) {
               frame += world[y][x];
            }
            frame += "\n";
         }
   
   save << frame;
   save.close();
}
void load_world(char world[WORLD_HEIGHT][WORLD_WIDTH], int world_index, entity& main_player) {
    std::ifstream save("saves/world_" + std::to_string(world_index) + ".txt");

    std::string pos_x, pos_y, hp;

    if (!std::getline(save, pos_x)) return;
    if (!std::getline(save, pos_y)) return;
    if (!std::getline(save, hp)) return;

    main_player.position.x = std::stoi(pos_x);
    main_player.position.y = std::stoi(pos_y);
    main_player.hp = std::stoi(hp);

    for (int y = 0; y < WORLD_HEIGHT; y++) {
        std::string line;

        if (!std::getline(save, line))
            line = std::string(WORLD_WIDTH, ' ');

        for (int x = 0; x < WORLD_WIDTH; x++) {
            world[y][x] = (x < line.size()) ? line[x] : ' ';
        }
    }
    save.close();
}
void reset_world(int world_index) { // int world_index must be 1, 2 or 3
   std::filesystem::copy_file("saves/world_0.txt", "saves/world_" + std::to_string(world_index) + ".txt", std::filesystem::copy_options::overwrite_existing);
}

bool can_collide(coordinates coords, char world[WORLD_HEIGHT][WORLD_WIDTH]) {
   return world[coords.y][coords.x] != ' ';
}
void move_to(entity& entity, coordinates old_coords, coordinates new_coords, char world[WORLD_HEIGHT][WORLD_WIDTH]) {
   world[old_coords.y][old_coords.x] = ' ';
   entity.position = new_coords;
   world[new_coords.y][new_coords.x] = entity.reference;

   std::this_thread::sleep_for(std::chrono::milliseconds(48));
}

void render(char world[WORLD_HEIGHT][WORLD_WIDTH], entity& main_player, std::string ui_to_render, std::string inventory_ui[], inventory inv, std::string death_ui[], game_state current_game_state, std::string menu_ui[]) {
      std::string frame;
      
      frame += "\033[2J\033[1;1H";

      if (ui_to_render == "world") {
      for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int x = 0; x < (WORLD_WIDTH); x++) {
               if (world[y][x]=='^' || world[y][x]=='>' || world[y][x]=='<' || world[y][x]=='v') {
                  frame += "\033[034m";
                  frame += world[y][x];
               }
               /*else if (world[y+2][x]=='^' || world[y+2][x]=='>' || world[y+2][x]=='<' || world[y+2][x]=='v') {
                  frame.erase(frame.size()-5);
                  frame += main_player.nametag;
               }*/
               else if (world[y][x]=='#' && x==0 || x==WORLD_WIDTH-3 || y==0 || y==WORLD_HEIGHT-1) {
                  frame += "\033[032m";
                  frame += world[y][x];
               }
               else {
                  frame += world[y][x];
               }
               frame += "\033[0m";
            }
            frame += "\n";
         }
      }
      else if (ui_to_render == "inventory") {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            frame += inventory_ui[y];
            frame += "\n";
            // \e[1;36m
         }
      }
      else if (ui_to_render == "death") {
         for (int y = 0; y < WORLD_HEIGHT; y++) {
            frame += death_ui[y];
            frame += "\n";
         }
      }
      else if (ui_to_render == "menu") {
         for (int y = 0; y < WORLD_HEIGHT; y++) {
            frame += menu_ui[y];
            frame += "\n";
         }
      }
      
      std::cout << frame;

      std::cout << "\033[31mHealth: " << main_player.hp << "/" << main_player.max_hp << "\033[0m " << "Controls: Z=Up, Q=Left, D=Right, S=Down\n";
      std::cout << "\033[36mCoordinates: x=" << main_player.position.x <<
                   ", y=" << WORLD_HEIGHT-main_player.position.y << "\033[0m E=Inventory, R=Attack/Break, F=Place\n";
      
      
      std::string current_game_state_str;

      switch (current_game_state) {
         case MENU:
            current_game_state_str = "MENU";
            break;
         case GAME:
            current_game_state_str = "GAME";
            break;
         case INVENTORY:
            current_game_state_str = "INVENTORY";
            break;
         case DEATH:
            current_game_state_str = "DEATH";
            break;
         case NONE:
            current_game_state_str = "NONE";
            break;
         default:
            current_game_state_str = "UNKNOWN";
            break;
      }
      std::cout << "Current game state: " << current_game_state_str << "\n";
   }

void change_entity_rotation(entity& entity, direction rotation) {
   if (rotation==up) {
      entity.rotation = rotation;
      entity.reference = entity.up_reference;
   }
   else if (rotation==right) {
      entity.rotation = rotation;
      entity.reference = entity.right_reference;
   }
   else if (rotation==left) {
      entity.rotation = rotation;
      entity.reference = entity.left_reference;
   }
   else if (rotation==down) {
      entity.rotation = rotation;
      entity.reference = entity.down_reference;
   }
}

void place_block(entity& entity, char world[WORLD_HEIGHT][WORLD_WIDTH], char block) {
   if (entity.rotation==up && world[entity.position.y-1][entity.position.x]==' ') {
      world[entity.position.y-1][entity.position.x] = block;
   }
   else if (entity.rotation==right && world[entity.position.y][entity.position.x+1]==' ') {
      world[entity.position.y][entity.position.x+1] = block;
   }
   else if (entity.rotation==left && world[entity.position.y][entity.position.x-1]==' ') {
      world[entity.position.y][entity.position.x-1] = block;
   }
   else if (entity.rotation==down && world[entity.position.y+1][entity.position.x]==' ') {
      world[entity.position.y+1][entity.position.x] = block;
   }
}

char break_block(entity& entity, char world[WORLD_HEIGHT][WORLD_WIDTH]) {
   
   int block_x = entity.position.x;
   int block_y = entity.position.y;

   if (entity.rotation==up) {
      block_y--;
   }
   else if (entity.rotation==right) {
      block_x++;
   }
   else if (entity.rotation==left) {
      block_x--;
   }
   else if (entity.rotation==down) {
      block_y++;
   }

   if (world[block_y][block_x] == ' ') {
      return ' ';
   }

   if (block_x==0 || block_x==WORLD_WIDTH-3 || block_y==0 || block_y==WORLD_HEIGHT-1) {
      return ' ';
   }

   char block = world[block_y][block_x];
   world[block_y][block_x] = ' ';

   return block;
}

// main function

int main() {
   SetConsoleOutputCP(CP_UTF8); // to allow special characters

   game_state current_game_state = MENU;

   entity main_player;
   main_player.type = player;
   main_player.max_hp = 5;
   main_player.hp = 5;
   main_player.position.x = 32;
   main_player.position.y = 8;
   
   main_player.nametag = "Player";
   main_player.reference = '>';

   main_player.up_reference = '^';
   main_player.right_reference = '>';
   main_player.left_reference = '<';
   main_player.down_reference = 'v';

   main_player.rotation = right;

   main_player.natural_regen = true;
   
   inventory inv = main_player.inventory;
   slot selected_slot = inv.selected_slot;
   slot slot_1 = inv.slot_1;
   slot slot_2 = inv.slot_2;
   slot slot_3 = inv.slot_3;
   slot slot_4 = inv.slot_4;
   slot slot_5 = inv.slot_5;

   inv.selected_slot = slot_1;
   slot_1.index = 0;
   slot_2.index = 1;
   slot_3.index = 2;
   slot_4.index = 3;
   slot_5.index = 4;

   char world[WORLD_HEIGHT][WORLD_WIDTH] = { // each block of the world is a character
      "###############################################################",
      "#                                                             #",
      "#                                                             #",
      "#    T                                                        #",
      "#                                              T              #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                 T           #",
      "#                                                             #",
      "#            T                                                #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "###############################################################",
   };
   std::string inventory_ui[WORLD_HEIGHT] = {
      "###############################################################",
      "#                                                             #",
      "#    ██ ▄▄  ▄▄ ▄▄ ▄▄ ▄▄▄▄▄ ▄▄  ▄▄ ▄▄▄▄▄▄ ▄▄▄  ▄▄▄▄  ▄▄ ▄▄     #",
      "#    ██ ███▄██ ██▄██ ██▄▄  ███▄██   ██  ██▀██ ██▄█▄ ▀███▀     #",
      "#    ██ ██ ▀██  ▀█▀  ██▄▄▄ ██ ▀██   ██  ▀███▀ ██ ██   █       #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#    \033[36m1▄▄▄▄▄▄▄   2▄▄▄▄▄▄▄   3▄▄▄▄▄▄▄   4▄▄▄▄▄▄▄   5▄▄▄▄▄▄▄\033[0m     #",
      "#     \033[36m█     █    █     █    █     █    █     █    █     █\033[0m     #",
      "#     \033[36m█     █    █     █    █     █    █     █    █     █\033[0m     #",
      "#     \033[36m▀▀▀▀▀▀▀    ▀▀▀▀▀▀▀    ▀▀▀▀▀▀▀    ▀▀▀▀▀▀▀    ▀▀▀▀▀▀▀\033[0m     #",
      "#                                                             #",
      "#                                                             #",
      "###############################################################",
   };
   std::string death_ui[WORLD_HEIGHT] = {
      "###############################################################",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#     \033[31m██  ██ ▄████▄ ██  ██   ████▄  ██ ██████ ████▄   ██\033[0m      #",
      "#      \033[31m▀██▀  ██  ██ ██  ██   ██  ██ ██ ██▄▄   ██  ██  ██\033[0m      #",
      "#       \033[31m██   ▀████▀ ▀████▀   ████▀  ██ ██▄▄▄▄ ████▀   ▄▄\033[0m      #",
      "#                                                             #",
      "#                                                             #",
      "#                  Press \033[36m[Spacebar]\033[0m to respawn                #",
      "#                ________________________________             #",
      "#                                                             #",
      "#               Press \033[35m[Esc]\033[0m to return to main menu            #",
      "#                                                             #",
      "#                                                             #",
      "###############################################################",
   };
   std::string menu_ui[WORLD_HEIGHT] = {
      "###############################################################",
      "#                                                             #",
      "#     \033[93m▄█████ ▄████▄ ███  ██ ████▄  █████▄ ▄████▄ ██  ██\033[0m       #",
      "#     \033[93m▀▀▀▄▄▄ ██▄▄██ ██ ▀▄██ ██  ██ ██▄▄██ ██  ██  ████ \033[0m       #",
      "#     \033[93m█████▀ ██  ██ ██   ██ ████▀  ██▄▄█▀ ▀████▀ ██  ██\033[0m       #",
      "#                                                             #",
      "#                                                      \033[4;96mv1.3\033[0m   #",
      "#     \033[36m[Spacebar]\033[0m to load world, \033[35m[Esc]\033[0m 5x to reset world,      #",
      "#                     \033[33m[←/→]\033[0m to select world                   #",
      "#                                                             #",
      "#                     \033[1;31m                      \033[0m                  #", // insert str "world_x has been reset" at char index 29 and erase the 22 following chars
      "#           \033[1;36mV\033[0m                  \033[1;36m \033[0m                  \033[1;36m \033[0m           #", // 1->12, 2->32, 3->51
      "#                                                             #", // 19, 49, 79
      "#     |  world_1  |      |  world_2  |      |  world_3  |     #", // \033[1;36m"insert world here"\033[0m
      "#                                                             #", // -3 offset for \033[1;36m and +4 offset for \033[0m
      "###############################################################", // 1a->9, 1b->16; 2a->29, 2b->36; 3a->48, 3b->55;
   };
   char console[WORLD_HEIGHT][WORLD_WIDTH] = {
      "###############################################################",
      "#                                                             #",
      "#  Developer console:                                         #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#  > give sword 1                                             #",
      "#                                                             #",
      "###############################################################",
   };

   /*
      "###############################################################",
      "#                                                             #",
      "#    ██ ▄▄  ▄▄ ▄▄ ▄▄ ▄▄▄▄▄ ▄▄  ▄▄ ▄▄▄▄▄▄ ▄▄▄  ▄▄▄▄  ▄▄ ▄▄     #",
      "#    ██ ███▄██ ██▄██ ██▄▄  ███▄██   ██  ██▀██ ██▄█▄ ▀███▀     #",
      "#    ██ ██ ▀██  ▀█▀  ██▄▄▄ ██ ▀██   ██  ▀███▀ ██ ██   █       #",
      "#                                                             #",
      "#                                                             #",
      "#                                                             #",
      "#      Apple     Pickaxe     Sword   Cobblestone    Axe       #",
      "#    1▄▄▄▄▄▄▄   2▄▄▄▄▄▄▄   3▄▄▄▄▄▄▄   4▄▄▄▄▄▄▄   5▄▄▄▄▄▄▄     #",
      "#     █  .  █    █  .  █    █  .  █    █  .  █    █  .  █     #",
      "#     █   x4█    █   x1█    █   x1█    █  x24█    █   x1█     #",
      "#     ▀▀▀▀▀▀▀    ▀▀▀▀▀▀▀    ▀▀▀▀▀▀▀    ▀▀▀▀▀▀▀    ▀▀▀▀▀▀▀     #",
      "#                                                             #",
      "#                                                             #",
      "###############################################################",
   */
   
   bool running = true;
   bool movement_enabled = true;
   bool inventory_opened = false;
   bool inventory_enabled = true;
   std::string ui_to_render;

   int selected_world = 1;
   int esc_counter = 0;

   ui_to_render = "menu";
   render(world, main_player, ui_to_render, inventory_ui, inv, death_ui, current_game_state, menu_ui);

   while (running) {
      int key = _getch();
      if (key==27) {
         esc_counter+=1;
      }
      else {
         esc_counter=0;
      }
      
      if (current_game_state==MENU) {
         ui_to_render = "menu";
         if (movement_enabled==true) {
            movement_enabled = false;
         }
         if (inventory_enabled==true) {
            inventory_enabled = false;
         }
         if (esc_counter>=5 && selected_world) { // delete selected world
            esc_counter = 0;
            reset_world(selected_world);
            // insert str "world_x has been reset" at char index 29 and erase the 22 following chars
            menu_ui[10].replace(29, 22, "world_" + std::to_string(selected_world) + " has been reset");
            render(world, main_player, ui_to_render, inventory_ui, inv, death_ui, current_game_state, menu_ui);
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            menu_ui[10].replace(29, 22, 22, ' ');
         }
         if (key==0 || key==224) { // arrow key detection
            key = _getch(); // important: second _getch(); returns arrow keycode
            
            // keycodes: 72=up, 80=down, 77=right, 75=left

            if (key==77 && selected_world<3) {
               selected_world++;
            }
            else if (key==75 && selected_world>1) {
               selected_world--;
            } // 19, 49, 79
            switch (selected_world) {
               case 1:
                  menu_ui[11].replace(19, 1, "V");
                  menu_ui[11].replace(49, 1, " ");
                  menu_ui[11].replace(79, 1, " ");
                  break;
               case 2:
                  menu_ui[11].replace(19, 1, " ");
                  menu_ui[11].replace(49, 1, "V");
                  menu_ui[11].replace(79, 1, " ");
                  break;
               case 3:
                  menu_ui[11].replace(19, 1, " ");
                  menu_ui[11].replace(49, 1, " ");
                  menu_ui[11].replace(79, 1, "V");
                  break;
               default:
                  menu_ui[11].replace(19, 1, "V");
                  menu_ui[11].replace(49, 1, " ");
                  menu_ui[11].replace(79, 1, " ");
                  break;
            }
         }
         else if (key==32) { // spacebar detection
            load_world(world, selected_world, main_player);
            ui_to_render = "world";
            if (movement_enabled==false) {
               movement_enabled = true;
            }
            if (inventory_enabled==false) {
               inventory_enabled = true;
            }
            current_game_state = GAME;
         }
      }
      
      if (main_player.hp==0) { // if the player dies, show death ui, then main menu ui
         current_game_state = DEATH;
         ui_to_render = "death";
         movement_enabled = false;
         inventory_enabled = false;
         if (key==32) { // 32=spacebar
            main_player.hp = main_player.max_hp;
            current_game_state = GAME;
            coordinates old_coords;
            old_coords.x = main_player.position.x;
            old_coords.y = main_player.position.y;
            coordinates new_coords;
            new_coords.x = 32;
            new_coords.y = 8;
            move_to(main_player, old_coords, new_coords, world);
            ui_to_render = "world";
            movement_enabled = true;
            inventory_enabled = true;
         }
         else if (key==27) { // 27=esc
            current_game_state = MENU;
         }
      }
      
      if (key=='e' && inventory_enabled) { // inventory
         inventory_opened = !inventory_opened;
         ui_to_render = "inventory";
         if (inventory_opened) {
            current_game_state = INVENTORY;
            movement_enabled = false;
         }
         else {
            current_game_state = GAME;
            ui_to_render = "world";
            movement_enabled = true;
         }
      }
      if (inventory_opened==true && current_game_state==INVENTORY) {
         // for inv ui, use "V" above box for selection
         if (key=='&' || key=='1') {
               selected_slot = slot_1;
            }
            else if (key==130 || key=='2') { // 130=é
               selected_slot = slot_2;
            }
            else if (key=='"' || key=='3') {
               selected_slot = slot_3;
            }
            else if (key=='\'' || key=='4') {
               selected_slot = slot_4;
            }
            else if (key=='(' || key=='5') {
               selected_slot = slot_5;
            }
      }
      else if (key=='f' && current_game_state==GAME) { // place block
         place_block(main_player, world, '#');
      }
      else if (key=='r' && current_game_state==GAME) { // break block / attack
         break_block(main_player, world);
      }
      
      if (movement_enabled) {
         current_game_state = GAME;
      if (key=='z') { // up
         change_entity_rotation(main_player, up);
         coordinates old_coords;
         old_coords.x = main_player.position.x;
         old_coords.y = main_player.position.y;
         coordinates new_coords;
         new_coords.x = main_player.position.x;
         new_coords.y = main_player.position.y-1;
         if (can_collide(new_coords, world)==false) {
            move_to(main_player, old_coords, new_coords, world);
         }
         ui_to_render = "world";
      }
      else if (key=='q') { // left
         change_entity_rotation(main_player, left);
         coordinates old_coords;
         old_coords.x = main_player.position.x;
         old_coords.y = main_player.position.y;
         coordinates new_coords;
         new_coords.x = main_player.position.x-1;
         new_coords.y = main_player.position.y;
         if (can_collide(new_coords, world)==false) {
            move_to(main_player, old_coords, new_coords, world);
         }
         ui_to_render = "world";
      }
      else if (key=='d') { // right
         change_entity_rotation(main_player, right);
         coordinates old_coords;
         old_coords.x = main_player.position.x;
         old_coords.y = main_player.position.y;
         coordinates new_coords;
         new_coords.x = main_player.position.x+1;
         new_coords.y = main_player.position.y;
         if (can_collide(new_coords, world)==false) {
            move_to(main_player, old_coords, new_coords, world);
         }
         ui_to_render = "world";
      }
      else if (key=='s') { // down
         change_entity_rotation(main_player, down);
         coordinates old_coords;
         old_coords.x = main_player.position.x;
         old_coords.y = main_player.position.y;
         coordinates new_coords;
         new_coords.x = main_player.position.x;
         new_coords.y = main_player.position.y+1;
         if (can_collide(new_coords, world)==false) {
            move_to(main_player, old_coords, new_coords, world);
         }
         ui_to_render = "world";
      }
      }
      render(world, main_player, ui_to_render, inventory_ui, inv, death_ui, current_game_state, menu_ui);
      if (current_game_state==GAME) {
         save_world(world, selected_world, main_player);
      }  
   }
   
   std::cin.get();

   return 0;
}
