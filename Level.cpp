//
// Created by Danny Aguilar on 3/15/24.
//
#include "PageTable.h"
#include "Level.h"

//we use this constructor for the page table to default and create the first level of the tree
Level::Level(int entries) {
    depth = 0;
    map = nullptr;
    //we specify how many bits need to be allocated and set each entry to nullptr
    nextLevelPtr = new Level*[entries];
    for(int idx = 0; idx < entries; idx++)
    {
        nextLevelPtr[idx] = nullptr;
    }
}

//we use this constructor for all the levels after the first to represent the new depth we are creating this level
Level::Level(int levelSize, int newDepth){
    depth = newDepth;
    map = nullptr;
    nextLevelPtr = new Level*[levelSize];
    //we specify how many bits need to be allocated and set each entry to nullptr
    for(int idx = 0; idx < levelSize; idx++)
    {
        nextLevelPtr[idx] = nullptr;
    }
}


