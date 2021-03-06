#if !defined NIKMAN_LEVEL_H
#define NIKMAN_LEVEL_H

#include <vector>
#include <fstream>
#include <iostream>


struct LevelDesc {

    int h;
    int w;

    std::vector<std::pair<int, int>> ver_walls;
    std::vector<std::pair<int, int>> hor_walls;
    std::pair<int, int> nik_pos;
    std::pair<int, int> ste_pos;
    std::vector<std::pair<int, int>> weapons;
    std::vector<std::pair<int, int>> home;
    std::vector<std::pair<int, int>> mud;
    std::vector<std::pair<int, int>> empty;
    std::vector<std::pair<int, int>> teleports;

};

LevelDesc ReadLevelDesc(const char* filename) {

#define INVALID_FORMAT    { std::cerr << "Error in ReadLevelDesc: invalid format.\n";  return level; }
#define EXPECT_CHAR(c)  { if (is.get() != (c)) INVALID_FORMAT }

    LevelDesc level;

    auto UseLetterLambda = [&](int c, int x, int y)->bool {
        switch (c) {
        case 'n':
            level.nik_pos = std::make_pair(x, y);    break;
        case 's':
            level.ste_pos = std::make_pair(x, y);    break;
        case 'w':
            level.weapons.emplace_back(x, y);   break;
        case 'h':
            level.home.emplace_back(x, y);  break;
        case 'm':
            level.mud.emplace_back(x, y);  break;
        case 'e':
            level.empty.emplace_back(x, y);  break;
        case 't':
            level.teleports.emplace_back(x, y);  break;
        case ' ':
            break;
        default:
            return false;
        }
        return true;
    };

    std::ifstream is(filename);
    if (!is.is_open()) {
        std::cerr << "Error in ReadLevelDesc: can't open filename.\n";
        return level;
    }

    is.seekg(0, std::ios_base::end);
    size_t filesize = is.tellg();
    is.seekg(0, std::ios_base::beg);

    int c;

    // Read first line
    int w = 0;
    while (true) {
        EXPECT_CHAR('+')

            c = is.get();
        if (c == '-') {
            for (int i = 0; i < 2; ++i) {
                EXPECT_CHAR('-')
            }
            ++w;
        }
        else if (c == '\n') {
            break;
        }
        else
            INVALID_FORMAT
    }

    int h = (filesize / (w * 4 + 2) - 1) / 2;

    if (w < 1 || h < 1) {
        INVALID_FORMAT
    }

    level.h = h;
    level.w = w;

    int y = h - 1;
    int x = 0;

    for (y = h - 1; y > 0; --y) {

        EXPECT_CHAR('|')
            EXPECT_CHAR(' ')
            for (x = 0; x < w - 1; ++x) {
                c = is.get();
                if (!UseLetterLambda(c, x, y)) {
                    INVALID_FORMAT
                }
                EXPECT_CHAR(' ')
                    c = is.get();
                if (c == '|') {
                    level.ver_walls.emplace_back(x + 1, y);
                }
                else if (c != ' ') {
                    INVALID_FORMAT
                }
                EXPECT_CHAR(' ')
            }
        c = is.get();
        if (!UseLetterLambda(c, x, y)) {
            INVALID_FORMAT
        }
        EXPECT_CHAR(' ')
            EXPECT_CHAR('|')
            EXPECT_CHAR('\n')

            // Read wall line
            for (x = 0; x < w; ++x) {
                EXPECT_CHAR('+')
                    c = is.get();
                if (c == ' ') {
                    for (int i = 0; i < 2; ++i) {
                        EXPECT_CHAR(' ')
                    }
                }
                else if (c == '-') {
                    for (int i = 0; i < 2; ++i) {
                        EXPECT_CHAR('-')
                    }
                    level.hor_walls.emplace_back(x, y);
                }
                else {
                    INVALID_FORMAT
                }
            }
        EXPECT_CHAR('+')
            EXPECT_CHAR('\n')
            // End wall line
    }

    EXPECT_CHAR('|')
        EXPECT_CHAR(' ')
        for (x = 0; x < w - 1; ++x) {
            c = is.get();
            if (!UseLetterLambda(c, x, y)) {
                INVALID_FORMAT
            }
            EXPECT_CHAR(' ')
                c = is.get();
            if (c == '|') {
                level.ver_walls.emplace_back(x + 1, y);
            }
            else if (c != ' ') {
                INVALID_FORMAT
            }
            EXPECT_CHAR(' ')
        }
    c = is.get();
    if (!UseLetterLambda(c, x, y)) {
        INVALID_FORMAT
    }
    EXPECT_CHAR(' ')
        EXPECT_CHAR('|')
        EXPECT_CHAR('\n')

        // Read end line
        for (x = 0; x < w; ++x) {
            EXPECT_CHAR('+')
                for (int i = 0; i < 3; ++i) {
                    EXPECT_CHAR('-')
                }
        }
    EXPECT_CHAR('+')
        // EXPECT_CHAR('\n')

        // Add external walls
        for (x = 0; x < w; ++x) {
            level.hor_walls.emplace_back(x, 0);
            level.hor_walls.emplace_back(x, h);
        }
    for (y = 0; y < h; ++y) {
        level.ver_walls.emplace_back(0, y);
        level.ver_walls.emplace_back(w, y);
    }

    return level;

#undef EXPECT_CHAR
#undef INVALID_FORMAT
}

#endif // NIKMAN_LEVEL_H