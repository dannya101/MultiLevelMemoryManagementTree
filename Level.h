//
// Created by Danny Aguilar on 3/15/24.
//

#ifndef ASSIGNMENT3_LEVEL_H
#define ASSIGNMENT3_LEVEL_H

#include "PageTable.h"

class Level {
    //we are setting the attributes in this level class
public:
    int depth;
    Level** nextLevelPtr;
    int* map;

    //here is where we create our level constructors
    Level(int entries);
    Level(int levelSize, int newDepth);


};


#endif //ASSIGNMENT3_LEVEL_H
