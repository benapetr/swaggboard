//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//! This file exist only for compiler options that can be changed before you build huggle
//! Please do not commit any changes in this file

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

typedef char byte_ht;

#if defined _WIN64 || defined _WIN32
    #define WIN
#endif

#endif

