
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "proc.h"
#include "debug.h"

#define BUFFER_STDIN_SIZE 4096

// Utiliser pour le mode debug, lit une commande.
// 
// Commandes :
//      help : affiche cette liste de commandes
//      (entrée) : passe à la commande suivante
//      quit | q : quit le programme.
//      abus : affiche le bus d'adresse
//      al : affiche Address Latch
//      pc : affiche PC
//      dbus : affiche le bus de données
//      dl : affiche DataLatch
//      ir : affiche le registre d'instruction ir
//      alu : affiche la sortie de l'alu
//      x : affiche X (entrée alu)
//      y : affiche Y (entrée alu)
//      flags : affiche les flags de l'alu
// 
// retourne : 1 pour la commande "quit" ou "q" sinon 0
int readDebugCommand() {
    // La taille du buffer doit être assez grande pour capturer tout les caractères 
    // sans en laisser dans le buffer stdin, sinon ils risquent d'être capturer au prochain fgets.
    char buf[BUFFER_STDIN_SIZE]; 
    char stop = -1;

    while ( stop == -1) {
        printf("\n>");
        fgets(buf, BUFFER_STDIN_SIZE, stdin);
        // Vide le buffer STDIN

        
        if (strncmp("help", buf, 4) == 0) {
            displayHelp();
        }
        else if (strncmp("\n", buf, 1) == 0) {
            stop = 0;
        }
        else if (strncmp("quit", buf, 4) == 0) {
            stop = 1;
        }
        else if (strncmp("q", buf, 1) == 0) {
            stop = 1;
        }
        else if (strncmp("abus", buf, 4) == 0) {
            displayAbus();
        }
        else if (strncmp("alu", buf, 3) == 0) {
            displayALU();
        }
        else if (strncmp("al", buf, 2) == 0) {
            displayALatch();
        }
        else if (strncmp("pc", buf, 2) == 0) {
            displayPC();
        }
        else if (strncmp("dbus", buf, 4) == 0) {
            displayDbus();
        }
        else if (strncmp("dl", buf, 2) == 0) {
            displayDLatch();
        }
        else if (strncmp("ir", buf, 2) == 0) {
            displayIR();
        }
        else if (strncmp("x", buf, 1) == 0) {
            displayX();
        }
        else if (strncmp("y", buf, 1) == 0) {
            displayY();
        }
        else if (strncmp("flags", buf, 5) == 0) {
            displayFlags();
        }
        else {
            printf("commande incorrecte. Tapez 'help' pour afficher la liste de commande.\n");
        }
    }
        
    printf("\n");
    return stop;
}

void displayHelp() {
    printf("\n************************\n*** Liste de commandes :\n\thelp : affiche cette liste de commandes\n\t(entrée) : passe à la commande suivante\n\tquit | q : quit le programme.\n\tabus : affiche le bus d'adresse\n\tal : affiche Address Latch\n\tpc : affiche PC\n\tdbus : affiche le bus de données\n\tdl : affiche DataLatch\n\tir : affiche le registre d'instruction ir\n\talu : affiche la sortie de l'alu\n\tx : affiche X (entrée alu)\n\ty : affiche Y (entrée alu)\n\tflags : affiche les flags de l'alu\n");
}


// AFFICHAGE DES REGISTRES
void displayRegister(int num) {
    if (num >= 0 && num <= 7)
        printf("R[%d] = %02x\n", num, reg[num]);
}

void displayRegisters() {
    printf("*****************\n*** Registres ***\n*****************\n");
    for (int r = 0; r < 8; r++)
        displayRegister(r);
}

// AFFICHAGE DE LA RAM
void displayRamValue(int16_t adr) {
    if (adr >= 0 && adr <= TAILLE_RAM)
        printf("RAM<%02x%02x> = %02x\n", (adr & 0xFF00)>>8, (adr & 0xFF), ram[adr]);
}

void displayRam(int16_t from, int16_t to) {
    if (from >= 0 && to <= TAILLE_RAM) {
        printf("\n*************************\n*** RAM [%04x ; %04x] ***\n*************************\n", from, to);
        for (int16_t i = from; i <= to; i++) 
            printf("RAM<%02x%02x> = %02x\n", (i & 0xFF00)>>8, (i & 0xFF), ram[i]);
    }
}

// AFFICHAGE ADDRESSE
void displayAddr() {
    printf("\n> Partie adresse :\n");
    printf("\tabus \t\t : %04x\n", abus);
    printf("\tAddressLatch \t : %02x%02x\n", pc.PCH, pc.PCL);
    printf("\tPC \t\t : %02x%02x\n", pc.PCH, pc.PCL);
}
void displayAbus() { printf("Bus d'adresse : #%02x%02x\n", (abus & 0xFF00)>>8, (abus & 0xFF)); }
void displayALatch() { printf("Adresse Latch : #%02x%02x\n", al.ALH, al.ALL); }
void displayPC() { printf("PC : #%02x%02x\n", pc.PCH, pc.PCL); }


// AFFICHAGE DONNÉE
void displayData() {
    printf("\n> Partie donnée :\n");
    printf("\tdbus \t\t : %02x\n", dbus);
    printf("\tIR \t\t : %02x\n", ir);
    printf("\tDataLatch \t : %02x\n", DLatch);
}
void displayDbus() { printf("bus de donnée : %02x\n", dbus); }
void displayIR() { printf("IR : #%02x\n", ir); }
void displayDLatch() { printf("Data Latch : %02x\n", DLatch); }

// AFFICHAGE ALU
void displayALUPart() {
    printf("\n> Partie ALU :\n");
    printf("\tX \t\t : %02x\n", X);
    printf("\tY \t\t : %02x\n", Y);
    printf("\tFlags \t : %02x\n", flags);
    printf("\tALU \t : %02x\n", alu);
}
void displayX() { printf("X (alu) : %02x\n", X); }
void displayY() { printf("Y (alu) : %02x\n", Y); }
void displayFlags() { printf("Flags (alu): %02x\n", flags); }
void displayALU() { printf("ALU (output) : %02x\n", alu); }


