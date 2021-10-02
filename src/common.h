#if !defined(__COMMON_H__)
#define __COMMON_H__
#include <stdlib.h>

#if defined(DEBUG)
#define debug(...) fprintf(stderr, __VA_ARGS__)
#else
#define debug(...)
#endif

// La valeur d'alignement de nos données (8 octets pour un système Linux)
#define ALIGN 8
// On définit notre valeur de saut par les ALIGN-1 bits de poids faibles
#define LSB_STEP (ALIGN - 1)

/*
 * On crée un masque pour supprimer les ALIGN-1 bits de poids faibles et on l'applique à notre taille
 * (Note : ici notre saut (LSB_STEP) est ajouté à notre taille avant application du masque
 * pour obtenir un "alignement minimum" correct si 0 < size < ALIGN)
*/
#define get_align(size) ((size + LSB_STEP) & ~(LSB_STEP))

void *get_memory_adr(void);
size_t get_memory_size(void);

#endif
