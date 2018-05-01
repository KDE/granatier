
#ifndef _GRANATIER_RANDOM_H_
#define _GRANATIER_RANDOM_H_

#include <random>

namespace granatier {

namespace  RNG {
    /**
    * @brief random number generator
    * 
    * @param T: template type
    * @param min: the min value of the range (inclusive)
    * @param max: the max value of the range (inclusive)
    * @return T: the random number
    */
    template <typename T>
    T fromIntRange(T min, T max) {
    static std::random_device randomDevice;
        std::uniform_int_distribution<T> distribution(min, max);
        return distribution(randomDevice);
    }
    
    template <typename T>
    T fromRealRange(T min, T max) {
    static std::random_device randomDevice;
        std::uniform_real_distribution<T> distribution(min, max);
        return distribution(randomDevice);
    }
};

}

#endif // _GRANATIER_RANDOM_H_
