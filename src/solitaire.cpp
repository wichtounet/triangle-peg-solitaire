#include <iostream>
#include <vector>

void solve(int levels);
void display(const std::vector<bool>& puzzle, int levels);

int main(int argc, const char* argv[]) {
    if(argc < 2){
        std::cout << "Not enough options provided" << std::endl;
    } else {
        int levels = strtol(argv[1], 0, 10); 

        solve(levels);
    }
}

int level(int index){
   return (index - 1) / 4 + 1; 
}

int moveRight(int index){
    return +3;
}

int moveLeft(int index){
    return -3;
}

int moveDownLeft(int index){
    return 1 + level(index) * 2;
}

int moveDownRigt(int index){
    return 3 + level(index) * 2;
}

int moveUpRight(int index){
    return -2 * level(index) + 1;
}

int moveUpLeft(int index){
    return -2 * level(index) - 1;
}

void solve(int levels){
    std::cout << "Generate solutions for a triangular solution with " << levels << " levels" << std::endl;

    std::vector<bool> puzzle;

    for(int i = 0; i <= levels * levels; ++i){
        puzzle.push_back(true);
    }

    display(puzzle, levels); 
}

void display(const std::vector<bool>& puzzle, int levels){
    for(int i = 1; i <= levels; ++i){
        int start = (i - 1) * levels + 1;

        for(int j = 0; j < i; ++j){
            std::cout << puzzle[start + j] << " ";
        }

        std::cout << std::endl;
    }
}
