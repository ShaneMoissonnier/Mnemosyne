#include "common.h"
#include "mem.h"
#include "mem_os.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_ALLOC 100000
#define MAX_BLOC 200
#define FREQ_FREE 3

static void *allocs[MAX_ALLOC];

void afficher_z(void *adresse, size_t taille, int free)
{
    debug("Zone %s, Adresse : %lu, Taille : %lu\n", free ? "libre" : "occupee",
          (unsigned long)adresse, (unsigned long)taille);
}

int main(int argc, char *argv[])
{
    mem_init();
    if (argc == 2)
    {
        if (strcmp(argv[1], "first") == 0)
        {
            mem_fit(mem_first_fit);
            printf("Stratégie first fit\n");
        }
        else if (strcmp(argv[1], "best") == 0)
        {
            mem_fit(mem_best_fit);
            printf("Stratégie best fit\n");
        }
        else if (strcmp(argv[1], "worst") == 0)
        {
            mem_fit(mem_worst_fit);
            printf("Stratégie worst fit\n");
        }
    }
    srand(time(NULL));
    fprintf(stderr,
            "Test réalisant des series d'allocations / désallocations\n"
            "afin d'obtenir une forte fragmentation de manière aléatoire\n");

    int i = 0;
    int j = 0;
    int size = (rand() % MAX_BLOC) + 1;
    int free = 0;
    // On alloue en boucle des bloc de tailles variable et aléatoire comprise
    // entre 1 et MAX_BLOC
    while ((i < MAX_ALLOC) && (allocs[i] = mem_alloc(size)) != NULL)
    {
        printf("%d -------------------------------\n", i);
        printf("Allocation en %d\n",
               (int)((char *)allocs[i] - (char *)get_memory_adr()));
        printf("size : %d\n", size);
        assert(allocs[i] <
               (void *)((char *)get_memory_adr() + get_memory_size()));

        // On libère à intervalle aléatoire un bloc occupé d'adresse aléatoire
        // parmis les blocs alloué en mémoire
        if (rand() % FREQ_FREE == 0)
        {
            free = ((rand() % (i + 1)) - 1);
            printf("Libération %d\n", free);
            assert(allocs[free] <
                   (void *)((char *)get_memory_adr() + get_memory_size()));
            mem_free(allocs[free]);
        }
        size = (rand() % MAX_BLOC) + 1;
        i++;
        // mem_show(afficher_z);
    }
    fb *current = get_head()->fb_head;
    while (current != NULL)
    {
        j++;
        current = current->next;
    }

    // Affichage à la fin du test une fois la mémoire trop fragmentée
    if (mem_alloc(size) == NULL)
    {
        printf("Tentative d'allocation de  %d octets.\n"
               "Impossible car la mémoire est trop fragmentée.\n"
               "%i blocs ont été alloué (certains ont peut-être été libérés) et %i blocs libres\n",
               size, i, j);
    }
    else
    {
        printf("Le tableau d'allocation est trop petit, augmentez MAX_ALLOC ou "
               "MAX_BLOC\n");
    }

    // Fin du test
    return 0;
}
