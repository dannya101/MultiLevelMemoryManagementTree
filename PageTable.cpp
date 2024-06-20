//
// Created by Danny Aguilar on 3/13/24.
//
#include "PageTable.h"
#include "Level.h"
#include "Bitstring.h"

using namespace std;

PageTable::PageTable(int totalLevels, unsigned int *bitMaskArr, int *shiftArray, int *entriesArr) {
    //setting the corresponding parameters to their specific attribute in pageTable
    this->levelCount = totalLevels;
    this->bitmask = bitMaskArr;
    this->shiftAry = shiftArray;
    this->entryCount = entriesArr;
    //also taking into account the offset size which is the last value of the shift array
    this->offsetSize = shiftAry[levelCount - 1];
    //we are also creating an instance of Level in the rootNodePtr to show when you create a page table root node on default it generates a level pointer array simulating the array of the first level
    this->rootNodePtr = new Level(entriesArr[0]);
    //this is where we properly instantiate variables
    this->offset = 0;
    this->victimBitstring = nullptr;
    this->vpnReplacing = nullptr;
}

//this is where we extract the virtual address to part of the VPN
unsigned int PageTable::extractVPNFromVirtualAddress(unsigned int virtualAddress, unsigned int mask, unsigned int shift) {
    //we create the page variable by taking the specific mask specified by the level and 'anding' is with the virtual address
    //then shift the result to get the VPN
    unsigned int page = virtualAddress & mask;
    page = page >> shift;
    return page;
}

int PageTable::searchMappedPfn(Level* levelPtr, unsigned int virtualAddress, int treeDepth){
    //here we create an instance of Pagetable to reference from and we specify which vpn needs to be extracted from the virtual address
    PageTable* holdRequirements = this;
    unsigned int theVA = extractVPNFromVirtualAddress(virtualAddress, holdRequirements->bitmask[treeDepth],
                                                      holdRequirements->shiftAry[treeDepth]);
    //we then check if the tree depth enters the leaf node level we return the frame number that is stored in the map array of the leaf node
    if (treeDepth == holdRequirements->levelCount - 1) {
        return levelPtr->map[theVA];
    }
    //this is where we recursively go through the tree going through each specific node according to the VPN
    return searchMappedPfn(levelPtr->nextLevelPtr[theVA], virtualAddress, treeDepth + 1);
}

bool PageTable::insertMapForVpn2Pfn(Level *levelPtr, unsigned int virtualAddress, int frame, int treeDepth,
                                    int counter, int intervalSize, Bitstring** bitstringArr, unsigned int* pageHits, unsigned long* bytes_used, int totalFrames) {
    //here we create an instance of Pagetable to reference from and we specify which vpn needs to be extracted from the virtual address
    PageTable* holdRequirements = this;
    unsigned int theVA = extractVPNFromVirtualAddress(virtualAddress,holdRequirements->bitmask[treeDepth],
                                                      holdRequirements->shiftAry[treeDepth]);

    //we then check if the tree depth enters the leaf node level
    if(treeDepth == holdRequirements->levelCount - 1) {
        //then creating a map only if the map does not exist already in the leaf node
        //setting all positions in map to invalid and keeping note of bytes used
        //also after we create the desired map in the leaf node we set the frame accordingly
        //and create a new bitstring to represent the newly found pfn
        if (levelPtr->map == nullptr) {
            levelPtr->map = new int[holdRequirements->entryCount[levelPtr->depth]];
            for (int idx = 0; idx < holdRequirements->entryCount[levelPtr->depth]; idx++) {
                levelPtr->map[idx] = -1;
                *bytes_used += sizeof(levelPtr->map[idx]);
            }
            levelPtr->map[theVA] = frame;
            *bytes_used += sizeof(levelPtr->map[theVA]);
            Bitstring *newBit = new Bitstring();
            newBit->address = virtualAddress;
            //counter is keeping track of the virtual time of the program and if it reaches the specified interval size
            //this is where we update the bitstrings
            if(counter % intervalSize == 0){
                //loop through bitstring array
                //shift all bitstring right by 1
                //prepend a 1 if flag is true
                int idx = 0;
                while(idx < totalFrames && bitstringArr[idx] != nullptr){
                    bitstringArr[idx]->accessBitstring(bitstringArr[idx], bitstringArr[idx]->flag);
                    bitstringArr[idx]->flag = false;
                    idx++;
                }
            }
            //once the bitstrings are updated this is where we will load in the next page
            bitstringArr[frame] = newBit;
            bitstringArr[frame]->flag = true;
            //if the page was created during the interval update it does not count as being accessed
            if(counter % intervalSize == 0){
                bitstringArr[frame]->flag = false;
            }
            return false;
        }
            //go in this if statement if the map has been created already though the frame number does not exist
        else if(levelPtr->map[theVA] == -1){
            //we set the new frame number and update bytes used accordingly
            //then we follow the same procedure of creating a new bitstring
            levelPtr->map[theVA] = frame;
            *bytes_used += sizeof(levelPtr->map[theVA]);
            Bitstring *newBit = new Bitstring();
            newBit->address = virtualAddress;
            //if it reaches the interval size update the bitstrings accordingly if they were accesed during the interval or not
            if(counter % intervalSize == 0){
                //loop through bitstring array
                //shift all bitstring right by 1
                //prepend a 1 if flag is true
                int idx = 0;
                while(idx < totalFrames && bitstringArr[idx] != nullptr){
                    bitstringArr[idx]->accessBitstring(bitstringArr[idx], bitstringArr[idx]->flag);
                    bitstringArr[idx]->flag = false;
                    idx++;
                }
            }
            //once we update the bitstrings we load the new bitstring into the bitstring array
            bitstringArr[frame] = newBit;
            bitstringArr[frame]->flag = true;
            //if the bitstring was created when the update interval occurred do not prepend a 1 as it has not been accessed in the next interval
            if(counter % intervalSize == 0){
                bitstringArr[frame]->flag = false;
            }
            return false;
        }
        else {
            //if you access an address that has already been created go into here
            int idx = 0;
            //take note of the updated virtual address we are passing in the function and increment the page hits variable to show we have mapped to the same frame variable
            unsigned int newUpdatedAddress = virtualAddress >> this->offsetSize;
            *pageHits = *pageHits + 1;
            //here we are finding which bitstring corresponds to the new address being inputted
            //if we find the specific bitstring represented by the address we show that it has been accessed and flag the bitstring as true to later prepend a 1 in updating
            while(idx < totalFrames && bitstringArr[idx] != nullptr){
                unsigned int otherUpdatedAddress = bitstringArr[idx]->address >> this->offsetSize;
                if(newUpdatedAddress == otherUpdatedAddress){
                    bitstringArr[idx]->flag = true;
                    break;
                }
                idx++;
            }
            //we also check if it time to update our bitstrings
            if(counter % intervalSize == 0){
                //loop through bitstring array
                //shift all bitstring right by 1
                //prepend a 1 if flag is true
                int idx = 0;
                while(idx < totalFrames && bitstringArr[idx] != nullptr){
                    bitstringArr[idx]->accessBitstring(bitstringArr[idx], bitstringArr[idx]->flag);
                    bitstringArr[idx]->flag = false;
                    idx++;
                }
            }
            return true;
        }
    }
    //if the nextLevelPtr in the array has no value in specified position create a new level pointer in array
    if(levelPtr->nextLevelPtr[theVA] == nullptr) {
        //we create a new level and set it to its corresponding VPN in the tree
        // we also take note of the bytes used and add it to our total
        Level* newNode = new Level(holdRequirements->entryCount[treeDepth + 1], treeDepth + 1);
        levelPtr->nextLevelPtr[theVA] = newNode;
        *bytes_used += sizeof(*levelPtr->nextLevelPtr[theVA]);
    }
    //here is where we are recursively calling through our tree to get to the next level
    return insertMapForVpn2Pfn(levelPtr->nextLevelPtr[theVA], virtualAddress, frame, treeDepth + 1,
                               counter, intervalSize, bitstringArr, pageHits, bytes_used, totalFrames);
}

int PageTable::pageReplacement(Level *levelPtr, Bitstring** bitstringArr, unsigned int* vpnReplacing, unsigned int* victimBitstring, int totalFrames) {
    //here is where we set automatically set the minimum to the first bitstring in the array saying this is the smallest bitstring
    //we keep note of its frame number and address
    Bitstring* tempMinimum = bitstringArr[0];
    int victimFrame = 0;
    unsigned int changeOldAddress = tempMinimum->address;
    int idx = 0;
    //where we loop through the bitstring array if the bitstring is either equal or less than the current smallest bitstring than go through if statements
    while(idx < totalFrames && bitstringArr[idx] != nullptr){
        if(bitstringArr[idx]->bitstring <= tempMinimum->bitstring)
        {
            if(bitstringArr[idx]->bitstring == tempMinimum->bitstring)
            {
                //if this if statement is executed we know that the bitstrings are equal and we need to compare their virtual time
                //we then check who has the smaller virtual time to see which bitstring is the new smallest
                if(bitstringArr[idx]->virtualTime < tempMinimum->virtualTime)
                {
                    tempMinimum= bitstringArr[idx];
                    changeOldAddress = bitstringArr[idx]->address;
                    victimFrame = idx;
                }
            }
            //if they are not equal then the new bitstring interated too is the smallest so update the new smallest bitstring
            else{
                tempMinimum = bitstringArr[idx];
                changeOldAddress = bitstringArr[idx]->address;
                victimFrame = idx;
            }

        }
        idx++;
    }
    PageTable* holdRequirements = this;
    *vpnReplacing = changeOldAddress >> this->offsetSize;
    *victimBitstring = tempMinimum->bitstring;
    //here is where we loop through the tree again iteratively to go through to the original frame number we are replacing
    //once we reach the corresponding address in the map then we invalidate the frame number
    for(int idx = 0; idx < this->levelCount; idx++){
        unsigned int theVA = extractVPNFromVirtualAddress(changeOldAddress,holdRequirements->bitmask[idx], holdRequirements->shiftAry[idx]);
        if(levelPtr->map != nullptr)
        {
            levelPtr->map[theVA] = -1;
            break;
        }
        levelPtr = levelPtr->nextLevelPtr[theVA];
    }
    //we return the victim frame that is going to be replaced next here
    return victimFrame;
}
