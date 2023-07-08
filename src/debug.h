


#ifndef DEBUG_H
#define DEBUG_H 

#include <stdint.h>


typedef unsigned char byte;

int readDebugCommand();

void displayHelp();

void displayRam(int16_t from, int16_t to);

void displayRamValue(int16_t adr);

void displayRegisters();

void displayRegister(int num);


void displayAddr();
void displayAbus();
void displayALatch();
void displayPC();

void displayData();
void displayDbus();
void displayIR();
void displayDLatch();

void displayALUPart();
void displayX();
void displayY();
void displayFlags();
void displayALU();

#endif








