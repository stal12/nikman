#include <filesystem>

int main(void) {
    auto path = std::filesystem::current_path(); //getting path
    std::filesystem::current_path(path / std::filesystem::path("bin")); //setting path
    system("Game.exe");
}