
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

// Affiche une valeur (8 bits) sous forme binaire
void print_binary(byte n) {
    for (int b = 0; b < 8; b++) {
        byte mask = 128 >> b;
        if ((n & mask) == mask)
            printf("1");
        else
            printf("0");
    }
}


// Calcul la valeur hexadécimal des 2 caractères donnés
// et retourne cette valeur dans un octet.
// Biensûr on peut utiliser la fonction strtol(str, NULL, 16) à la place.
byte byteFromHexValue(char c1, char c2) {
    byte octet = 0;
    // C1 : 
    if (c1 >= 48 && c1 <= 57) { //  - de 0 à 9
        octet = (c1 - 48) << 4;
    }
    else if (c1 >= 65 && c1 <= 70) { //  - de A à F
        octet = (c1 - 55) << 4;
    }
    else if (c1 >= 97 && c1 <= 102) { //  - de a à f
        octet = (c1 - 87) << 4;
    }
    // C2 : 
    if (c2 >= 48 && c2 <= 57) { //  - de 0 à 9
        octet = octet + (c2 - 48);
    }
    else if (c2 >= 65 && c2 <= 70) { //  - de A à F
        octet = octet + (c2 - 55);
    }
    else if (c2 >= 97 && c2 <= 102) { //  - de a à f
        octet = octet + (c2 - 87);
    }

    return octet;
}


// Fonction qui passe les esapces et lit une valeur hexadécimal dans une ligne
// parametres : une chaîne de caractères, la ligne
//              l'offset duquel on doit commencer à lire.
hexaParsed parseHexaToByte(char *line, int from_offset) {

    // Passe les espaces
    while (line[from_offset] == ' ') {
        from_offset++;
    }

    // Calcul de la valeur de l'octet hexa
    hexaParsed res;
    res.hex_val = byteFromHexValue(line[from_offset], line[from_offset + 1]);
    //printf("hex_val is %02x\n", res.hex_val);
    res.next_offset = from_offset + 2;

    return res;
}




/**
 * Lit une ligne d'un programme et la place dans la RAM selon son adresse.
 * @return <int> retourne l'adresse de l'instruction si réussit, sinon retourne :
 * -1 pour une erreur de lecture (syntaxe du programme erronée),
 * -2 pour une erreur d'adresse (si la RAM est trop petite par exemple),
 * -3 pour la dernière ligne atteinte : lecture de 2 octets pleins "FFFF"
 * @param char *line la ligne à parser. La ligne doit être de la forme :
 *
 *  +-----------------+----------------------+---------------------+
 *  | Adresse (héxa.) |  Séparateur > ":"    | Instruction (héxa.) |
 *  +-----------------+----------------------+---------------------+
 *
 *  exemple : "HHLL: HHEEXXAA..."
 *
 *  La casse est tolérée. 
 * 
 * @param byte *ram la variable ram, tableau d'octet.
*/
long parseLineToRam(char *line, byte *ram, int ram_size) {
    // Analyse syntaxique de l'adresse
    long addr = strtol(line, NULL, 16); // en base 16
    printf("Écriture d'instruction dans la RAM à l'adresse <%04lx>... ", addr);
    
    // L'adresse est-elle la balise de fin du programme "FFFF" ?
    if(strncmp("FFFF", &line[0], 4) == 0) {
        printf("[END]\n");
        return -3; // fin.
    }
    
    // Validation de l'adresse
    if(addr < 0x0000 | addr > (0x0000 + ram_size)) {
        printf("[FA!L]\n");
        return -2; // erreur
    }

    // Séparateur ":   "
    if(strncmp(":", &line[4], 1) != 0) {
        printf("[FA!L]\n");
        return -1; // erreur
    }

    // On s'apprête à parser les valeurs hexadécimal de la ligne 
    // correspondant à l'instruction à écrire dans la ram
    hexaParsed hex_p;
    hex_p.next_offset = 5;
    int num_vals_parsed = 0;

    // Tant que le caractère suivant n'est pas la fin de la ligne, alors on parse la valeur
    while(line[hex_p.next_offset] != '\0' && line[hex_p.next_offset] != '\n') {
        // Récupération de la valeur hexa
        hex_p = parseHexaToByte(line, hex_p.next_offset);

        //printf("hexa_val_parsed is %02x \n", hex_p.hex_val);

        // On s'assure de ne pas dépasser la taille de la ram avant d'écrire
        if((addr + num_vals_parsed) > (addr + ram_size)) {
            printf("[FA!L]\n");
            return -2; // erreur
        }
        // Écriture dans la ram
        ram[addr + num_vals_parsed] = hex_p.hex_val;
        // incrémentation du nombre de valeurs décodées
        num_vals_parsed++;
    }

    // Tout s'est bien passé !
    printf("[ok]\n");
    return addr;
}






