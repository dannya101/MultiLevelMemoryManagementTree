#include <iostream>
#include "vaddr_tracereader.h"
#include <cmath>
#include "log_helpers.h"
#include "PageTable.h"
#include "Bitstring.h"
#include <unistd.h>
#include <string>


using namespace std;

int main(int argc, char **argv) {
    FILE *ifp;            /* trace file */
    unsigned long i = 0;  /* instructions processed */
    p2AddrTr trace;    /* traced address */
    char* file = nullptr; // setting file char pointer to nullptr so we do not generate a garbage value
    int totalVirtualAddresses = -1; //set to -1 as if not set by user than default is to run all vitualAddresses
    int totalFrames = 999999; //specifying default total frames which is 999999
    int bitIntervalSize = 10; //specifying default bit interval size which is 10
    string strategy = "summary"; //specifying default strategy which is summary
    int Option = -1;
    /*If the option has an argument, optarg is set to point to the
    argument associated with the option, here we are looping to retrieve the optional arguments in the command line. */
    while ((Option = getopt(argc, argv, "n:f:b:l:")) != -1) {
        switch (Option) {
            case 'n':
                //check if total virtual addresses are valid
                if(atoi(optarg) < 1){
                    cout << "Number of memory accesses must be a number and greater than 0" << endl;
                    exit(1);
                }
                totalVirtualAddresses = atoi(optarg); //optarg points to whatever follows the char 'n'
                break;
            case 'f':
                //check if a valid max total frame have been inputted
                if(atoi(optarg) < 1){
                    cout << "Number of available frames must be a number and greater than 0" << endl;
                    exit(1);
                }
                totalFrames = atoi(optarg); //optarg points to whatever follows the char 'f'
                break;
            case 'b':
                //check if a valid bitstring update has been passed
                if(atoi(optarg) < 1){
                    cout << "Bit string update interval must be a number and greater than 0" << endl;
                    exit(1);
                }
                bitIntervalSize = atoi(optarg); //optarg points to whatever follows the char 'b'
                break;
            case 'l':
                strategy = optarg; //optarg points to whatever follows the char 'l'
                break;
            default:
                exit(1);
        }
    }
    // here we are checking and retrieving the first mandatory argument specified in the command line which is the file
    int idx = optind;
    file = argv[idx];
    int *inputLevels = new int[argc - 1];
    idx++;
    int idxInLvls = 0;
    //we then are goingto loop on idx starting from 1 to argc to retrieve the rest of the mandatory arguments
    while (idx < argc) {
        //once the final level has been retrieved it will loop again and enter the catch statement
        try{
            inputLevels[idxInLvls] = stoi(argv[idx]);
            idxInLvls++;
            idx++;
        }
        catch(...){
            break;
        }
    }
    int length = idxInLvls;
    int totalSum = 0;
    //loop on each level in our pointer to check if each level is valid while also keep tracking of the sum
    for(int iteration = 0; iteration < length; iteration++){
        if(inputLevels[iteration] < 1){
            cout << "Level " << iteration << " page table must have at least 1 bit" << endl;
            exit(1);
        }
        totalSum += inputLevels[iteration];
    }
    //we then check for a valid sum
    if(totalSum > 28){
        cout << "Too many bits used for the page table" << endl;
        exit(1);
    }
    /* attempt to open trace file */
    if ((ifp = fopen(file, "rb")) == NULL) {
        fprintf(stderr, "Unable to open <<%s>>\n", file);
        exit(1);
    }
    int *entryCount = new int[length];
    int *shiftArray = new int[length];
    unsigned int *bitMaskArray = new unsigned int[length];
    unsigned mask = -1;
    //mask array
    //here we are creating the bitmask arrays from a default 32 bit address space
    // we first get the 2 to the power of levels - 1 to get the mask
    //we do that for every level creating a tailored mask for each level
    int dfaultBits = 32;
    for (int idx = 0; idx < length; idx++) {
        mask = pow(2, inputLevels[idx]) - 1;
        dfaultBits = dfaultBits - inputLevels[idx];
        mask = mask << dfaultBits;
        bitMaskArray[idx] = mask;
    }
    //Shift Array
    //we create the shift array as well with each level with the same 32 bit default value
    //update how many bits we need to shift by subtracting the level value from original value
    dfaultBits = 32;
    int adjustBits = dfaultBits - inputLevels[0];
    shiftArray[0] = adjustBits;
    for(int idx = 0; idx < length; idx++){
        //no need to adjust in first index as shift amount is already calculated
        if(idx == 0){
            shiftArray[idx] = adjustBits;
            continue;
        }
        adjustBits -= inputLevels[idx];
        shiftArray[idx] = adjustBits;
    }
    //then we fill in the entry count array which would hold total entries in each level
    for(int idx = 0; idx < length; idx++) {
        entryCount[idx] = pow(2, inputLevels[idx]);
    }
    int iter = 0;
    int frame = 0;
    int holdFrameVal = 0;
    //here we are creating our base root node that will hold all our created arrays tailored for each level
    PageTable* root = new PageTable(length, bitMaskArray, shiftArray, entryCount);
    //we also create a bitstring array to hold the bitstrings of each address of how many times it has been accessed or not
    Bitstring** bitstringArr = new Bitstring * [totalFrames];
    //we set each part of the double pointer array to nullptr to avoid improper initialization
    for(int idx = 0; idx < totalFrames; idx++)
    {
        bitstringArr[idx] = nullptr;
    }
    int counter = 1;
    //each of these pointers will be going through each specific function to either track the victim bitstring from the page replacement method to return to the print statements
    //the vpn replacing pointer also gets the vpn we are replacing in page replacement to pass through the print statements
    //the page hits will track how many pages were hit in total in our tree
    //the bytes used pointer will track how many bytes were created when building our tree
    unsigned int victimBitstrAddress = -1;
    unsigned int vpnReplacingAddress = -1;
    unsigned int* vpnReplacing = &vpnReplacingAddress;
    unsigned int* victimBitstring = &victimBitstrAddress;
    unsigned int pageHitAddress = 0;
    unsigned int* pageHits = &pageHitAddress;
    unsigned long addressOfBytesUsed = 0;
    unsigned long* bytesUsed = &addressOfBytesUsed;

    //this will count how many replacements are needed to fit all addresses
    int countReplacements = -1;

    /* get next address and process */
    while(NextAddress(ifp, &trace)) {
        //counting how many addresses we read in total
        if(iter == totalVirtualAddresses) {
            break;
        }
        i++;
        if ((i % 100000) == 0) {
            fprintf(stderr, "%dK samples processed\r", i / 100000);
        }
        //this is going to run when the maximimum amount of frames are allowed in the pagetable tree, and we need to use page replacement to replace frames
        //using according to their bitstring
        if(holdFrameVal >= totalFrames){
            frame = root->pageReplacement(root->rootNodePtr, bitstringArr, vpnReplacing, victimBitstring, totalFrames);
            //countReplacements++;
        }
        //this is when we are going to try to insert the new address being read to the tree
        //we also call this after page replacement is needed to be able to insert a new address with their new frame number
        bool insertResult = root->insertMapForVpn2Pfn(root->rootNodePtr,trace.addr, frame,0, counter,
                                                      bitIntervalSize, bitstringArr, pageHits,
                                                      bytesUsed, totalFrames);

        //if something was inserted into the tree the insert map will create a new bitstring and run this if statement
        //update the new frame number and frame value to check if total frames have been surpassed
        if(!insertResult)
        {
            //if statement will only run if there is a new node inserted into the tree
            //if not we know the node has already been inserted
            //if the frame values have been become greater than the total frames count how many replacements done
            if(holdFrameVal >= totalFrames){
                countReplacements++;
            }
            frame++;
            holdFrameVal++;
        }
        //here we call searchMappedPFN on the specific address to see look for this addresses bitstring in the bitstring array and update its new virtual time
        bitstringArr[root->searchMappedPfn(root->rootNodePtr,trace.addr, 0)]->virtualTime = counter;
        int vpnReplaced = *vpnReplacing;
        unsigned int victim_Bistring = *victimBitstring;
        //calculate the offset from the specific address to be used later
        mask = pow(2, root->offsetSize) - 1;
        root->offset = trace.addr & mask;
        //if strategy has been set to va2pa in command line run specific print functions
        if(strategy == "va2pa"){
            //using the search function we created we build the physical address from the virtual address
            unsigned int pa = ((root->searchMappedPfn(root->rootNodePtr,trace.addr, 0)) << root->offsetSize) + root->offset;
            log_va2pa(trace.addr, pa);
        }
        //if strategy has been set to offset in command line run specific print functions
        else if(strategy == "offset"){
            print_num_inHex(root->offset);
        }
        //if strategy has been set to vpns_pfn in command line run specific print functions
        //where we show the mapping of vpn to pfn
        else if(strategy == "vpns_pfn"){
            unsigned int* allVPNS = new unsigned int[root->levelCount];
            //we store this for loop to provide all the specific vpns according to the level
            for(int index = 0; index < root->levelCount; index++)
            {
                allVPNS[index] = root->extractVPNFromVirtualAddress(trace.addr, root->bitmask[index], root->shiftAry[index]);
            }
            log_vpns_pfn(root->levelCount, allVPNS, root->searchMappedPfn(root->rootNodePtr,trace.addr, 0));
        }
        //if strategy has been set to vpn2pfn_pr in command line run specific print functions
        else if(strategy == "vpn2pfn_pr")
        {
            //here we are checking that if the insert result hit an address that was already in the page table do not replace
            if(insertResult){
                vpnReplaced = -1;
            }
            log_mapping(trace.addr >> root->offsetSize, root->searchMappedPfn(root->rootNodePtr,trace.addr, 0), vpnReplaced, victim_Bistring, insertResult);
        }
        counter++;
        iter++;
    }
    //if strategy has been set to bitmasks in command line run specific print functions that print the bitmasks of each level
    if(strategy == "bitmasks"){
        log_bitmasks(root->levelCount, root->bitmask);
    }
    //the default strategy is run here if a new strategy was not called

    else if(strategy == "summary"){
        //check if the totalFrames were surpassed if not no page replacement was done so print accordingly
        if(totalFrames > holdFrameVal) {
            log_summary(pow(2, root->offsetSize), 0, *pageHits, iter, holdFrameVal, *bytesUsed);
        }
        //if holdFrameVal is greater than we know page replacement has occurred
        else{
            log_summary(pow(2, root->offsetSize), countReplacements - 1, *pageHits + 2, iter, totalFrames, *bytesUsed);
        }
    }
    /* clean up and return success */
    fclose(ifp);
    return 0;
}









