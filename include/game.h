// MIT License
// 
// Copyright (c) 2021 Stefano Allegretti, Davide Papazzoni, Nicola Baldini, Lorenzo Governatori e Simone Gemelli
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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

    Sfondo sfondo;
    Map map;
    Tile mud;
    Tile home;
    Crust crust;
    Weapon weapon;
    Player nik;
    Player ste;
    Wall wall;
    Teleport teleport;
    std::vector<Ghost> ghosts;
    UI ui;
    GameState state;
    unsigned int prev_wasd = 0;
    int main_menu_selected = 0;
    int pause_menu_selected = 0;
    const float kTransitionDuration = 1.f;
    float transition_t;
    int score = 0;

    static constexpr int crustScore = 1;
    static constexpr int weaponScore = 5;
    static constexpr int killScore = 10;
    static constexpr int lifePrice = 500;

    std::random_device rd;
    std::mt19937 mt;

    sf::SoundBuffer gameOverBuffer;
    sf::Sound gameOver;

    sf::SoundBuffer endLevelBuffer;
    sf::Sound endLevel;

    sf::SoundBuffer grabWeaponBuffer;
    sf::Sound grabWeapon;

    sf::SoundBuffer winBuffer;
    sf::Sound win;

    sf::Music music;

    Game() :
        state(GameState::MainMenu),
        mt(rd()),
        map(),
        mud("mud"),
        home("home"),
        wall(),
        crust(map.grid),
        teleport(map.grid, mt),
        nik(Player::Name::Nik, map.grid, teleport),
        ste(Player::Name::Ste, map.grid, teleport),
        weapon(map.grid, nik, ste)
    {
        level_filenames = LoadLevelsList();

        Ghost::shader = Shader("ghost");
        for (const auto color : ghost_colors) {
            ghosts.emplace_back(color, map.grid, teleport, mt, nik, ste);
        }

        LoadLevel(level_filenames[current_level].c_str(), mt);

        ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = true;
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        if (!gameOverBuffer.loadFromFile(SoundPath("arato.wav"))) {
            std::cerr << "Game::Game: can't open file \"arato.wav\"\n";
        }
        gameOver.setBuffer(gameOverBuffer);
        gameOver.setVolume(20.f);

        if (!grabWeaponBuffer.loadFromFile(SoundPath("nooo.wav"))) {
            std::cerr << "Game::Game: can't open file \"nooo.wav\"\n";
        }
        grabWeapon.setBuffer(grabWeaponBuffer);

        if (!endLevelBuffer.loadFromFile(SoundPath("concettualmente.wav"))) {
            std::cerr << "Game::Game: can't open file \"concettualmente.wav\"\n";
        }
        endLevel.setBuffer(endLevelBuffer);
        endLevel.setVolume(25.f);

        if (!winBuffer.loadFromFile(SoundPath("luna.wav"))) {
            std::cerr << "Game::Game: can't open file \"luna.wav\"\n";
        }
        win.setBuffer(winBuffer);
        win.setVolume(5.f);

        if (!music.openFromFile(SoundPath("music.wav"))) {
            std::cerr << "Game::Game: can't open file \"music.wav\"\n";
        }
        music.setVolume(5.f);

    }

    ~Game() {
        Ghost::shader.Release();
    }

    // wasd is a bitmapped value containing the keys pressed
    // 0  1  2  3  4   5     6     7      8      9    
    // W  A  S  D  Up  Left  Down  Right  Enter  Esc  
    void Update(float delta, unsigned wasd, bool& stop_game) {

        if (state == GameState::Game) {

            int scoreDelta = 0;

            if ((wasd & 512) && !(prev_wasd & 512)) {
                state = GameState::Pause;
                ui.panel_map.at("pause").second = true;
                pause_menu_selected = 0;
                ui.panel_map.at("pause").first.writings[pause_menu_selected].highlighted = true;
                ui.panel_map.at("pause").first.writings[(pause_menu_selected + 1) % 2].highlighted = false;
                prev_wasd = wasd;
                music.pause();
                return;
            }

            unsigned int eaten = 0;
            bool grabWeaponNik = false;
            bool grabWeaponSte = false;

            unsigned int wasdNik = wasd >> 4;
            if (!isSte) {
                wasdNik |= wasd;
            }
            nik.Update(delta, wasdNik, ste.precise_x, ste.precise_y, eaten, grabWeaponNik);

            if (isSte) {
                ste.Update(delta, wasd, nik.precise_x, nik.precise_y, eaten, grabWeaponSte);
            }

            scoreDelta += eaten * crustScore;
            scoreDelta += (grabWeaponNik + grabWeaponSte) * weaponScore;

            bool hitNik = false;
            bool hitSte = false;
            for (auto& ghost : ghosts) {
                bool collisionNik = false;
                bool collisionSte = false;
                ghost.Update(delta, ghosts[0].precise_x, ghosts[0].precise_y, collisionNik, collisionSte);
                if (collisionNik) {
                    if (nik.armed) {
                        ghost.Killed();
                        scoreDelta += killScore;
                        weapon.PlaySound();
                        ghost.hitSound.play();
                    }
                    else {
                        hitNik = true;
                    }
                }

                if (isSte && collisionSte) {
                    if (ste.armed) {
                        ghost.Killed();
                        scoreDelta += killScore;
                        weapon.PlaySound();
                        ghost.hitSound.play();
                    }
                    else {
                        hitSte = true;
                    }
                }
            }

            if (hitNik && !nik.just_hit) {
                nik.just_hit = true;
                nik.time_after_hit = 0;
                nik.lives--;
            }

            if (hitSte && !ste.just_hit) {
                ste.just_hit = true;
                ste.time_after_hit = 0;
                ste.lives--;
            }

            if(hitNik || hitSte) {
                char str[] = "Lives: 00";
                snprintf(str + 7, 3, "%d", nik.lives);
                ui.panel_map.at("game_ui").first.writings[0].Update(str);
                //std::cerr << "Updated lives\n";

                if (nik.lives <= 0) {
                    // Game over :(
                    music.stop();
                    gameOver.play();
                    state = GameState::Over;
                    char strScore[] = "Score: 0   ";
                    snprintf(strScore + 7, 5, "%d", score);
                    ui.panel_map.at("game_over").first.writings[1].Update(strScore);

                    ui.panel_map.at("game_over").second = true;
                    ui.panel_map.at("game_ui").second = false;
                }
            }

            map.remaining_crusts -= eaten;
            if (map.remaining_crusts <= 0) { // || (wasd == 2 + 4 + 8 && prev_wasd != 2 + 4 + 8)) {
                // Fine livello!
                current_level++;
                music.stop();
                if (current_level == level_filenames.size()) {
                    win.play();
                    state = GameState::End;                    
                    char strScore[] = "Score: 0   ";
                    snprintf(strScore + 7, 5, "%d", score);
                    ui.panel_map.at("end_game").first.writings[1].Update(strScore);

                    ui.panel_map.at("end_game").second = true;
                    ui.panel_map.at("game_ui").second = false;                    
                }
                else {
                    endLevel.play();
                    LoadLevel(level_filenames[current_level].c_str(), mt);
                    char str[] = "Stage xx";
                    snprintf(str + 6, 3, "%2d", current_level + 1);
                    ui.panel_map.at("transition").first.writings[0].Update(str);
                    state = GameState::Transition;
                    transition_t = 0;
                    ui.panel_map.at("transition").second = true;
                }
            }

            if (grabWeaponNik) {
                nik.GrabWeapon();
                grabWeapon.play();
                for (auto& ghost : ghosts) {
                    ghost.Frighten();
                }
            }

            if (grabWeaponSte) {
                ste.GrabWeapon();
                grabWeapon.play();
                for (auto& ghost : ghosts) {
                    ghost.Frighten();
                }
            }

            prev_wasd = wasd;

            if (scoreDelta) {
                if ((score + scoreDelta) / lifePrice > score / lifePrice) {
                    nik.lives++;
                    char str[] = "Lives: 00";
                    snprintf(str + 7, 3, "%d", nik.lives);
                    ui.panel_map.at("game_ui").first.writings[0].Update(str);
                }
                score += scoreDelta;
                char strScore[] = "Score: 0   ";
                snprintf(strScore + 7, 5, "%d", score);
                ui.panel_map.at("game_ui").first.writings[1].Update(strScore);
            }
        }
        else if (state == GameState::MainMenu) {
            if ((wasd & 256) && !(prev_wasd & 256)) {
                if (main_menu_selected < 2) {
                    // New game
                    if (main_menu_selected == 0) {
                        isSte = false;
                    }
                    else {
                        isSte = true;
                    }
                    nik.lives = 3;
                    current_level = 0;
                    score = 0;
                    LoadLevel(level_filenames[current_level].c_str(), mt);
                    ui.panel_map.at("game_ui").second = true;
                    ui.panel_map.at("main_menu").second = false;
                    state = GameState::Transition;
                    char str[] = "Stage xx";
                    snprintf(str + 6, 3, "%2d", current_level + 1);
                    ui.panel_map.at("transition").first.writings[0].Update(str);
                    ui.panel_map.at("transition").second = true;
                    transition_t = 0;

                    char strLives[] = "Lives: 00";
                    snprintf(strLives + 7, 3, "%d", nik.lives);
                    ui.panel_map.at("game_ui").first.writings[0].Update(strLives);

                    char strScore[] = "Score: 0   ";
                    ui.panel_map.at("game_ui").first.writings[1].Update(strScore);

                    return;
                }
                else if (main_menu_selected == 2) {
                    // Quit
                    stop_game = true;
                    return;
                }
            }
            else if ((wasd & 16) && !(prev_wasd & 16)) {
                ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = false;
                main_menu_selected = (main_menu_selected + (3 - 1)) % 3;  // -1 e poi %3
                ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = true;
            }
            else if ((wasd & 64) && !(prev_wasd & 64)) {
                ui.panel_map.at("main_menu").first.writings[main_menu_selected].highlighted = false;
                main_menu_selected = (main_menu_selected + 1) % 3;  // +1 e poi %3
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
                    music.play();
                    return;
                }
                else if (pause_menu_selected == 1) {
                    // Back to menu
                    state = GameState::MainMenu;
                    ui.panel_map.at("main_menu").second = true;
                    ui.panel_map.at("pause").second = false;
                    ui.panel_map.at("game_ui").second = false;
                    prev_wasd = wasd;
                    music.stop();
                    return;
                }
            }
            else if ((wasd & 512) && !(prev_wasd & 512)) {
                // Resume
                state = GameState::Game;
                ui.panel_map.at("pause").second = false;
                prev_wasd = wasd;
                music.play();
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
                music.play();
                return;
            }
        }

    }

    void Render() {

        if (state == GameState::Game || state == GameState::Pause || state == GameState::Transition) {
            map.Render();
            mud.Render();
            home.Render();
            wall.Render();
            teleport.Render();
            crust.Render();
            nik.Render();
            if (isSte) ste.Render();
            weapon.Render();
            for (const auto& ghost : ghosts) {
                ghost.Render();
            }
        }
        else if (state == GameState::MainMenu) {
            sfondo.Render();
        }

        ui.Render();
    }

    void LoadLevel(const char* filename, std::mt19937& mt) {

        LevelDesc level = ReadLevelDesc((std::filesystem::path(kLevelRoot) / std::filesystem::path(filename)).string().c_str());

        map.LoadLevel(level, mt);
        mud.LoadLevel(level, level.mud);
        home.LoadLevel(level, level.home);
        wall.LoadLevel(level);
        teleport.LoadLevel(level);
        nik.LoadLevel(level, current_level);
        if(isSte) ste.LoadLevel(level, current_level);
        crust.LoadLevel(level);
        weapon.LoadLevel(level);
        for (auto& ghost : ghosts) {
            ghost.LoadLevel(level, current_level);
        }        

    }

};

const std::vector<Ghost::Color> Game::ghost_colors = { Ghost::Color::Red, Ghost::Color::Yellow, Ghost::Color::Blue, Ghost::Color::Purple };
//const std::vector<Ghost::Color> Game::ghost_colors = { Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, 
//Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Yellow, Ghost::Color::Blue, Ghost::Color::Blue, Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue,Ghost::Color::Blue, Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red,Ghost::Color::Red, };
//const std::vector<Ghost::Color> Game::ghost_colors = { Ghost::Color::Red, Ghost::Color::Yellow, Ghost::Color::Blue, Ghost::Color::Purple };
//const std::vector<Ghost::Color> Game::ghost_colors = { Ghost::Color::Red, Ghost::Color::Purple };
//const std::vector<Ghost::Color> Game::ghost_colors = { Ghost::Color::Blue };


#endif // NIKMAN_GAME_H