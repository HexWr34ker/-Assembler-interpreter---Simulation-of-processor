


#ifndef PROC_H
#define PROC_H


#include <stdint.h>

// Ici on va utiliser le type byte pour des variables d'un octet (0 à 255)
typedef unsigned char byte;

#define NOMBRE_REGISTRES 8
#define TAILLE_RAM 2048

// Registre PC, contient l'adresse de la prochaine instruction, composé en PCH (partie haute) & PCL (partie basse)
typedef struct rPC {

    byte PCH;
    byte PCL;

} rPC;

// Registre adress latch (16 bits) contient une adresse, ALH (partie haute) & ALL (partie basse)
struct AddrLatch {

    byte ALH;
    byte ALL;

}typedef AddrLatch;

// registre adress latch (16 bits)
extern struct AddrLatch al;

// DataLatch (8 bits) registre servant à la mémoire pour lire et écrire (appelle à Read et Write)
extern byte DLatch;

// Registres r0 à r7
extern byte reg[NOMBRE_REGISTRES];

// CS Chip Selector : le registre sélectionné
extern int CS;

// L'ALU, une variable de 8 bits correspondant au retour de l'éxécution de l'instructon
extern byte alu;

// Les deux registres d'entrée de l'ALU : X et Y
extern byte X;
extern byte Y;

// Les flags de l'ALU : 
// 0000 00cz
// C (carry), Z (zero)
extern byte flags;

// Registre PC, contient l'adresse de la prochaine instruction, composé en PCH (partie haute) & PCL (partie basse)
extern struct rPC pc;

// Registre IR, contient le code de l'instruction à éxécuter.
extern byte ir;

// la RAM, un tableau d'octets (contenant les instructons du programme donné)
extern byte ram[TAILLE_RAM];

// Le bus de données, contient une donnée (8 bits)
extern byte dbus;

// Le bus d'adresse, contient une adresse (16 bits)
extern int16_t abus;



// SIGNAUX & MICRO INSTRUCTIONS

// --- PC ---
// PCout : met l'adresse de la prochaine instruction sur le bus d'adresse.
void PCout();

// PCin : met dans le registre PC la valeur contenue sur le bus d'adresse.
void PCin();

// PCLin : met la valeur du bus de données dans la partie basse de PC
void PCLin();

// PCHin : met la valeur du bus de données dans la partie haute de PC
void PCHin();


// --- ADRESSE LATCH ---
// ALLin : met la valeur du bus d'adresses dans la partie basse d'adress latch
void ALLin();

// ALHin : met la valeur du bus d'adresses dans la partie haute d'adress latch
void ALHin();

// ALin : met la valeur du bus d'adresses dans le registre adress latch
void ALin();

// AAout : calcule l'adresse de l'instruction suivante, en incrémentant l'adresse d'AddrLatch 
// et en plaçant le résultat sur le bus d'adresses.
void AAout();


// --- DATALATCH ---
// DLin : met la valeur du bus de données dans le registre DLatch
void DLin();

// DLout : met la valeur du registre DLatch dans le bus de données
void DLout();

// --- IR ---
// IRin : met la valeur du bus de donnée dans le registre d'instruction
void IRin();

// --- ALU ---
// Xin : met la valeur du bus de données dans le registre X
void Xin();

// Yin : met la valeur du bus de données dans le registre Y
void Yin();

// ALUout : met la sortie de l'ALU dans le bus de données
void ALUout();
// Opérations internes de l'ALU (8 maximums)
// Addition
void addALU();
// Soustraction
void subALU();
// Incrémentation
void incALU();
// Decrémentation
void decALU();
// ET logique
void andALU();
// NON logique
void notALU();


// --- REGISTRES ---
// SR pour Select Register - Modifie le registre CS (Chips Selector) pour sélectionné un registre parmis R0-R7
void SR(byte n);

// Rin : met la valeur du bus de données dans le registre Rn ou n est donnée en paramètre
void Rin();

// Rout : met la valeur du registre Rn ou n est donnée en paramètre dans le bus de données
void Rout();

// --- RAM ---
void Read();

void Write();

void WaitMem();

// Chargement de la prochaine instruction pointé par l'adresse PC et calcul de l'adresse suivante
int load_next_inst();

// Chargement de l'opérande selon la structure operande.
// void load_operand(operation *op);


#endif





