/*
    SPDX-FileCopyrightText: 2018 Mathias Kraus <k.hias@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _GRANATIER_RANDOM_H_
#define _GRANATIER_RANDOM_H_

#include <random>

namespace granatier {
namespace  RNG {

/**
* @brief random number generator
* 
* @param min: the min value of the range (inclusive)
* @param max: the max value of the range (inclusive)
* @return T: the random number
*/
template <typename T>
T fromRange(T min, T max, typename std::enable_if<std::is_integral<T>::value >::type* = nullptr) {
static std::random_device randomDevice;
    std::uniform_int_distribution<T> distribution(min, max);
    return distribution(randomDevice);
}

/**
* @brief random number generator
* 
* @param min: the min value of the range (inclusive)
* @param max: the max value of the range (inclusive)
* @return T: the random number
*/
template <typename T>
T fromRange(T min, T max, typename std::enable_if<std::is_floating_point<T>::value >::type* = nullptr) {
static std::random_device randomDevice;
    std::uniform_real_distribution<T> distribution(min, max);
    return distribution(randomDevice);
}

} // namespace RNG
} // namespace granatier

#endif // _GRANATIER_RANDOM_H_
