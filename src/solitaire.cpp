#include <iostream>
#include <iterator>
#include <vector>
#include <stack>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

unsigned int levels;

void solve(int hole);
void display(const std::vector<unsigned int>& puzzle);

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

inline unsigned int level(unsigned int index){
   return (index - 1) / levels + 1; 
}

inline int moveRight(unsigned int index){
    return index + 2;
}

inline int moveLeft(unsigned int index){
    return index - 2;
}

inline int moveDownLeft(unsigned int index){
    return index + (levels * 2);
}

inline int moveDownRight(unsigned int index){
    return index + (levels * 2 + 2);
}

inline int moveUpRight(unsigned int index){
    return index - (levels * 2);
}

inline int moveUpLeft(unsigned int index){
    return index - (levels * 2 + 2);
}

inline bool valid(unsigned int index){
    unsigned int l = level(index);

    return l <= levels && index >= 1 && index <= (levels * l) - (levels - l);
}

inline bool canMoveLeft(unsigned int index){
    unsigned int l = level(index);

    return level(moveLeft(index)) == l && valid(moveLeft(index));
}

inline bool canMoveRight(unsigned int index){
    unsigned int l = level(index);

    return level(moveRight(index)) == l && valid(moveRight(index));
}

inline bool canMoveUpRight(unsigned int index){
    unsigned int l = level(index);

    return level(moveUpRight(index)) == l - 2 && valid(moveUpRight(index));
}

inline bool canMoveUpLeft(unsigned int index){
    unsigned int l = level(index);

    return level(moveUpLeft(index)) == l - 2 && valid(moveUpLeft(index));
}

inline bool canMoveDownRight(unsigned int index){
    unsigned int l = level(index);

    return level(moveDownRight(index)) == l + 2 && valid(moveDownRight(index));
}

inline bool canMoveDownLeft(unsigned int index){
    unsigned int l = level(index);

    return level(moveDownLeft(index)) == l + 2 && valid(moveDownLeft(index));
}

inline unsigned int score(const std::vector<unsigned int>& puzzle, const std::vector<unsigned int>& indexes){
    unsigned int score = 0;

    for(unsigned int i = 1; i < puzzle.size(); ++i){
        score += puzzle[i] * indexes[i];
    }

    return score;
}

inline bool win(const std::vector<unsigned int>& puzzle){
    int sum = 0;

    for(unsigned int i = 1; i < puzzle.size(); ++i){
        sum += puzzle[i];
    }

    return sum == 1;
}

struct Move {
    unsigned int i;//hole
    unsigned int j;//intos[hole][j]
    unsigned int from;
    unsigned int by;
};

inline void generate_normal_indexes(std::vector<unsigned int>& normal_indexes){
    unsigned int acc = 1;

    for(unsigned int i = 1; i < normal_indexes.size(); ++i){
        normal_indexes[i] = acc;
        acc *= 2; 
    }
}

inline void generate_symetric_indexes(std::vector<unsigned int>& symetric_indexes){
    unsigned int acc = 1;

    for(unsigned int level = 1; level <= levels; ++level){
        int start = (level * (level + 1)) / 2;
        for(unsigned int index = start; index > start - level; --index){
            symetric_indexes[index] = acc;
            acc *= 2; 
        }
    }
}

inline void generate_rotate_once_indexes(std::vector<unsigned int>& rotate_once_indexes){
    std::vector<std::vector<unsigned int>> indexes;

    int index = 1;

    for(unsigned int level = 1; level <= levels; ++level){
        indexes.push_back(std::vector<unsigned int>());

        for(unsigned int col = 0; col < level; ++col){
            indexes[level - 1].push_back(index);

            index++;
        }
       
        std::reverse(indexes[level - 1].begin(), indexes[level - 1].end());
    }

    int tested = ((levels + 1) * levels) / 2;
    int current_level = levels;
    int it = 1;
    int d = 1;

    unsigned int acc = 1;
    
    //init the first level
    --tested;
    rotate_once_indexes[indexes[levels - 1].back()] = acc;
    acc *= 2;
    indexes[levels - 1].pop_back();
        
    while(tested > 0){
        int index = indexes[current_level - 1].back();
        indexes[current_level - 1].pop_back();

        rotate_once_indexes[index] = acc;
        acc *= 2;

        if(d == 0){
            current_level = levels;

            d = it + 2;
            ++it;
        } else {
            --current_level;
        }

        --d;
        --tested;
    }
}

inline void generate_rotate_twice_indexes(std::vector<unsigned int>& rotate_twice_indexes){
    std::vector<std::stack<unsigned int>> indexes;

    int index = 1;

    for(unsigned int level = 1; level <= levels; ++level){
        indexes.push_back(std::stack<unsigned int>());

        for(unsigned int col = 0; col < level; ++col){
            indexes[level - 1].push(index);

            index++;
        }
    }

    int tested = ((levels + 1) * levels) / 2;
    int current_level = levels;
    int it = 1;
    int d = 0;

    unsigned int acc = 1;
    while(tested > 0){
        int index = indexes[current_level - 1].top();
        indexes[current_level - 1].pop();

        rotate_twice_indexes[index] = acc;
        acc *= 2;

        if(d == 0){
            current_level -= it;
            ++it;

            d = levels - current_level + 1;

        } else {
            ++current_level;
        }

        --d;
        --tested;
    }
}

#define PRUNE(indexes)\
firstScore = score(puzzle, indexes);\
if(history.find(firstScore) != history.end()){\
    solutions += history[firstScore];\
    puzzle[move.from] = true;\
    puzzle[move.by] = true;\
    puzzle[i] = false;\
    continue;\
}

void solve(int hole){
    std::cout << "Generate solutions for a triangular solitaire with " << levels << " levels" << std::endl;
    std::cout << "With hole at position " << hole << std::endl;

    std::vector<unsigned int> normal_indexes(((levels + 1) * levels) / 2 + 1);
    generate_normal_indexes(normal_indexes);

    std::vector<unsigned int> symetric_indexes(((levels + 1) * levels) / 2 + 1);
    generate_symetric_indexes(symetric_indexes);
    
    std::vector<unsigned int> rotate_once_indexes(((levels + 1) * levels) / 2 + 1);
    generate_rotate_once_indexes(rotate_once_indexes);
    
    std::vector<unsigned int> rotate_twice_indexes(((levels + 1) * levels) / 2 + 1);
    generate_rotate_twice_indexes(rotate_twice_indexes);

    std::vector<unsigned int> puzzle(((levels + 1) * levels) / 2 + 1, true);
    
    std::vector<std::vector<Move>> intos(((levels + 1) * levels) / 2 + 1);
    {
        int index = 1;

        for(unsigned int level = 1; level <= levels; ++level){
            int start = (level - 1) * levels + 1;

            for(unsigned int col = 0; col < level; ++col){
                unsigned int fake = start + col;

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

    unsigned long solutions = 0;
    std::unordered_map<int, int> history;


    std::vector<Move> solution;

    bool backtrace = false;
    bool restart = false;

    Move lastMove;

    int current = ((levels + 1) * levels) / 2 - 1;

    std::stack<unsigned int> sol;

    while(true){
        for(unsigned int i = 1; i < puzzle.size(); ++i){
            unsigned int j = 0;

            if(backtrace){
                i = lastMove.i;
                j = lastMove.j + 1;
                backtrace = false;
            }
    
            //It's an hole : try to fill it        
            if(!puzzle[i]){
                for(; j < intos[i].size(); ++j){
                    Move& move = intos[i][j];

                    //A potential move is found
                    if(puzzle[move.from] && puzzle[move.by]){
                        puzzle[move.from] = false;
                        puzzle[move.by] = false;
                        puzzle[i] = true;

                        int firstScore;

                        //The subtree has already been calculated
                        PRUNE(normal_indexes)
                        PRUNE(symetric_indexes)
                        PRUNE(rotate_once_indexes)
                        PRUNE(rotate_twice_indexes)
            
                        //If the subtree has not already been computed, we compute it
                        sol.push(solutions);
                        solutions = 0;

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
       
        //There is no more moves 
        if(!solution.empty()){
            history[score(puzzle, normal_indexes)] = solutions;
            history[score(puzzle, symetric_indexes)] = solutions;
            history[score(puzzle, rotate_once_indexes)] = solutions;
            history[score(puzzle, rotate_twice_indexes)] = solutions;
            
            /*#pragma omp parallel num_threads(4) shared(history)
            {
                #pragma omp sections
                {
                    #pragma omp section
                    {
                        int c = score(puzzle, normal_indexes);
                        #pragma omp critical
                        history[c] = solutions;
                    }
                    #pragma omp section
                    {
                        int c = score(puzzle, symetric_indexes);
                        #pragma omp critical
                        history[c] = solutions;
                    }
                    #pragma omp section
                    {
                        int c = score(puzzle, rotate_once_indexes);
                        #pragma omp critical
                        history[c] = solutions;
                    }
                    #pragma omp section
                    {
                        int c = score(puzzle, rotate_twice_indexes);
                        #pragma omp critical
                        history[c] = solutions;
                    }
                }
            }*/
            
            //We undo the last move
            lastMove = solution.back();
            solution.pop_back();
            
            puzzle[lastMove.from] = true;
            puzzle[lastMove.by] = true;
            puzzle[lastMove.i] = false;
            ++current;

            solutions += sol.top();
            sol.pop();

            backtrace = true;
        } else {
            //We searched everything
            break;
        }
    }

    std::cout << "Found " << solutions << " solutions" << std::endl;
}

void display(const std::vector<unsigned int>& puzzle){
    int index = 0;
    for(unsigned int i = 1; i <= levels; ++i){
        for(unsigned int j = 0; j < i; ++j){
            std::cout << puzzle[++index] << " ";
        }

        std::cout << std::endl;
    }
}
