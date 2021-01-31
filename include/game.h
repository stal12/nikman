#if !defined NIKMAN_GAME_H
#define NIKMAN_GAME_H

#include <vector>
#include <string>
#include <random>

#include "entity.h"
#include "level.h"
#include "utility.h"
#include "ui.h"

enum class GameState { MainMenu, Game, End, Over, Pause, Transition };

struct Game {

    std::vector<std::string> level_filenames;
    int current_level = 0;

    static const std::vector<Ghost::Color> ghost_colors;

    Map map;
    Crust crust;
    Weapon weapon;
    Player player;
    Wall wall;
    Teleport teleport;
    //Ghost red;
    //Ghost yellow;
    //Ghost blue;
    //Ghost brown;
    std::vector<Ghost> ghosts;
    UI ui;
    GameState state;
    unsigned int prev_wasd = 0;
    int main_menu_selected = 0;
    int pause_menu_selected = 0;
    const float kTransitionDuration = 2.f;
    float transition_t;

    std::random_device rd;
    std::mt19937 mt;

    Game(const LevelDesc& level) :
        state(GameState::MainMenu),
        mt(rd()),
        map(level.h, level.w, level),
        wall(level.h, level.w, level),
        crust(level.h, level.w, map.grid),
        teleport(level.h, level.w, map.grid),
        weapon(level.h, level.w, map.grid),
        player(level.h, level.w, map.grid, teleport)
        //red(Ghost::Color::Red, level.h, level.w, map.grid, teleport),
        //yellow(Ghost::Color::Yellow, level.h, level.w, map.grid, teleport),
        //blue(Ghost::Color::Blue, level.h, level.w, map.grid, teleport),
        //brown(Ghost::Color::Brown, level.h, level.w, map.grid, teleport)
    {
        level_filenames = {
            //"pacman.txt",
            //"level.txt",
            "spispopd1.txt",
            "spispopd2.txt",
            //"level2.txt",
            //"level3.txt",
        };

        Ghost::shader = Shader("ghost");
        for (const auto color : ghost_colors) {
            ghosts.emplace_back(color, level.h, level.w, map.grid, teleport, mt);
        }

        LoadLevel(level_filenames[current_level].c_str());

        // TODO togliere queste prove

        //Panel panel(50, 0);
        //panel.AddWriting("YAML!@/\\VA", 0, ui.font.h_space + 100, ui.font, false);
        //panel.AddWriting("YAML!@/\\VA", 0, 0, ui.font);
        //ui.AddPanel("prova", std::move(panel));


        ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = true;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    }

    ~Game() {
        Ghost::shader.Release();
    }

    // wasd is a bitmapped value containing the keys pressed
    // 0  1  2  3  4   5     6     7      8      9    
    // W  A  S  D  Up  Left  Down  Right  Enter  Esc  
    void Update(float delta, unsigned wasd, bool& stop_game) {

        if (state == GameState::Game) {

            if ((wasd & 512) && !(prev_wasd & 512)) {
                state = GameState::Pause;
                ui.panel_map.at("pause").second = true;
                pause_menu_selected = 0;
                ui.panel_map.at("pause").first.writings[pause_menu_selected].highlighted = true;
                ui.panel_map.at("pause").first.writings[(pause_menu_selected + 1) % 2].highlighted = false;
                prev_wasd = wasd;
                return;
            }

            unsigned int eaten;
            bool grab_weapon = false;
            player.Update(delta, wasd, eaten, grab_weapon);

            bool hit_player = false;
            for (auto& ghost : ghosts) {
                bool hit_ghost = false;
                ghost.Update(delta, player.precise_x, player.precise_y, player.direction, ghosts[0].precise_x, ghosts[0].precise_y, hit_ghost);
                if (hit_ghost) {
                    if (weapon.player_armed) {
                        ghost.Killed();
                    }
                    else {
                        hit_player = true;
                    }
                }
            }

            if (hit_player && !player.just_hit) {
                player.just_hit = true;
                player.time_after_hit = 0;
                player.lives--;
                char str[] = "Lives: x";
                str[7] = '0' + player.lives;
                ui.panel_map.at("game_ui").first.writings[0].Update(str);

                if (player.lives <= 0) {
                    state = GameState::Over;
                    ui.panel_map.at("game_over").second = true;
                    ui.panel_map.at("game_ui").second = false;
                }
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
                    char str[] = "Stage x";
                    str[6] = '1' + current_level;   // TODO: support 10 levels or more
                    ui.panel_map.at("transition").first.writings[0].Update(str);
                    state = GameState::Transition;
                    transition_t = 0;
                    ui.panel_map.at("transition").second = true;
                }
            }

            if (grab_weapon) {
                weapon.SetPlayerArmed();
                for (auto& ghost : ghosts) {
                    ghost.Frighten();
                }
            }
            if (weapon.player_armed) {
                weapon.Update(delta, player.precise_x, player.precise_y);
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
                    player.lives = 3;
                    current_level = 0;
                    LoadLevel(level_filenames[current_level].c_str());
                    ui.panel_map.at("game_ui").second = true;
                    ui.panel_map.at("main_menu").second = false;
                    state = GameState::Transition;
                    char str[] = "Stage x";
                    str[6] = '1' + current_level;
                    ui.panel_map.at("transition").first.writings[0].Update(str);
                    ui.panel_map.at("transition").second = true;
                    transition_t = 0;
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
        else if (state == GameState::Over) {
            if ((wasd & 256) && !(prev_wasd & 256)) {
                state = GameState::MainMenu;
                ui.panel_map.at("main_menu").second = true;
                ui.panel_map.at("game_over").second = false;
            }
            prev_wasd = wasd;
        }
        else if (state == GameState::Pause) {
            if ((wasd & 256) && !(prev_wasd & 256)) {
                if (pause_menu_selected == 0) {
                    // Resume
                    state = GameState::Game;
                    ui.panel_map.at("game_ui").second = true;
                    ui.panel_map.at("pause").second = false;
                    prev_wasd = wasd;
                    return;
                }
                else if (pause_menu_selected == 1) {
                    // Back to menu
                    state = GameState::MainMenu;
                    ui.panel_map.at("main_menu").second = true;
                    ui.panel_map.at("pause").second = false;
                    ui.panel_map.at("game_ui").second = false;
                    prev_wasd = wasd;
                    return;
                }
            }
            else if ((wasd & 512) && !(prev_wasd & 512)) {
                // Resume
                state = GameState::Game;
                ui.panel_map.at("pause").second = false;
                prev_wasd = wasd;
                return;
            }
            else if ((wasd & 16) && !(prev_wasd & 16)) {
                ui.panel_map.at("pause").first.writings[pause_menu_selected].highlighted = false;
                pause_menu_selected = (pause_menu_selected + (2 - 1)) & 1;  // -1 e poi %2
                ui.panel_map.at("pause").first.writings[pause_menu_selected].highlighted = true;
            }
            else if ((wasd & 64) && !(prev_wasd & 64)) {
                ui.panel_map.at("pause").first.writings[pause_menu_selected].highlighted = false;
                pause_menu_selected = (pause_menu_selected + 1) & 1;  // +1 e poi %2
                ui.panel_map.at("pause").first.writings[pause_menu_selected].highlighted = true;
            }
            prev_wasd = wasd;
        }
        else if (state == GameState::Transition) {
            transition_t += delta;
            if (transition_t >= kTransitionDuration) {
                state = GameState::Game;
                ui.panel_map.at("transition").second = false;
                prev_wasd = wasd;
                return;
            }
        }

    }

    void Render() {

        if (state == GameState::Game || state == GameState::Pause || state == GameState::Transition) {
            map.Render();
            wall.Render();
            teleport.Render();
            crust.Render();
            player.Render();
            weapon.Render();
            for (const auto& ghost : ghosts) {
                ghost.Render();
            }
        }

        glEnable(GL_BLEND);
        ui.Render();
        glDisable(GL_BLEND);
    }

    void LoadLevel(const char* filename) {

        LevelDesc level = ReadLevelDesc((std::filesystem::path(kLevelRoot) / std::filesystem::path(filename)).string().c_str());

        map.LoadLevel(level);
        wall.LoadLevel(level);
        teleport.LoadLevel(level);
        player.LoadLevel(level);
        crust.LoadLevel(level);
        weapon.LoadLevel(level);
        for (auto& ghost : ghosts) {
            ghost.LoadLevel(level);
        }
    }

};

const std::vector<Ghost::Color> Game::ghost_colors = { Ghost::Color::Red, Ghost::Color::Yellow, Ghost::Color::Blue, Ghost::Color::Brown, Ghost::Color::Purple };
//const std::vector<Ghost::Color> Game::ghost_colors = { Ghost::Color::Purple };


#endif // NIKMAN_GAME_H