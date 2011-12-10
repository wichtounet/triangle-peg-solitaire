#include <iostream>
#include <iterator>
#include <vector>
#include <stack>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

#include <omp.h>

struct Move {
    unsigned int i;//hole
    unsigned int j;//intos[hole][j]
    unsigned int from;
    unsigned int by;
};

struct StartingPosition {
    std::vector<unsigned int> puzzle;
    Move move;
};

const unsigned int THREADS = 16;
const unsigned int STARTING = 128;

unsigned int levels;
unsigned long globalSolutions;

StartingPosition startingPositions[STARTING];

std::vector<unsigned int> normal_indexes;
std::vector<unsigned int> symetric_indexes;
std::vector<unsigned int> rotate_once_indexes;
std::vector<unsigned int> rotate_twice_indexes;

std::vector<std::vector<Move>> intos;

std::unordered_map<unsigned int, unsigned long> history;

void precalculate();

void solve(int hole);
void solveMP(int hole);
void display(const std::vector<unsigned int>& puzzle);

int main(int argc, const char* argv[]) {
    if(argc < 3){
        std::cout << "Not enough options provided" << std::endl;
    } else {
        levels = strtol(argv[1], 0, 10); 
        int hole = strtol(argv[2], 0, 10); 

        //Precalculattions needed by both versions
        precalculate();

        solveMP(hole);
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

void precalculate(){
    normal_indexes.assign(((levels + 1) * levels) / 2 + 1, 0);
    generate_normal_indexes(normal_indexes);

    symetric_indexes.assign(((levels + 1) * levels) / 2 + 1, 0);
    generate_symetric_indexes(symetric_indexes);
    
    rotate_once_indexes.assign(((levels + 1) * levels) / 2 + 1, 0);
    generate_rotate_once_indexes(rotate_once_indexes);
    
    rotate_twice_indexes.assign(((levels + 1) * levels) / 2 + 1, 0);
    generate_rotate_twice_indexes(rotate_twice_indexes);
    
    intos.assign(((levels + 1) * levels) / 2 + 1, std::vector<Move>());
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
}

void computeSolutions(unsigned int i){
    StartingPosition& position = startingPositions[i];

    std::vector<unsigned int> puzzle = position.puzzle;

    if(puzzle.empty()){
        return;
    }
    
    Move& firstMove = position.move;
    puzzle[firstMove.from] = false;
    puzzle[firstMove.by] = false;
    puzzle[firstMove.i] = true;

    unsigned long solutions = 0;
    std::vector<Move> solution;

    bool backtrace = false;
    bool restart = false;

    Move lastMove;

    std::stack<unsigned long> sol;

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
                      
                        bool found = false;
                        unsigned long recovered = 0;
                            
                        int normal_score = score(puzzle, normal_indexes);
                        int symetric_score = score(puzzle, symetric_indexes);
                        int rotate_once_score = score(puzzle, rotate_once_indexes);
                        int rotate_twice_score = score(puzzle, rotate_twice_indexes);
                       
                        #pragma omp critical
                        {
                            if(history.find(normal_score) != history.end()){
                                recovered = history[normal_score];
                                found = true;
                            } else if(history.find(symetric_score) != history.end()){
                                recovered = history[symetric_score];
                                found = true;
                            } else if(history.find(rotate_once_score) != history.end()){
                                recovered = history[rotate_once_score];
                                found = true;
                            } else if(history.find(rotate_twice_score) != history.end()){
                                recovered = history[rotate_twice_score];
                                found = true;
                            }
                        }
                        
                        if(found){
                            solutions += recovered;
                            puzzle[move.from] = true;
                            puzzle[move.by] = true;
                            puzzle[i] = false;
                            continue;
                        }

                        //If the subtree has not already been computed, we compute it
                        sol.push(solutions);
                        solutions = 0;
                       
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
        
        solutions += win(puzzle);
       
        //There is no more moves 
        if(!solution.empty()){
            int normal_score = score(puzzle, normal_indexes);
            int symetric_score = score(puzzle, symetric_indexes);
            int rotate_once_score = score(puzzle, rotate_once_indexes);
            int rotate_twice_score = score(puzzle, rotate_twice_indexes);

            #pragma omp critical
            {
                history[normal_score] = solutions;
                history[symetric_score] = solutions;
                history[rotate_once_score] = solutions;
                history[rotate_twice_score] = solutions;
            }

            //We undo the last move
            lastMove = solution.back();
            solution.pop_back();
            
            puzzle[lastMove.from] = true;
            puzzle[lastMove.by] = true;
            puzzle[lastMove.i] = false;

            solutions += sol.top();
            sol.pop();

            backtrace = true;
        } else {
            //We searched everything
            break;
        }
    }

    #pragma omp flush(globalSolutions)

    #pragma omp atomic
    globalSolutions += solutions;
}

void generateStartingPositions(int hole){
    std::vector<unsigned int> puzzle(((levels + 1) * levels) / 2 + 1, true);
    puzzle[hole] = false;
    
    unsigned int current = 0;
    
    //Generate starting positions for the starting hole    
    for(unsigned int j = 0; j < intos[hole].size(); ++j){
        Move& move = intos[hole][j];

        //A potential move is found
        if(puzzle[move.from] && puzzle[move.by]){
            startingPositions[current] = {puzzle, move};
            ++current;
        }
    }
   
    //Then we explode the starting positions
    while(true){
        int edited = false;

        unsigned int end = current;
        for(unsigned int i = 0; i < end; ++i){
            StartingPosition& position = startingPositions[i];
            std::vector<unsigned int> tempPuzzle(position.puzzle);
            
            tempPuzzle[position.move.from] = false;
            tempPuzzle[position.move.by] = false;
            tempPuzzle[position.move.i] = true;

            unsigned int count = 0;

            for(unsigned int h = 1; h < tempPuzzle.size(); ++h){
                if(!tempPuzzle[h]){
                    for(unsigned int j = 0; j < intos[h].size(); ++j){
                        Move& move = intos[h][j];

                        //A potential move is found
                        if(tempPuzzle[move.from] && tempPuzzle[move.by]){
                            ++count;
                        }
                    }
                }
            }
            
            //If we have enough places to handle this explosion
            if(current + count - 1 < STARTING){
                bool first = true;
                for(unsigned int h = 1; h < tempPuzzle.size(); ++h){
                    if(!tempPuzzle[h]){
                        for(unsigned int j = 0; j < intos[h].size(); ++j){
                            Move& move = intos[h][j];

                            //A potential move is found
                            if(tempPuzzle[move.from] && tempPuzzle[move.by]){
                                if(first){
                                    startingPositions[i] = {tempPuzzle, move};
                                    first = false;
                                } else {
                                    startingPositions[current] = {tempPuzzle, move};
                                    ++current;
                                }
                            }
                        }
                    }
                }

                edited = true;
            } 
        }

        //If there are less than 2 solutions to generate
        if(!edited || current >= STARTING - 2){
            break;
        }
    }
   
    std::cout << current << " starting positions have been generated" << std::endl;
    
    for(; current < STARTING; ++current){
        startingPositions[current] = {std::vector<unsigned int>(0), Move()};
    }
}

void solveMP(int hole){
    std::cout << "Generate solutions for a triangular solitaire with " << levels << " levels" << std::endl;
    std::cout << "With hole at position " << hole << std::endl;
    std::cout << "With " << THREADS << " threads" << std::endl;
    std::cout << "With " << STARTING << " solutions" << std::endl;
    
    generateStartingPositions(hole);
  
    #pragma omp parallel num_threads(THREADS) shared(globalSolutions) shared(startingPositions)
    {
        #pragma omp for nowait schedule(static, 1)
        for(unsigned int i = 0; i < STARTING; ++i){
            computeSolutions(i);
        }

        printf("%i finished \n", omp_get_thread_num());
    }

    std::cout << "Solutions " << globalSolutions << std::endl;
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
