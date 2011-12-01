#include <iostream>
#include <iterator>
#include <vector>
#include <stack>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

void solve(int hole);
void display(const std::vector<unsigned int>& puzzle);

int levels;

int main(int argc, const char* argv[]) {
    if(argc < 3){
        std::cout << "Not enough options provided" << std::endl;
    } else {
        levels = strtol(argv[1], 0, 10); 
        int hole = strtol(argv[2], 0, 10); 

        solve(hole);
    }
}

//For hole index
int map(int index, int levels){
    int mapped = 1;
    for(int i = 1; i <= levels; ++i){
        int start = (i - 1) * levels + 1;
        
        for(int j = 0; j < i; ++j){
            if(mapped == index){
                return start + j;         
            }

            ++mapped;
        }
    }

    assert(false);
}

inline int level(int index){
   return (index - 1) / levels + 1; 
}

inline int moveRight(int index){
    return index + 2;
}

inline int moveLeft(int index){
    return index - 2;
}

inline int moveDownLeft(int index){
    return index + (levels * 2);
}

inline int moveDownRight(int index){
    return index + (levels * 2 + 2);
}

inline int moveUpRight(int index){
    return index - (levels * 2);
}

inline int moveUpLeft(int index){
    return index - (levels * 2 + 2);
}

inline bool valid(int index){
    int l = level(index);

    return l <= levels && index >= 1 && index <= (levels * l) - (levels - l);
}

inline bool canMoveLeft(int index){
    int l = level(index);

    return level(moveLeft(index)) == l && valid(moveLeft(index));
}

inline bool canMoveRight(int index){
    int l = level(index);

    return level(moveRight(index)) == l && valid(moveRight(index));
}

inline bool canMoveUpRight(int index){
    int l = level(index);

    return level(moveUpRight(index)) == l - 2 && valid(moveUpRight(index));
}

inline bool canMoveUpLeft(int index){
    int l = level(index);

    return level(moveUpLeft(index)) == l - 2 && valid(moveUpLeft(index));
}

inline bool canMoveDownRight(int index){
    int l = level(index);

    return level(moveDownRight(index)) == l + 2 && valid(moveDownRight(index));
}

inline bool canMoveDownLeft(int index){
    int l = level(index);

    return level(moveDownLeft(index)) == l + 2 && valid(moveDownLeft(index));
}

inline bool win(const std::vector<unsigned int>& puzzle/*, const std::vector<unsigned int>& cases*/){
    int sum = 0;

    for(unsigned int i = 1; i < puzzle.size(); ++i){
        sum += puzzle[i];
    }

    return sum == 1;
}

struct Move {
    unsigned int i;//hole = cases[i]
    unsigned int j;//intos[hole][j]
    unsigned int from;
    unsigned int by;
};

void solve(int hole){
    std::cout << "Generate solutions for a triangular solitaire with " << levels << " levels" << std::endl;
    std::cout << "With hole at position " << hole << std::endl;

    std::vector<unsigned int> puzzle(((levels + 1) * levels) / 2 + 1, true);

    std::vector<std::vector<Move>> intos(((levels + 1) * levels) / 2 + 1);
    {
        int index = 1;

        for(int level = 1; level <= levels; ++level){
            int start = (level - 1) * levels + 1;

            for(int col = 0; col < level; ++col){
                int fake = start + col;

                if(canMoveLeft(fake)){
                    intos[index - 2].push_back({-1, -1, index, index - 1});
                }

                if(canMoveRight(fake)){
                    intos[index + 2].push_back({-1, -1, index, index + 1});
                }

                if(canMoveUpRight(fake)){
                    intos[index - (3 + (level - 3) * 2)].push_back({-1, -1, index, index - level + 1});
                }

                if(canMoveUpLeft(fake)){
                    intos[index - (5 + (level - 3) * 2)].push_back({-1, -1, index, index - level});
                }

                if(canMoveDownRight(fake)){
                    intos[index + (5 + (level - 1) * 2)].push_back({-1, -1, index, index + level + 1});
                }

                if(canMoveDownLeft(fake)){
                    intos[index + (3 + (level - 1) * 2)].push_back({-1, -1, index, index + level});
                }

                ++index;
            }
        }
    }
    
    for(unsigned int i = 1; i < intos.size(); ++i){
        for(unsigned int j = 0; j < intos[i].size(); ++j){
            intos[i][j].i = i;
            intos[i][j].j = j;
        }
    }

    puzzle[hole] = false;

    std::vector<Move> solution;
    
    unsigned long solutions = 0;

    bool backtrace = false;
    bool restart = false;

    Move lastMove;

    int current = ((levels + 1) * levels) / 2 - 1;

    while(true){
        for(unsigned int i = 1; i < puzzle.size(); ++i){
            unsigned int j = 0;

            if(backtrace){
                i = lastMove.i;
                j = lastMove.j + 1;
                backtrace = false;
            }

            if(!puzzle[i]){
                for(; j < intos[i].size(); ++j){
                    Move& move = intos[i][j];

                    if(puzzle[move.from] && puzzle[move.by]){
                        puzzle[move.from] = false;
                        puzzle[move.by] = false;
                        puzzle[i] = true;
                        --current;

                        solution.push_back(move);

                        restart = true;
                        break;
                    }
                }

                if(restart){
                    break;
                }
            }
        }

        if(restart){
            restart = false;
            continue;
        }
        
        solutions += current == 1;
        
        if(!solution.empty()){
            //We undo the last move
            lastMove = solution.back();
            solution.pop_back();

            puzzle[lastMove.from] = true;
            puzzle[lastMove.by] = true;
            puzzle[lastMove.i] = false;
            ++current;
            
            backtrace = true;
        } else {
            //We searched everything
            break;
        }
    }
   
    std::cout << "Found " << solutions/*.size()*/ << " solutions" << std::endl;
}

void display(const std::vector<unsigned int>& puzzle){
    int index = 0;
    for(int i = 1; i <= levels; ++i){
        for(int j = 0; j < i; ++j){
            std::cout << puzzle[++index] << " ";
        }

        std::cout << std::endl;
    }
}
