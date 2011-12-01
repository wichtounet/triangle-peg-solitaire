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

inline bool win(const std::vector<int>& puzzle, const std::vector<unsigned int>& cases){
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

struct Move {
    unsigned int i;
    unsigned int from;
    unsigned int by;
    unsigned int to;
    Direction direction;
};

struct Move2 {
    unsigned int i;
    unsigned int j;
    unsigned int from;
    unsigned int by;
    unsigned int to;
    std::vector<unsigned int> holes;
};

void solve(int levels, int hole){
    std::cout << "Generate solutions for a triangular solitaire with " << levels << " levels" << std::endl;
    std::cout << "With hole at position " << hole << std::endl;

    /*std::cout << "ignore " << (unsigned int) Direction::IGNORE << std::endl;
    std::cout << "left " << (unsigned int) Direction::LEFT << std::endl;
    std::cout << "right " << (unsigned int) Direction::RIGHT << std::endl;
    std::cout << "up left " << (unsigned int) Direction::UP_LEFT << std::endl;
    std::cout << "up right " << (unsigned int) Direction::UP_RIGHT << std::endl;
    std::cout << "down left " << (unsigned int) Direction::DOWN_LEFT << std::endl;
    std::cout << "down right " << (unsigned int) Direction::DOWN_RIGHT << std::endl;*/

    std::vector<int> puzzle2(levels * levels + 1, true);

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
    std::vector<std::vector<Move>> intos(levels * levels + 1);
    for(unsigned int i = 0; i < cases.size(); ++i){
        int index = cases[i];

        if(canMoveLeft(index)){
            intos[moveLeft(index)].push_back({i, index, index - 1, moveLeft(index), Direction::LEFT});
        }

        if(canMoveRight(index)){
            intos[moveRight(index)].push_back({i, index, index + 1, moveRight(index), Direction::RIGHT});
        }

        if(canMoveUpRight(index)){
            intos[moveUpRight(index)].push_back({i, index, index - levels, moveUpRight(index), Direction::UP_RIGHT});
        }

        if(canMoveUpLeft(index)){
            intos[moveUpLeft(index)].push_back({i, index, index - levels - 1, moveUpLeft(index), Direction::UP_LEFT});
        }

        if(canMoveDownRight(index)){
            intos[moveDownRight(index)].push_back({i, index, index + levels + 1, moveDownRight(index), Direction::DOWN_RIGHT});
        }

        if(canMoveDownLeft(index)){
            intos[moveDownLeft(index)].push_back({i, index, index + levels, moveDownLeft(index), Direction::DOWN_LEFT});
        }
    }
    
    int mappedHole = map(hole, levels);
    assert(mappedHole <= levels * levels);
    puzzle2[mappedHole] = false;

    std::vector<Move2> solution2;
    
    unsigned long solutions2 = 0;

    bool backtrace = false;
    bool restart = false;

    Move2 lastMove2;

    std::vector<unsigned int> holes;
    holes.push_back(mappedHole);

    while(true){
        for(unsigned int i = 0; i < holes.size(); ++i){
            unsigned int j = 0;

            if(backtrace){
                holes.swap(lastMove2.holes);
                i = lastMove2.i;
                j = lastMove2.j + 1;
                backtrace = false;
            }

            unsigned int hole = holes[i];

            for(; j < intos[hole].size(); ++j){
                Move& move = intos[hole][j];
                
                assert(hole == move.to);

                if(puzzle2[move.from] && !puzzle2[hole] && puzzle2[move.by]){
                    puzzle2[move.from] = false;
                    puzzle2[move.by] = false;
                    puzzle2[move.to] = true;
                    
                    solution2.push_back({i, j, move.from, move.by, move.to, holes});

                    holes.erase(remove(holes.begin(), holes.end(), move.to), holes.end());            
                    holes.push_back(move.by);
                    holes.push_back(move.from);
                    
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
        
        solutions2 += win(puzzle2, cases);
        
        if(!solution2.empty()){
            //We undo the last move
            lastMove2 = solution2.back();
            solution2.pop_back();

            puzzle2[lastMove2.from] = true;
            puzzle2[lastMove2.by] = true;
            puzzle2[lastMove2.to] = false;

            holes.erase(remove(holes.begin(), holes.end(), lastMove2.by), holes.end());            
            holes.erase(remove(holes.begin(), holes.end(), lastMove2.from), holes.end());            
            holes.push_back(lastMove2.to);
            
            backtrace = true;
        } else {
            //We searched everything
            break;
        }
    }
   
    std::cout << "Found " << solutions2/*.size()*/ << " solutions" << std::endl;
}

void display(const std::vector<int>& puzzle, int levels){
    for(int i = 1; i <= levels; ++i){
        int start = (i - 1) * levels + 1;

        for(int j = 0; j < i; ++j){
            std::cout << puzzle[start + j] << " ";
        }

        std::cout << std::endl;
    }
}
