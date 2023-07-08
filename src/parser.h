

#ifndef PARSER_H
#define PARSER_H

// Ici on va utiliser le type byte pour des variables d'un octet (0 à 255)
typedef unsigned char byte;

// Utilisé comme valeur de retour à la fonction parseHexaToByte
// contient la valeur de l'octet hexadecimale parsé ainsi que l'offset qui suit cette valeur dans la ligne.
typedef struct hexaParsed {
    byte hex_val;
    int next_offset;
} hexaParsed;

// Affiche une valeur en binaire
void print_binary(byte n);

// Calcul la valeur hexadécimal des 2 caractères donnés
// et retourne cette valeur dans un octet.
byte byteFromHexValue(char c1, char c2);

// Parser de valeur hexa
hexaParsed parseHexaToByte(char *line, int from_offset);

// Parser de lignes de fichiers de prgm vers la ram
long parseLineToRam(char *line, byte *ram, int ram_size);

#endif








