#include <iostream>

#include "map.cpp"

int main(int, const char*[]) {
    if(!Initialize()){
        std::cout << "Unable to initialize" << std::endl;
        return 1;
    }

    for(int i = 0; i < 10000; ++i){
        Set(i, 2*i);
    }

    std::cout << Get(1234) << std::endl;
    std::cout << Get(1222) << std::endl;
    std::cout << Get(2221) << std::endl;
    std::cout << Get(1111) << std::endl;
}
