#include <iostream>
#include <iterator>
#include <vector>
#include <cassert>
#include <unordered_map>

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

inline bool canMoveLeft(int index){
    int l = level(index);

    return level(moveLeft(index)) == l && moveLeft(index) >= 1;
}

inline bool canMoveRight(int index){
    int l = level(index);

    return level(moveRight(index)) == l;
}

inline bool canMoveUpRight(int index){
    int l = level(index);

    return level(moveUpRight(index)) == l - 2 && moveUpRight(index) >= 1;
}

inline bool canMoveUpLeft(int index){
    int l = level(index);

    return level(moveUpLeft(index)) == l - 2 && moveUpLeft(index) >= 1;
}

inline bool canMoveDownRight(int index){
    int l = level(index);

    return level(moveDownRight(index)) == l + 2 && moveDownRight(index) <= levels * levels;
}

inline bool canMoveDownLeft(int index){
    int l = level(index);

    return level(moveDownLeft(index)) == l + 2 && moveDownLeft(index) <= levels * levels;
}

inline bool win(const std::vector<int>& puzzle, const std::vector<int>& cases){
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
    int i;
    int from;
    int by;
    int to;
    Direction direction;
};

void solve(int levels, int hole){
    std::cout << "Generate solutions for a triangular solitaire with " << levels << " levels" << std::endl;
    std::cout << "With hole at position " << hole << std::endl;

    std::vector<int> puzzle(levels * levels + 1, true);

    std::vector<int> cases;
    cases.reserve((levels + 1) * (levels / 2));
    for(int i = 1; i <= levels; ++i){
        int start = (i - 1) * levels + 1;

        for(int j = 0; j < i; ++j){
            cases.push_back(start + j);
        }
    }

    //Precalculate all the moves
    std::vector<std::vector<Move>> moves(levels * levels + 1);
    for(unsigned int i = 0; i < cases.size(); ++i){
        int index = cases[i];

        if(canMoveLeft(index)){
            moves[index].push_back({i, index, index - 1, moveLeft(index), Direction::LEFT});
        }

        if(canMoveRight(index)){
            moves[index].push_back({i, index, index + 1, moveRight(index), Direction::RIGHT});
        }

        if(canMoveUpRight(index)){
            moves[index].push_back({i, index, index - levels, moveUpRight(index), Direction::UP_RIGHT});
        }

        if(canMoveUpLeft(index)){
            moves[index].push_back({i, index, index - levels - 1, moveUpLeft(index), Direction::UP_LEFT});
        }

        if(canMoveDownRight(index)){
            moves[index].push_back({i, index, index + levels + 1, moveDownRight(index), Direction::DOWN_RIGHT});
        }

        if(canMoveDownLeft(index)){
            moves[index].push_back({i, index, index + levels, moveDownLeft(index), Direction::DOWN_LEFT});
        }
    }
    
    int mappedHole = map(hole, levels);
    assert(mappedHole <= levels * levels);
    puzzle[mappedHole] = false;
  
    display(puzzle, levels);

    std::vector<Move> solution;
    //std::vector<std::vector<Move>> solutions;
    unsigned long solutions = 0;

    bool stop = false;
    bool backtrace = false;
    bool restart = false;

    Move lastMove;

    while(!stop){
        for(unsigned int i = 0; i < cases.size(); ++i){
            Direction ignore = Direction::IGNORE;

            if(backtrace){
                i = lastMove.i;
                ignore = lastMove.direction;
                backtrace = false;
            }
            
            int index = cases[i];

            if(puzzle[index]){
                for(Move& move : moves[index]){
                    if(move.direction > ignore && !puzzle[move.to] && puzzle[move.by]){
                        puzzle[index] = false;
                        puzzle[move.by] = false;
                        puzzle[move.to] = true;

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
        
        if(win(puzzle, cases)){
            ++solutions;

            if(solutions % 1000 == 0){
                std::cout << solutions << std::endl;
            }

            //solutions.push_back(solution);

            //We undo the last move
            lastMove = solution.back();
            solution.pop_back();

            puzzle[lastMove.from] = true;
            puzzle[lastMove.by] = true;
            puzzle[lastMove.to] = false;

            backtrace = true;
        } else if(solution.empty()){
            //We searched everything
            stop = true;
        } else {
            //We undo the last move
            lastMove = solution.back();
            solution.pop_back();

            puzzle[lastMove.from] = true;
            puzzle[lastMove.by] = true;
            puzzle[lastMove.to] = false;

            backtrace = true;
        }
    }
   
    std::cout << "Found " << solutions/*.size()*/ << " solutions" << std::endl;
    /*for(auto& solution : solutions){
        for(Move& m : solution){
            //std::cout << m.from << "->" << m.to << " ";    
        }
        //std::cout << std::endl;
    }*/
    
    //display(puzzle, levels); 
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
