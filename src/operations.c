
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "operations.h"
#include "proc.h"

#define _POSIX_C_SOURCE 200809L

/* OPÉRATIONS */

// NOP : NOP : Ne fait rien.
// code_value 0000 0000
// code_num_bits (8 bits)
void fNOP() { /* Rien ne se passe...*/ }
operation NOP = { "NOP", 0x00, 8, 'n', 'n', fNOP };

// Le file descriptor du fichier de sortie des instructions assembleur est par défaut négatif, 
// c-à-d qu'on ne récupère pas les instructions assembleurs.
// Cette variable doit être initialisée par la fonction open().
int fd_output_assm = -1;


// JUMP : JMP HHLL : Effectue un branchement à l'adresse HHLL.
// 0111 0000
// (8 bits)
// HHLL
void fJUMP() { 
    // Remplace l'adresse contenue dans PC par l'adresse passé en argument : HH LL
    // Chargement de l'octet suivant : LL
    PCout();
    ALin();
    Read();
    WaitMem();
    // LL est chargé dans PCL
    DLout();
    PCLin();
    // Chargement du second HH
    AAout();
    ALin();
    Read();
    WaitMem();
    // HH est chargé dans PCH
    DLout();
    PCHin();
    // PC contient, maintenant, HH LL et le chargement de l'instruction suivante se fera à l'adresse HH LL.

    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "JMP %02x%02x\n", pc.PCH, pc.PCL);
}
operation JUMP = { "JMP", 0x70, 8, 'a', 'n', fJUMP };

//////////////////////////
// JUMPZERO : JZ HHLL : Effectue un branchement à l'adresse HHLL si le résultat précèdent est 0.
// 0111 0001
// (8 bits)
// HHLL
void fJUMPZERO() {
    byte h; // pour les logs
    byte l; // pour les logs
    // Résultat précédent est zero ?
    if ( flags & 1 == 1 ) {
        // Remplace l'adresse contenue dans PC par l'adresse passé en argument : HH LL
        // Chargement de l'octet suivant : LL
        PCout();
        ALin();
        Read();
        WaitMem();
        // LL est chargé dans PCH
        DLout();
        PCLin();
        // Chargement du second octet HH
        AAout();
        ALin();
        Read();
        WaitMem();
        // LL est chargé dans PCH
        DLout();
        PCHin();
        // PC contient, maintenant, HH LL et le chargement de l'instruction suivante se fera à l'adresse HH LL.
        h = pc.PCH;
        l = pc.PCL;
    }
    else {
        // Passe les 2 octets suivants
        PCout();
        ALin();
        Read();
        DLout();
        l = dbus;
        AAout();
        ALin();
        Read();
        DLout();
        h = dbus;
        AAout();
        PCin();
    }

    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "JZ %02x%02x\n", h, l);
}
operation JUMPZERO = { "JZ", 0x71, 8, 'a', 'n', fJUMPZERO };

//////////////////////////
// JUMPCARRY : JC HHLL : Effectue un branchement à l'adresse HHLL si le résultat précèdent à engendré une retenue.
// 0111 0010
// (8 bits)
// HHLL
void fJUMPCARRY() {
    byte h; // pour les logs
    byte l; // pour les logs
    // Y a t-il eu une retenue ?
    if ( flags & 2 == 2 ) {
        // Remplace l'adresse contenue dans PC par l'adresse passé en argument : HH LL
        // Chargement de l'octet suivant : LL
        PCout();
        ALin();
        Read();
        WaitMem();
        // LL est chargé dans PCL
        DLout();
        PCLin();
        // Chargement du second octet HH
        AAout();
        ALin();
        Read();
        WaitMem();
        // HH est chargé dans PCH
        DLout();
        PCHin();
        // PC contient, maintenant, HH LL et le chargement de l'instruction suivante se fera à l'adresse HH LL.
        h = pc.PCH;
        l = pc.PCL;
    }
    else {
        // Passe les 2 octets suivants
        PCout();
        ALin();
        Read();
        DLout();
        l = dbus;
        AAout();
        ALin();
        Read();
        DLout();
        h = dbus;
        AAout();
        PCin();
    }

    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "JC %02x%02x\n", h, l);
}
operation JUMPCARRY = { "JC", 0x72, 8, 'a', 'n', fJUMPCARRY };

//////////////////////////
// STORE : ST Rn, HHLL : qui stocke la valeur du registre Rn  en mémoire à l'adresse sur 16 bits.
// 0100 0nnn
// (5 bits)
// Rn
// HHLL
void fSTORE() {
    // Récupération de n
    byte mask = 7; // 0000 0111
    byte n = ir & mask;
    SR(n);
    // Chargement de l'adresse HHLL pour écriture
    // partie HH
    PCout();
    ALin();
    Read();
    WaitMem();
    DLout();
    // partie LL
    AAout();
    ALin();
    Read();
    WaitMem();
    AAout();
    PCin();
    // vers AdressLatch
    ALHin();
    DLout();
    ALLin();
    // Écriture de la valeur de Rn vers HHLL 
    Rout();
    DLin();
    Write();
    WaitMem();

    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "ST R%d %02x%02x\n", n, al.ALH, al.ALL);
}
operation STORE = { "ST", 0x40, 5, 'c', 'a', fSTORE };

//////////////////////////
// STORE_RX : STX R0, RXn : qui stocke la valeur du registre R0 en mémoire à l'adresse contenue dans RXn.
// 0111 10nn
// (6 bits)
// RXn
void fSTOREX() {
    // Récupération de n
    byte mask = 3; // 0000 0011
    byte n = ir & mask;

    // Chargement de la valeur de R0 dans DLatch
    SR(0);
    Rout();
    DLin();
    // Chargement de l'adresse contenue dans RXn vers AddressLatch
    // partie HH
    SR(2 * n);
    Rout();
    ALHin();
    // partie LL
    SR(2 * n + 1);
    Rout();
    ALLin();
    // Écriture
    Write();
    WaitMem();

    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "STX R0 RX%d\n", n);
}
operation STOREX = { "STX", 0x78, 6, 'c', 'n', fSTOREX };



//////////////////////////
// LOAD : LD Rn, HHLL : qui charge dans le registre Rn la valeur stockée en mémoire à l'adresse sur 16 bits.
// 0100 1nnn
// (5 bits)
// Rn
// HHLL
void fLOAD() {

    // Récupération de n
    byte mask = 7; // 0000 0111
    byte n = ir & mask;
    // Chargement de l'adresse HHLL pour lecture
    // partie HH
    PCout();
    ALin();
    Read();
    WaitMem();
    DLout();
    // partie LL
    AAout();
    ALin();
    Read();
    WaitMem();
    AAout();
    PCin();
    // vers AdressLatch
    ALHin();
    DLout();
    ALLin();
    // Lecture de la valeur de HHLL et enregistrement Rn 
    Read();
    WaitMem();
    DLout();
    SR(n);
    Rin();

    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "LD R%d %02x%02x\n", n, al.ALH, al.ALL);
}
operation LOAD = { "LD", 0x48, 5, 'c', 'a', fLOAD };


//////////////////////////
// LOADX : LDX R0, RXn: qui charge dans le registre R0 la valeur stockée en mémoire à l'adresse contenue dans RXn.
// 0111 11nn
// (6 bits)
// Rn
// HHLL
void fLOADX() {
    // Récupération de n
    byte mask = 3; // 0000 0011
    byte n = ir & mask;

    // Chargement de l'adresse contenue dans RXn vers AddressLatch
    // partie HH
    SR(2 * n);
    Rout();
    ALHin();
    // partie LL
    SR(2 * n + 1);
    Rout();
    ALLin();
    // Lecture
    Read();
    WaitMem();
    // Mise de la valeur dans R0
    SR(0);
    DLout();
    Rin();

    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "LDX R0 RX%d\n", n);
}
operation LOADX = { "LDX", 0x7C, 6, 'c', 'n', fLOADX };

//////////////////////////
// MOVE : MV Rn, arg : qui charge dans le registre Rn la valeur de arg (une valeur immédiate 1 octet).
// 0101 0nnn
// (5 bits)
// Rn
// arg (8 bits)
void fMOVE() { 
    // Récupération de n
    byte mask = 7; // 0000 0111
    byte n = ir & mask;
    // Récupération de arg
    PCout();
    ALin();
    AAout();
    PCin();
    Read();
    WaitMem();
    // Écriture dans le registre
    SR(n);
    DLout();
    Rin();
    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "MV R%d %02x\n", n, dbus);
}
operation MOVE = { "MV", 0x50, 5, 'c', 'c', fMOVE };

//////////////////////////
// MOVEREG : MV Rn, Rm : qui charge dans le registre Rn la valeur du registre Rm.
// 00nn nmmm
// (2 bits)
// Rn
// Rm
void fMOVEREG() { 
    // Récupération de m
    byte mask = 7; // 0000 0111
    byte m = ir & mask;
    SR(m);
    // Mise de la donnée sur le bus
    Rout();
    // Récupération de n
    mask = 56; // 0011 1000
    byte n = (ir & mask) >> 3;
    SR(n);
    // Mise de la donnée dans Rn
    Rin();
    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "MVR R%d R%d\n", n, m);
}
operation MOVEREG = { "MVR", 0x00, 2, 'c', 'c', fMOVEREG };

//////////////////////////
// DECREMENT : DEC Rn : qui décrémente la valeur du registre Rn.
// 0101 1nnn
// (5 bits)
// Rn
void fDECREMENT() { 
    // Récupération de n
    byte mask = 7; // 0000 0111
    byte n = ir & mask;
    SR(n);
    // Mise de la donnée sur X
    Rout();
    Xin();
    // decrémentation
    decALU();
    // Mise du résultat de l'ALU dans Rn
    ALUout();
    Rin();
    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "DEC R%d\n", n);
}
operation DECREMENT = { "DEC", 0x58, 5, 'c', 'n', fDECREMENT };

//////////////////////////
// INCREMENT : INC Rn : qui incrémente la valeur du registre Rn.
// 0110 0nnn
// (5 bits)
// Rn
void fINCREMENT() { 
    // Récupération de n
    byte mask = 7; // 0000 0111
    byte n = ir & mask;
    SR(n);
    // Mise de la donnée sur X
    Rout();
    Xin();
    // incrémentation
    incALU();
    // Mise du résultat de l'ALU dans Rn
    ALUout();
    Rin();
    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "INC R%d\n", n);
}
operation INCREMENT = { "INC", 0x60, 5, 'c', 'n', fINCREMENT };

//////////////////////////
// NOT : NOT Rn : qui inverse bit à bit la valeur du registre Rn.
// 0110 1nnn
// (5 bits)
// Rn
void fNOT() { 
    // Récupération de n
    byte mask = 7; // 0000 0111
    byte n = ir & mask;
    SR(n);
    // Mise de la donnée sur X
    Rout();
    Xin();
    // NON logique
    notALU();
    // Mise du résultat de l'ALU dans Rn
    ALUout();
    Rin();
    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "NOT R%d\n", n);
}
operation NOT = { "NOT", 0x68, 5, 'c', 'n', fNOT };

//////////////////////////
// ADD : ADD Rn, Rm : qui ajoute à la valeur du registre Rn, la valeur du registre Rm.
// 100n nmmm
// (3 bits)
// Rn
// Rm
void fADD() {
    // Récupération de m
    byte mask = 7; // 0000 0111
    byte m = ir & mask;
    SR(m);
    // Mise de la donnée sur Y
    Rout();
    Yin();
    // Récupération de n
    mask = 24; // 0001 1000
    byte n = (ir & mask) >> 3;
    SR(n);
    // Mise de la donnée sur X
    Rout();
    Xin();
    // Addition
    addALU();
    // Mise du résultat de l'ALU dans Rn
    ALUout();
    Rin();
    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "ADD R%d R%d\n", n, m);
}
operation ADD = { "ADD", 0x80, 3, 'c', 'c', fADD };

//////////////////////////
// SUB : SUB Rn, Rm : qui soustrait à la valeur du registre Rn, la valeur du registre Rm.
// 101n nnnn
// (3 bits)
// Rn
// Rm
void fSUB() {
    // Récupération de m
    byte mask = 7; // 0000 0111
    byte m = ir & mask;
    SR(m);
    // Mise de la donnée sur Y
    Rout();
    Yin();
    // Récupération de n
    mask = 24; // 0001 1000
    byte n = (ir & mask) >> 3;
    SR(n);
    // Mise de la donnée sur X
    Rout();
    Xin();
    // Soustraction
    subALU();
    // Mise du résultat de l'ALU dans Rn
    ALUout();
    Rin();
    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "SUB R%d R%d\n", n, m);
}
operation SUB = { "SUB", 0xA0, 3, 'c', 'c', fSUB };

//////////////////////////
// AND : AND Rn, Rm : qui fait la conjonction bit à bit des 2 registres Rn et Rm et met le résultat dans Rn.
// 110n nnnn
// (3 bits)
// Rn
// Rm
void fAND() {
    // Récupération de m
    byte mask = 7; // 0000 0111
    byte m = ir & mask;
    SR(m);
    // Mise de la donnée sur Y
    Rout();
    Yin();
    // Récupération de n
    mask = 24; // 0001 1000
    byte n = (ir & mask) >> 3;
    SR(n);
    // Mise de la donnée sur X
    Rout();
    Xin();
    // ET logique
    andALU();
    // Mise du résultat de l'ALU dans Rn
    ALUout();
    Rin();
    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "AND R%d R%d\n", n, m);
}
operation AND = { "AND", 0xC0, 3, 'c', 'c', fAND };

//////////////////////////
// SWAP : SWAP Rn, Rm : qui inverse les valeurs des registres Rn et Rm entre eux.
// 111n nmmm
// (3 bits)
// Rn
// Rm
void fSWAP() {
    // Récupération de m
    byte mask = 7; // 0000 0111
    byte m = ir & mask;
    SR(m);
    // Mise de la donnée dans DLatch
    Rout();
    DLin();
    // Récupération de n
    mask = 24; // 0001 1000
    byte n = (ir & mask) >> 3;
    SR(n);
    // Mise dans le bus
    Rout();
    // Rn -> Rm
    SR(m);
    Rin();
    // Rm -> Rn
    DLout();
    SR(n);
    Rin();
    // ASSEMBLEUR
    if (fd_output_assm >= 0)
        fprintf(fdopen(fd_output_assm, "w"), "SWP R%d R%d\n", n, m);
}
operation SWAP = { "SWP", 0xE0, 3, 'c', 'c', fSWAP };


// TABLEAU contenant toutes les opérations du jeu d'opérations assembleur (ici 17) !
int NUM_OPS = 17;
operation *OPS[17] = {
    &NOP,
    &JUMP,
    &JUMPZERO,
    &JUMPCARRY,
    &STORE,
    &STOREX,
    &LOAD,
    &LOADX,
    &MOVE,
    &MOVEREG,
    &DECREMENT,
    &INCREMENT,
    &NOT,
    &ADD,
    &SUB,
    &AND,
    &SWAP
};



// Décode une opération selon le code instruction (sur 8 bits max) fourni.
// ir_code  : un octet contenant un code d'une instruction
// ld_op    : un objet operation qui va contenir l'instruction décodée.
// verbose    : booléen : 1 -> la fonction écrit des informations supplémentaires vers la sortie standard.
// return   :   0 -> pas d'instruction trouvée
//              1 -> instruction trouvée et chargée dans ld_op 
//              -1 > une erreur est survenue 
int decode(byte ir_code, operation *ld_op, char verbose) {
    int res = 0;
    if (verbose)
        printf("Décodage de l'opération...");
    // Décodage de l'instruction dont le code instruction est dans le registre ir_code
    // Recherche du code instruction ir_code correspondant à une instructions dans le tableau OPS
    // on se sert d'un deuxième tableau "valid_ops" pour constater les opérations qui correspondent.
    // 1 -> valide ou 0 -> invalide
    int valid_ops[NUM_OPS];
    int num_ops_ecartees = 0;

    for (int i = 0; i < NUM_OPS; i++) { valid_ops[i] = 1; }
    // Pour l'instant toutes nos instructions sont valides et on va les filtrer bits à bits.
    // pour chaque bit dans un octet (code instructions -> 8 bits max)
    for (int b = 0; b < 8; b++) { 
        // Pour chacune des opérations : est-elle valide ?
        for (int i = 0; i < NUM_OPS; i++) { 
            // On traite uniquement les opérations qui n'ont pas encore été écartées.
            if (valid_ops[i] == 1) {
                // Correspondance du bit du code instruction (ir_code) avec le bit de l'opération i :
                byte mask_curr_bit = 128 >> b; // ex : b = 2 -> mask_curr_bit = 0010 0000
                // Mise à l'écart d'une opération si l'on détecte un bit différent entre ir_code et le code instruction
                // avant le nombre de bits utilisé pour le code op.
                if ( (ir_code & mask_curr_bit) != (OPS[i]->code_value & mask_curr_bit) && b+1 <= OPS[i]->code_num_bits) {
                    // On écarte cette opération car les deux bits sont différents
                    //printf("\nOn écarte l'opération %s au bit %d \t", OPS[i], b);
                    //print_binary((ir_code));
                    //printf(" != ");
                    //print_binary((OPS[i]->code_value));
                    num_ops_ecartees++;
                    valid_ops[i] = 0;
                }

            }    
        }
    }
    if (verbose)
        printf("[ok]\n");
    //printf("Sur %d OPS : %d sont écartées.\n", NUM_OPS, num_ops_ecartees);
    //printf("L'opération trouvé est OPS[%d] %s\n", decoded_num_op, OPS[decoded_num_op]);
    // Affichage des opérations valides
    for (int i = 0; i < NUM_OPS; i++) {
        if (valid_ops[i]) {
            if (verbose) { printf("L'operation %s est valide par rapport à ir_code %02x \n", OPS[i]->nom, ir_code); }
            // Écriture de l'opération validée et chargement des éventuelles opérandes dans current_op 
            // pour permettre à l'ALU d'éxécuter l'instruction :
            //  - nom de l'op
            strncpy(ld_op->nom, OPS[i]->nom, 3);
            //  - valeur du code
            ld_op->code_value = OPS[i]->code_value;
            //  - code_num_bits
            ld_op->code_num_bits = OPS[i]->code_num_bits;

            // Opérande 1
            ld_op->opr1_type = OPS[i]->opr1_type;
            // Opérande 2
            ld_op->opr2_type = OPS[i]->opr2_type;

            // Fonction de l'opération
            ld_op->fOP = OPS[i]->fOP;

            res = 1;
            break;
        }
    }
    
    return res;
}