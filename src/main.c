
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "proc.h"
#include "operations.h"
#include "parser.h"
#include "debug.h"


// MAIN
int main(int argc, char *argv[]) {

    printf("Vérification des arguments... ");
    if(argc <= 2) {
        printf("[FA!L]\n");
        perror("Erreur la commande s'apelle ainsi : proc [-debug] [-v] <chemin_du_programme> <chemin_du_fichier_sortie_assm>");
        exit(-1);
    }

    // Récupération des arguments
    // fichier_programme
    char *prgm_file;
    // fichier_sortie_assm
    char *out_file;
    // mode simple 0 | debug 1
    char mode = 0;
    // verbose active 1 | no active 0
    char verbose = 0;
    if (argc == 3) {
        prgm_file = argv[1];
        out_file = argv[2];
    }
    else if (argc == 4) {
        if (strncmp("-debug", argv[1], 6) == 0) { mode = 1; }
        if (strncmp("-v", argv[1], 6) == 0) { verbose = 1; }
        prgm_file = argv[2];
        out_file = argv[3];
    }
    else if (argc == 5) {
        if (strncmp("-debug", argv[1], 6) == 0 || strncmp("-debug", argv[2], 6) == 0) { mode = 1; }
        if (strncmp("-v", argv[1], 6) == 0 || strncmp("-v", argv[2], 6) == 0) { verbose = 1; }
        prgm_file = argv[3];
        out_file = argv[4];
    }
    else {
        printf("[FA!L]\n");
        perror("Erreur la commande s'apelle ainsi : proc [-debug] <chemin_du_programme> <chemin_du_fichier_sortie_assm>");
        exit(-1);
    }

    printf("[ok]\n");


    if (verbose) {
        printf("Construction du processeur... ");
        printf("[ok]\n");
    }

    // INITIALISATION
    // On veut charger dans la RAM le programme donné. Le format du programme est ainsi fait : chaque ligne est de la forme <adresse>:<instruction>
    // La fin du fichier est noté FFFF.
    if (verbose) {
        printf("Initialisation des registres... [ok]\n");
        printf("Chargement du programme... ");
    }
    // Chargement du fichier.
    FILE *prgm;
    prgm = fopen(prgm_file, "r");
    if(prgm == NULL) {
        perror("Erreur lors de l'ouverture, en lecture, du programme donné");
        exit(-1);
    }
    if (verbose)
        printf("[ok]\n");

    // Lecture du fichier
    // Utilisation avec fgets
    char line[TAILLE_RAM];
    char reading = 1;
    int n_inst = 0;

    // Lecture puis écriture dans la RAM de chaque ligne du fichier
    while (reading) {
        if(fgets(line, TAILLE_RAM, prgm) == NULL) {
            reading = 0;
        }
        else {
            //printf("\n%s\n", line);

            // Analyse syntaxique et écriture dans la ram
            short pRes = parseLineToRam(line, ram, TAILLE_RAM);
            if (pRes ==  -3) { // La dernière ligne a été atteinte !
                reading = 0;
            }
            else if(pRes == -2) { // Un erreur d'adresse est survenue !
                perror("Erreur d'écriture de l'instruction dans la RAM, l'adresse est incorrect ou la RAM est trop petite, voir la fonction parseLineToRam()");
                exit(-1);
            }
            else if(pRes == -1) { // Un erreur de lecture est survenue !
                perror("Erreur d'écriture de l'instruction dans la RAM, erreur de lecture du fichier (la syntaxe est sûrement incorrect), voir la fonction parseLineToRam()");
                exit(-1);
            }
            else {
                // Instruction écrite !
                // Sauvegarde de la première adresse du programme dans PC.
                if(n_inst == 0) {
                    if (verbose)
                        printf("Sauvegarde de l'adresse de départ <%04x> du programme dans PC ", pRes);
                    pc.PCH = (byte)(pRes >> 8); // Partie Haute
                    pc.PCL = (byte)(pRes & 255); // Partie Basse
                    if (verbose)
                        printf("(PCH <%02x> PCL <%02x>)... ", pc.PCH, pc.PCL);
                    if (verbose)
                        printf("[ok]\n");
                }
                n_inst++;
            }
        }
    }
    if (verbose)
        printf("Écriture dans la RAM de %d instructions... [ok]\n", n_inst);
    if (verbose)
        printf("Déchargement du fichier programme... ");
    if(fclose(prgm) != 0) {
        if (verbose)
            perror("Erreur lors de la fermeture du fichier, le programme continue");
        if (verbose)
            printf("[FA!L]\n");
    }
    else {
        if (verbose)
            printf("[ok]\n");
    }

    // Ouverture du fichier de sortie pour la récupération des instructions assembleurs
    if (verbose)
        printf("Chargement du fichier de sortie assembleur... ");
    // Chargement du fichier.
    FILE *output_assm;
    output_assm = fopen(out_file, "w");
    if(output_assm == NULL) {
        perror("Erreur lors de l'ouverture, en lecture, du fichier de sortie assembleur");
        exit(-1);
    }
    if (verbose)
        printf("[ok]\n");
    // récupération du file_descriptor depuis FILE*
    fd_output_assm = fileno(output_assm);
    // FIN de l'initialisation.

    // Affichage de la RAM
    // for (int d=500; d < 600; d++) { printf(" %d \t: %02x \n", d, ram[d]); }


    // BOUCLE INSTRUCTIONs
    // Affichage de l'aide pour les commandes
    
    // Tant qu'on arrive pas à la fin du programme on éxécute les instructions
    int iteration_insts = 0;
    int num_nop = 0;
    int force_quit = 0;
    while( num_nop < 3) {

        // Affiche de l'aide une première fois
        if (mode && iteration_insts == 0)
            displayHelp();

        // En mode debug on attend une commande
        if (mode) {
            num_nop = 0;
            if (readDebugCommand() == 1)
                break;
        }

        // Chargement de l'instruction
        if (verbose)
            printf("Chargement de l'instruction...");
        load_next_inst();
        //printf("\n pc is %02x %02x\n", pc.PCH, pc.PCL);
        //printf("ram[0210] is %02x\n", ram[0x0210]);
        if (verbose)
            printf(" [ ir = %02x ] ", ir);
        if (verbose)
            printf("[ok]\n");

        // Décodage de l'instruction
        operation current_op = { "---", 0x00, 0, 'n', 'n', NULL};
        int res_decode = decode(ir, &current_op, 0);
        if (res_decode != 1) {
            if (verbose)
                printf("Erreur lors du décodage ou l'instruction ne correspond à aucune instruction connue par le processeur (voir fichier operation.h et operation.c), code_instruction introuvable -> ");
            if (verbose)
                print_binary(ir);
            if (verbose)
                printf("\n");
        }
        else {
            // Compte le nombre de nop effectués à la suite (3 NOP = arrêt du programme);
            if (current_op.code_value == 0x00) { num_nop++; }
            else { num_nop = 0; }
        }

        // Éxécution de l'instruction
        if (current_op.fOP != NULL)
            (*current_op.fOP)();
            printf("[OP] : %s\n", current_op.nom);

        iteration_insts++;
    }
    printf("\n");

    // Affichage des registres
    displayRegisters();

    // Affichage de la ram
    //displayRam(0x0200, 0x20A);

    printf("[!] Le code assembleur se trouve dans le fichier : << %s >>\n", out_file);

    return EXIT_SUCCESS;
}


