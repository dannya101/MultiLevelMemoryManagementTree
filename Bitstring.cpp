//
// Created by Danny Aguilar on 3/20/24.
//

#include "Bitstring.h"

//where we modify the default constructor
//this provides us with the creation of a new bitstring
//virtual time, flag, and address set to default variables to show bitstring has not been manipulated
Bitstring::Bitstring() {
    bitstring = 0b1000000000000000;
    flag = false;
    virtualTime = -1;
    address = 0;

}

void Bitstring::accessBitstring(Bitstring* bitString, bool flagAccess) {
    //here we always shift the bitstring to the right by 1 to when it has been accessed or not recently
    bitString->bitstring >>= 1;

    //this represents that the specific bitstring has been flagged and has been accessed so prepend a 1 to the bitstring
    if(flagAccess){
        bitString->bitstring = (1 << 15) | bitString->bitstring;
    }
}




