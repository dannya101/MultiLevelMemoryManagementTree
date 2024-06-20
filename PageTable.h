//
// Created by Danny Aguilar on 3/13/24.
//

#ifndef ASSIGNMENT3_PAGETABLE_H
#define ASSIGNMENT3_PAGETABLE_H
#include "Level.h"
#include "Bitstring.h"
#include <vector>

using namespace std;
class PageTable {
public:
    //creating all the attributes that are going to be stored in the class PageTable
    int levelCount;
    unsigned int *bitmask;
    int *shiftAry;
    int *entryCount;
    int offsetSize;
    Level* rootNodePtr;
    unsigned int offset;
    unsigned int* vpnReplacing;
    unsigned int* victimBitstring;

    //this is the constructor we use to declare a page table object
    PageTable(int totalLevels, unsigned int *bitMaskArr, int *shiftArray, int *entriesArr);

    //these are the provided methods that we are required to implement
    unsigned int extractVPNFromVirtualAddress(unsigned int virtualAddress, unsigned int mask, unsigned int shift);
    int searchMappedPfn(Level* levelPtr, unsigned int virtualAddress, int treeDepth);
    bool insertMapForVpn2Pfn(Level *levelPtr, unsigned int virtualAddress, int frame, int treeDepth,
                             int counter, int intervalSize, Bitstring** bitstrigArr, unsigned int* pageHits, unsigned long* bytes_used, int totalFrames);
    //new function we created to simulate the paging replacement
    int pageReplacement(Level* levelPtr, Bitstring** bitstringArr, unsigned int* vpnReplacing, unsigned int* victimBitstring, int totalFrames);

};
#endif //ASSIGNMENT3_PAGETABLE_H
