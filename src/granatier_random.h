/*
    Copyright (C) 2018 Mathias Kraus <k.hias@gmx.de>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

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
