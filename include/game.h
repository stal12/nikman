#if !defined NIKMAN_GAME_H
#define NIKMAN_GAME_H

#include <vector>
#include <string>

#include "entity.h"
#include "level.h"
#include "utility.h"
#include "ui.h"

enum class GameState { MainMenu, Game, End };

struct Game {

    std::vector<std::string> level_filenames;
    int current_level = 0;

    Map map;
    Crust crust;
    Player player;
    Wall wall;
    RandomGuy random_guy;
    UI ui;
    GameState state;
    unsigned int prev_wasd = 0;
    int main_menu_selected = 0;

    Game(const LevelDesc& level) :
        state(GameState::MainMenu),
        map(level.h, level.w, level),
        wall(level.h, level.w, level),
        crust(level.h, level.w, map.grid),
        player(level.h, level.w, map.grid),
        random_guy(level.h, level.w, map.grid)
    {
        level_filenames = {
            "level.txt",
            "level2.txt",
            "level3.txt",
        };

        LoadLevel(level_filenames[current_level].c_str());

        // TODO togliere queste prove

        //Panel panel(50, 0);
        //panel.AddWriting("YAML!@/\\VA", 0, ui.font.h_space + 100, ui.font, false);
        //panel.AddWriting("YAML!@/\\VA", 0, 0, ui.font);
        //ui.AddPanel("prova", std::move(panel));


        ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = true;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    }

    // wasd is a bitmapped value containing the keys pressed
    // 0  1  2  3  4   5     6     7      8      9    
    // W  A  S  D  Up  Left  Down  Right  Enter  Esc  
    void Update(float delta, unsigned wasd, bool& stop_game) {

        if (state == GameState::Game) {

            if (wasd & 512) {
                state = GameState::MainMenu;
                ui.panel_map.at("game_ui").second = false;
                ui.panel_map.at("main_menu").second = true;
                main_menu_selected = 0;
                ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = true;
            }

            unsigned int eaten;
            player.Update(delta, wasd, eaten);
            bool hit_player = false;
            random_guy.Update(delta, player.precise_x, player.precise_y, hit_player);

            if (hit_player && !player.just_hit) {
                player.just_hit = true;
                player.time_after_hit = 0;
                player.lives--;
                std::cout << "Vite restanti: " << player.lives << '\n';
                char str[] = "Lives: x";
                str[7] = '0' + player.lives;
                ui.panel_map.at("game_ui").first.writings[0].Update(str);
            }

            if (player.lives <= 0) {
                std::cout << "Game over!\n";
                stop_game = true;
            }

            map.remaining_crusts -= eaten;
            if (map.remaining_crusts <= 0) {
                // Fine livello!
                current_level++;
                if (current_level == level_filenames.size()) {
                    state = GameState::End;
                    ui.panel_map.at("end_game").second = true;
                    ui.panel_map.at("game_ui").second = false;
                }
                else {
                    LoadLevel(level_filenames[current_level].c_str());
                }
            }

            // TODO: Rimuovere questo metodo di debug
            if (wasd == 2 + 4 + 8 && prev_wasd != 2 + 4 + 8) {
                current_level++;
                if (current_level == level_filenames.size()) {
                    state = GameState::End;
                    ui.panel_map.at("end_game").second = true;
                    ui.panel_map.at("game_ui").second = false;
                }
                else {
                    LoadLevel(level_filenames[current_level].c_str());
                }
            }

            prev_wasd = wasd;
        }
        else if (state == GameState::MainMenu) {
            if ((wasd & 256) && !(prev_wasd & 256)) {
                if (main_menu_selected == 0) {
                    // New game
                    state = GameState::Game;
                    player.lives = 3;
                    current_level = 0;
                    LoadLevel(level_filenames[current_level].c_str());
                    ui.panel_map.at("game_ui").second = true;
                    ui.panel_map.at("main_menu").second = false;
                    return;
                }
                else if (main_menu_selected == 1) {
                    // Quit
                    stop_game = true;
                    return;
                }
            }
            else if ((wasd & 16) && !(prev_wasd & 16)) {
                ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = false;
                main_menu_selected = (main_menu_selected + (2 - 1)) & 1;  // -1 e poi %2
                ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = true;
            }
            else if ((wasd & 64) && !(prev_wasd & 64)) {
                ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = false;
                main_menu_selected = (main_menu_selected + 1) & 1;  // +1 e poi %2
                ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = true;
            }
            prev_wasd = wasd;
        }
        else if (state == GameState::End) {
            if ((wasd & 256) && !(prev_wasd & 256)) {
                state = GameState::MainMenu;
                ui.panel_map.at("main_menu").second = true;
                ui.panel_map.at("end_game").second = false;
            }
            prev_wasd = wasd;
        }
    }

    void Render() {

        if (state == GameState::Game) {
            map.Render();
            wall.Render();
            crust.Render();
            player.Render();
            random_guy.Render();
        }

        glEnable(GL_BLEND);
        ui.Render();
        glDisable(GL_BLEND);
    }

    void LoadLevel(const char* filename) {

        LevelDesc level = ReadLevelDesc((std::filesystem::path(kLevelRoot) / std::filesystem::path(filename)).string().c_str());

        map.LoadLevel(level);
        wall.LoadLevel(level);
        player.LoadLevel(level);
        crust.LoadLevel(level);
        random_guy.LoadLevel(level);

    }

};




#endif // NIKMAN_GAME_H