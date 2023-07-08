


#ifndef OPERATIONS_H
#define OPERATIONS_H

// Ici on va utiliser le type byte pour des variables d'un octet (0 à 255)
typedef unsigned char byte;

// Codage des opérations pour le décodeur. 
// Une opération à un nom, un codage binaire (valeur, et taille en nombre de bits), une opérande 1 et une opérande 2
// Une opérande est :
// peut-être soit de type constante (3 bits pour un registre) ou de type adresse (sur 16 bits) ou de type non-définie ('n').
#define NOM_OP_SIZE 3

typedef struct operations {
    char nom[NOM_OP_SIZE + 1]; // +1 pour '\0'
    char code_value;
    char code_num_bits;
    char opr1_type; // 'c' ou 'a' ou 'n'
    char opr2_type; // 'c' ou 'a' ou 'n'
    void (*fOP)(); // La fonction de l'opération appelée lors de l'éxécution
} operation;


extern operation NOP;
extern operation JUMP;
extern operation JUMPZERO;
extern operation JUMPCARRY;
extern operation STORE;
extern operation STOREX;
extern operation LOAD;
extern operation LOADX;
extern operation MOVE;
extern operation MOVEREG;
extern operation DECREMENT;
extern operation INCREMENT;
extern operation NOT;
extern operation ADD;
extern operation SUB;
extern operation AND;
extern operation SWP;

extern int NUM_OPS;
extern operation *OPS[];

// Décode une opération selon le code instruction (sur 8 bits max) fourni.
// ir_code  : un octet contenant un code d'une instruction
// ld_op    : un objet operation qui va contenir l'instruction décodée.
// return   :   0 -> pas d'instruction trouvée
//              1 -> instruction trouvée et chargée dans ld_op 
//              -1 > une erreur est survenue 
int decode(byte ir_code, operation *ld_op, char verbose);

// ***      ***     *** *
// FONCTIONS OPERATIONS *
// ***      ***     *** *

void fNOP();
void fJUMP();
void fJUMPZERO();
void fJUMPCARRY();
void fSTORE();
void fSTOREX();
void fLOAD();
void fLOADX();
void fMOVE();
void fMOVEREG();
void fDECREMENT();
void fINCREMENT();
void fNOT();
void fADD();
void fSUB();
void fAND();
void fSWP();


// Pour récupérer les instructions assembleur dans un fichier il nous faut un file_descriptor
extern int fd_output_assm;


#endif


