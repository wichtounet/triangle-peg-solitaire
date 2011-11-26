#include <iostream>

int main(int argc, const char* argv[]) {
    if(argc < 2){
        std::cout << "Not enough options provided" << std::endl;
    } else {
        int levels = strtol(argv[1], 0, 10); 

        std::cout << "Generate solutions for a triangular solution with " << levels << " levels" << std::endl;
    }
}
