#include <iostream>
#include <iterator>
#include <vector>
#include <stack>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

void solve(int levels, int hole);
void display(const std::vector<int>& puzzle, int levels);

int levels;

int main(int argc, const char* argv[]) {
    if(argc < 3){
        std::cout << "Not enough options provided" << std::endl;
    } else {
        levels = strtol(argv[1], 0, 10); 
        int hole = strtol(argv[2], 0, 10); 

        solve(levels, hole);
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

inline bool win(const std::vector<unsigned int>& puzzle, const std::vector<unsigned int>& cases){
    int sum = 0;

    for(int index : cases){
        sum += puzzle[index];
    }

    return sum == 1;
}

enum class Direction : unsigned int {
    IGNORE,
    LEFT,
    RIGHT,
    UP_LEFT,
    UP_RIGHT,
    DOWN_LEFT,
    DOWN_RIGHT
};

struct Move2 {
    unsigned int i;
    unsigned int j;
    unsigned int from;
    unsigned int by;
};

void solve(int levels, int hole){
    std::cout << "Generate solutions for a triangular solitaire with " << levels << " levels" << std::endl;
    std::cout << "With hole at position " << hole << std::endl;

    std::vector<unsigned int> puzzle(levels * levels + 1, true);

    std::vector<unsigned int> cases;
    cases.reserve((levels + 1) * (levels / 2));
    for(int i = 1; i <= levels; ++i){
        int start = (i - 1) * levels + 1;

        for(int j = 0; j < i; ++j){
            //std::cout << (start + j) << " ";
            cases.push_back(start + j);
        }
    }

    //std::cout << std::endl;

    //Precalculate all the moves
    std::vector<std::vector<Move2>> intos(levels * levels + 1);
    for(unsigned int i = 0; i < cases.size(); ++i){
        int index = cases[i];

        if(canMoveLeft(index)){
            intos[moveLeft(index)].push_back({i, -1, index, index - 1});
        }

        if(canMoveRight(index)){
            intos[moveRight(index)].push_back({i, -1, index, index + 1});
        }

        if(canMoveUpRight(index)){
            intos[moveUpRight(index)].push_back({i, -1, index, index - levels});
        }

        if(canMoveUpLeft(index)){
            intos[moveUpLeft(index)].push_back({i, -1, index, index - levels - 1});
        }

        if(canMoveDownRight(index)){
            intos[moveDownRight(index)].push_back({i, -1, index, index + levels + 1});
        }

        if(canMoveDownLeft(index)){
            intos[moveDownLeft(index)].push_back({i, -1, index, index + levels});
        }
    }

    for(unsigned int i = 0; i < cases.size(); ++i){
        unsigned int hole = cases[i];
        
        for(unsigned int j = 0; j < intos[hole].size(); ++j){
            intos[hole][j].i = i;
            intos[hole][j].j = j;
        }
    }
    
    int mappedHole = map(hole, levels);
    assert(mappedHole <= levels * levels);
    puzzle[mappedHole] = false;

    std::vector<Move2> solution;
    
    unsigned long solutions = 0;

    bool backtrace = false;
    bool restart = false;

    Move2 lastMove2;

    while(true){
        for(unsigned int i = 0; i < cases.size(); ++i){
            unsigned int j = 0;

            if(backtrace){
                i = lastMove2.i;
                j = lastMove2.j + 1;
                backtrace = false;
            }
            
            unsigned int hole = cases[i];

            if(puzzle[hole]){
                continue;
            }

            for(; j < intos[hole].size(); ++j){
                Move2& move = intos[hole][j];
                
                if(puzzle[move.from] && puzzle[move.by]){
                    puzzle[move.from] = false;
                    puzzle[move.by] = false;
                    puzzle[hole] = true;
                    
                    solution.push_back(move);
                    
                    restart = true;
                    break;
                }
            }

            if(restart){
                break;
            }
        }

        if(restart){
            restart = false;
            continue;
        }
        
        solutions += win(puzzle, cases);
        
        if(!solution.empty()){
            //We undo the last move
            lastMove2 = solution.back();
            solution.pop_back();

            puzzle[lastMove2.from] = true;
            puzzle[lastMove2.by] = true;
            puzzle[cases[lastMove2.i]] = false;
            
            backtrace = true;
        } else {
            //We searched everything
            break;
        }
    }
   
    std::cout << "Found " << solutions/*.size()*/ << " solutions" << std::endl;
}

void display(const std::vector<unsigned int>& puzzle, int levels){
    for(int i = 1; i <= levels; ++i){
        int start = (i - 1) * levels + 1;

        for(int j = 0; j < i; ++j){
            std::cout << puzzle[start + j] << " ";
        }

        std::cout << std::endl;
    }
}
