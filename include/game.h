#if !defined NIKMAN_GAME_H
#define NIKMAN_GAME_H

#include <vector>
#include <string>

#include "entity.h"
#include "level.h"
#include "utility.h"


struct Game {

    std::vector<std::string> level_filenames;
    int current_level = 0;

    Map map;
    Crust crust;
    Player player;
    Wall wall;
    RandomGuy random_guy;

    Game(const LevelDesc& level) : 
        map(level.h, level.w, level),
        wall(level.h, level.w, level),
        crust(level.h, level.w, map.grid),
        player(level.h, level.w, map.grid),
        random_guy(level.h, level.w, map.grid)
    {
        level_filenames = {
            "C:/Users/stefa/OneDrive/Desktop/level.txt",
            "C:/Users/stefa/OneDrive/Desktop/level2.txt",
            "C:/Users/stefa/OneDrive/Desktop/level3.txt",
        };

        LoadLevel(level_filenames[current_level].c_str());
    }

    void Update(float delta, unsigned wasd, bool& stop_game) {

        unsigned int eaten;
        player.Update(delta, wasd, eaten);
        bool hit_player = false;
        random_guy.Update(delta, player.precise_x, player.precise_y, hit_player);

        if (hit_player && !player.just_hit) {
            player.just_hit = true;
            player.time_after_hit = 0;
            player.lives--;
            std::cout << "Vite restanti: " << player.lives << '\n';
        }

        if (player.lives <= 0) {
            std::cout << "Game over!\n";
            stop_game = true;
        }

        map.remaining_crusts -= eaten;
        if (map.remaining_crusts <= 0) {
            // Fine livello!
            current_level = (current_level + 1) % level_filenames.size();
            LoadLevel(level_filenames[current_level].c_str());
        }

        // TODO: Rimuovere quesoto metodo di debug
        if (wasd >= 2 + 4 + 8) {
            current_level = (current_level + 1) % level_filenames.size();
            LoadLevel(level_filenames[current_level].c_str());
        }
    }

    void Render() {

        map.Render();
        wall.Render();
        crust.Render();
        player.Render();
        random_guy.Render();
    }

    void LoadLevel(const char* filename) {

        LevelDesc level = ReadLevelDesc(filename);

        map.LoadLevel(level);
        wall.LoadLevel(level);
        player.LoadLevel(level);
        crust.LoadLevel(level);
        random_guy.LoadLevel(level);

    }

};




#endif // NIKMAN_GAME_H