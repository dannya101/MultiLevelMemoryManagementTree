//
// Created by Danny Aguilar on 3/20/24.
//

#ifndef ASSIGNMENT3_BITSTRING_H
#define ASSIGNMENT3_BITSTRING_H


class Bitstring {
public:
    //creating all attributes in the Bitstring class that users can access
    unsigned short bitstring;
    int virtualTime;
    unsigned int address;
    bool flag;

    //modifying the default constructor to help us provide the bitstring characteristics
    Bitstring();

    //a method we created to represent the shifting and possible prepending of a bitstring
    void accessBitstring(Bitstring* bitString, bool flag);

};


#endif //ASSIGNMENT3_BITSTRING_H
