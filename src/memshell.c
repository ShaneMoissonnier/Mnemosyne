#include "common.h"
#include "mem.h"
#include "mem_os.h"
#include <stdio.h>
#include <stdlib.h>

#define TAILLE_BUFFER 128
#define MAX_ALLOCATIONS 128

void aide()
{
    fprintf(stderr, "Aide :\n");
    fprintf(stderr, "Saisir l'une des commandes suivantes\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "a taille  :   allouer un bloc de la taille souhaitee\n");
    fprintf(stderr,
            "l adresse :   librer un bloc alloue precedemment a adresse\n");
    fprintf(
        stderr,
        "f n       :   librer le bloc alloue lors de la n-ieme allocation\n");
    fprintf(stderr, "i         :   afficher la liste des emplacements memoire "
                    "inoccupes\n");
    fprintf(stderr, "o         :   afficher la liste des emplacements memoire "
                    "occupes\n");
    fprintf(stderr, "M         :   afficher la liste de tous les emplacements "
                    "memoire (libres et occupes)\n");
    fprintf(stderr, "m         :   afficher le dump de la memoire\n");
    fprintf(stderr, "s         :   affiche le tableau de tous les emplacements "
                    "memoire (en-tete, libre et occupes)\n");
    fprintf(stderr, "h         :   afficher cette aide\n");
    fprintf(stderr, "q         :   quitter ce programme\n");
    fprintf(stderr, "\n");
}

void afficher_zone(void *adresse, size_t taille, int free)
{
    printf("Zone %s, Adresse : %lu, Taille : %lu\n", free ? "libre" : "occupee",
           (unsigned long)adresse, (unsigned long)taille);
}

void afficher_tas(void *adresse, size_t taille, int free)
{
    switch (free)
    {
    case 0:
        printf("%11d |%11s |%11ld |%11d |%11ld\n", (int)(adresse - get_memory_adr()), "BUSY", taille, (int)((adresse + sizeof(fb)) - get_memory_adr()), taille - sizeof(fb));
        break;
    case 1:
        printf("%11d |%11s |%11ld |\n", (int)(adresse - get_memory_adr()), "FREE", taille);
        break;
    case 2:
        printf("%11d |%11s |%11ld |\n", (int)(adresse - get_memory_adr()), "HEADER", taille);
        break;
    }
}

void afficher_entete()
{
    printf("%11s |%11s |%11s |%11s |%11s\n", "@", "TYPE", "SIZE", "user@", "user size");
    printf("---------------------------------------------------------------\n");
    afficher_tas(get_memory_adr(), sizeof(header), 2);
}

void afficher_zone_libre(void *adresse, size_t taille, int free)
{
    if (free)
        afficher_zone(adresse, taille, 1);
}

void afficher_zone_occupee(void *adresse, size_t taille, int free)
{
    if (!free)
        afficher_zone(adresse, taille, 0);
}

int main(int argc, char **argv)
{
    char buffer[TAILLE_BUFFER];
    char commande;
    char *adresse;
    void *ptr;
    int offset;
    int taille, i;
    void *allocations[MAX_ALLOCATIONS];
    int nb_alloc = 0;

    aide();
    mem_init();

    while (1)
    {
        fprintf(stderr, "? ");
        fflush(stdout);
        commande = getchar();
        switch (commande)
        {
        case 'a':
            scanf("%d", &taille);
            ptr = mem_alloc(taille);
            allocations[nb_alloc] = ptr;
            nb_alloc++;
            if (ptr == NULL)
                printf("Echec de l'allocation\n");
            else
                printf("Memoire allouee en %d\n",
                       (int)(ptr - get_memory_adr()));
            break;
        case 'l':
            scanf("%d", &offset);
            mem_free(get_memory_adr() + offset);
            printf("Memoire liberee\n");
            break;
        case 'f':
            scanf("%d", &offset);
            mem_free(allocations[offset - 1]);
            printf("Memoire liberee\n");
            break;
        case 'i':
            mem_show(afficher_zone_libre);
            break;
        case 'o':
            mem_show(afficher_zone_occupee);
            break;
        case 'M':
            mem_show(afficher_zone);
            break;
        case 's':
            afficher_entete();
            mem_show(afficher_tas);
            break;
        case 'm':
            printf("[ ");
            adresse = get_memory_adr();
            for (i = 0; i < get_memory_size(); i++)
                printf("%d ", adresse[i]);
            printf("]\n");
            break;
        case 'h':
            aide();
            break;
        case 'q':
            exit(0);
        default:
            fprintf(stderr, "Commande inconnue !\n");
        }
        /* vide ce qu'il reste de la ligne dans le buffer d'entree */
        fgets(buffer, TAILLE_BUFFER, stdin);
    }
    return 0;
}
