
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

// PROC_H
#include "proc.h"



// registre adress latch (16 bits)
struct AddrLatch al;

// DataLatch (8 bits) registre servant à la mémoire pour lire et écrire (appelle à Read et Write)
byte DLatch;

// Registres r0 à r7
byte reg[NOMBRE_REGISTRES];

// CS Chip Selector : le registre sélectionné
int CS;

// L'ALU, une variable de 8 bits correspondant au retour de l'éxécution de l'instructon
byte alu;

// Les deux registres d'entrée de l'ALU : X et Y
byte X;
byte Y;

// Les flags de l'ALU : 
// 0000 00cz
// C (carry), Z (zero)
byte flags = 0;

// Registre PC, contient l'adresse de la prochaine instruction, composé en PCH (partie haute) & PCL (partie basse)
struct rPC pc;

// Registre IR, contient le code de l'instruction à éxécuter.
byte ir;

// la RAM, un tableau d'octets (contenant les instructons du programme donné)
byte ram[TAILLE_RAM];

// Le bus de données, contient une donnée (8 bits)
byte dbus;

// Le bus d'adresse, contient une adresse (16 bits)
int16_t abus;


// ***      ***     ***     ***     ***     ***
// MICRO INSTRUCTIONS & INSTRUCTIONS HAUTES ***
// ***      ***     ***     ***     ***     ***

// --- PC ---
// PCout : met l'adresse de la prochaine instruction sur le bus d'adresse.
void PCout() { abus = ((int16_t)pc.PCH << 8); abus += pc.PCL; }

// PCin : met dans le registre PC la valeur contenue sur le bus d'adresse.
void PCin() { pc.PCL = (byte)(abus & 0xFF); pc.PCH = (byte)(abus >> 8); }

// PCLin : met la valeur du bus de données dans la partie basse de PC
void PCLin() { pc.PCL = dbus; }

// PCHin : met la valeur du bus de données dans la partie haute de PC
void PCHin() { pc.PCH = dbus; }


// --- ADRESSE LATCH ---
// ALLin : met la valeur du bus de donnée dans la partie basse d'adress latch
void ALLin() { al.ALL = dbus; }

// ALHin : met la valeur du bus de donnée dans la partie haute d'adress latch
void ALHin() { al.ALH = dbus; }

// ALin : met la valeur du bus d'adresses dans le registre adress latch
void ALin() { al.ALL = (byte)(abus & 0xFF); al.ALH = (byte)(abus >> 8); }

// AAout : calcule l'adresse de l'instruction suivante, en incrémentant l'adresse d'AddrLatch 
// et en plaçant le résultat sur le bus d'adresses.
void AAout() { 
    
    //printf("al.ALH is %02x\n", al.ALH);
    //printf("al.ALL is %02x\n", al.ALL);
    //printf("(int16_t)al.ALH is %04x\n", (int16_t)al.ALH);
    //printf("(int16_t)al.ALL is %04x\n", (int16_t)al.ALL);
    abus = (int16_t)al.ALL + ((int16_t)al.ALH << 8) + 1; 
}


// --- DATALATCH ---
// DLin : met la valeur du bus de données dans le registre DLatch
void DLin() { DLatch = dbus; }

// DLout : met la valeur du registre DLatch dans le bus de données
void DLout() { dbus = DLatch; }

// --- IR ---
// IRin : met la valeur du bus de donnée dans le registre d'instruction
void IRin() { ir = dbus; }

// --- ALU ---
// Xin : met la valeur du bus de données dans le registre X
void Xin() { X = dbus; }

// Yin : met la valeur du bus de données dans le registre Y
void Yin() { Y = dbus; }

// ALUout : met la sortie de l'ALU dans le bus de données
void ALUout() { dbus = alu; }

// Opérations internes de l'ALU (8 maximums)
// Addition
void addALU() {
    alu = X + Y;

    // Positionnement du flag zero
    if (alu == 0) { 
        flags = flags | 1; // 0000 0001
    }
    else {
        flags = flags & 0xFE; 
    }

    // Positionnement du flag carry
    int r = (int)X + (int)Y;
    if (alu != r) { 
        flags = flags | 2; // 0000 0010
    }
    else {
        flags = flags & 0xFD; // 1111 1101
    }
}
// Soustraction
void subALU() { 
    alu = X - Y;

    // Positionnement du flag zero
    if (alu == 0) { 
        flags = flags | 1; // 0000 0001
    }
    else {
        flags = flags & 0xFE; 
    }

    // Positionnement du flag carry
    int r = (int)X - (int)Y;
    if (alu != r) { 
        flags = flags | 2; // 0000 0010
    }
    else {
        flags = flags & 0xFD; // 1111 1101
    }
}
// Incrémentation
void incALU() { 
    alu = X + 1; 
    
    // Positionnement du flag zero
    if (alu == 0) { 
        flags = flags | 1; // 0000 0001
    }
    else {
        flags = flags & 0xFE; 
    }

    // Positionnement du flag carry
    int r = (int)X + 1;
    if (alu != r) { 
        flags = flags | 2; // 0000 0010
    }
    else {
        flags = flags & 0xFD; // 1111 1101
    }
}
// Decrémentation
void decALU() { 
    alu = X - 1;

    // Positionnement du flag zero
    if (alu == 0) { 
        flags = flags | 1; // 0000 0001
    }
    else {
        flags = flags & 0xFE; 
    }

    // Positionnement du flag carry
    int r = (int)X - 1;
    if (alu != r) { 
        flags = flags | 2; // 0000 0010
    }
    else {
        flags = flags & 0xFD; // 1111 1101
    }
}
// ET logique
void andALU() { 
    alu = X & Y;
    flags = flags & 0xFE; // 1111 1110
    flags = flags & 0xFD; // 1111 1101
}
// NON logique
void notALU() { 
    alu = !X;
    flags = flags & 0xFE; // 1111 1110
    flags = flags & 0xFD; // 1111 1101
}


// --- REGISTRES ---
// SR pour Select Register - Modifie le registre CS (Chips Selector) pour sélectionné un registre parmis R0-R7
void SR(byte n) { CS = n; }

// Rin : met la valeur du bus de données dans le registre Rn ou n est donnée en paramètre
void Rin() { reg[CS] = dbus; }

// Rout : met la valeur du registre Rn ou n est donnée en paramètre dans le bus de données
void Rout() { dbus = reg[CS]; }

// --- RAM ---
void Read() {
    DLatch = ram[(int16_t)al.ALL + (int16_t)(al.ALH << 8)];
}

void Write() {
    ram[(int16_t)al.ALL + (int16_t)(al.ALH << 8)] = DLatch;
}

void WaitMem() {  }


// Chargement de la prochaine instruction pointé par l'adresse PC et calcul de l'adresse suivante
int load_next_inst() {
    // 1 -- Chargement de l'adresse
    PCout();
    //printf("\nPC = %02x %02x \n", pc.PCH, pc.PCL);
    //printf("abus = %04x \n", abus);
    ALin();
    //printf("AL = %02x %02x \n", al.ALH, al.ALL);
    // 2 -- Lecture de la donnée
    Read();
    //printf("DLatch = %02x \n", DLatch);
    WaitMem(); // Attente de la RAM
    // 3 -- Chargement du code instruction dans IR
    DLout();
    IRin();
    //printf("dbus = %02x \n", dbus);
    // 4 -- Chargement de l'adresse suivante dans PC
    AAout();
    //printf("abus suivante = %04x \n", abus);
    PCin();
    //printf("\nPC suivant = %02x %02x \n", pc.PCH, pc.PCL);
    return 0;
}



