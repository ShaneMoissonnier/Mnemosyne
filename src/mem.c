
#include "mem.h"
#include "common.h"
#include "mem_os.h"
#include <stdio.h>

// Stocke l'adresse de l'entête
header *g_head;

//-------------------------------------------------------------
// mem_init
//-------------------------------------------------------------
void mem_init()
{
    void *mem_adr = get_memory_adr();
    size_t mem_size = get_memory_size();

    // On crée l'entête qui stockera nos informations globales
    g_head = (header *)mem_adr;

    // On crée le premier bloc libre qui occupe "toute" la mémoire
    fb *first_block = (fb *)(mem_adr + sizeof(header));

    // On initialise la taille du bloc libre à la mémoire restante
    first_block->size = mem_size - sizeof(header);

    // On initialise l'entête
    g_head->first = first_block;
    g_head->fit_func = &mem_first_fit;

    mem_fit(&mem_first_fit);
    return;
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
void *mem_alloc(size_t size)
{
    /* A COMPLETER */
    return NULL;
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
void mem_free(void *zone)
{
    /* A COMPLETER */
    return;
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free))
{
    void *mem_end = get_memory_adr() + get_memory_size();
    void *current_block = (void *)g_head->first;

    fb *free_block = (fb *)current_block;

    // On parcourt les blocs mémoires jusqu'à l'adresse de fin de mémoire
    while (current_block < mem_end)
    {
        // On récupère le type de la zone (libre (1) ou occupée (0)) ainsi que sa taille
        size_t block_state = (current_block == free_block);
        size_t size = free_block->size;

        // On affiche la zone mémoire
        print(current_block, size, block_state);

        // Si c'est une zone libre on passe à la zone suivante dans la liste
        if (block_state)
        {
            free_block = free_block->next;
        }

        current_block = (current_block + size);
    }
    return;
}

//-------------------------------------------------------------
// mem_fit
//-------------------------------------------------------------
void mem_fit(mem_fit_function_t *mff)
{
    g_head->fit_func = mff;
    return;
}

//-------------------------------------------------------------
// Stratégies d'allocation
//-------------------------------------------------------------
struct fb *mem_first_fit(struct fb *head, size_t size)
{
    fb *current_block = head;

    while (current_block != NULL)
    {
        // On récupère le premier bloc libre pouvant stocker notre bloc alloué
        if (size <= current_block->size)
        {
            return current_block;
        }

        current_block = current_block->next;
    }
    return NULL;
}
//-------------------------------------------------------------
struct fb *mem_best_fit(struct fb *head, size_t size)
{   
    fb *current_block = head;
    fb *best_block = head;

    while (current_block != NULL)
    {
        //On récupère le premier bloc qui à la taille minimum suffisante pour stocker notre bloc alloué.
        if (size <= current_block->size && (current_block->size < best_block->size || best_block->size < size))
        {
            best_block = current_block;
        }

        current_block = current_block->next;
    }

    if (size <= best_block->size){
        return best_block;
    }

    return NULL;
}
//-------------------------------------------------------------
struct fb *mem_worst_fit(struct fb *head, size_t size)
{   
    fb *current_block = head;
    fb *worst_block = head;

    while (current_block != NULL)
    {
        //On récupère le premier bloc qui à la taille maximum suffisante pour stocker notre bloc alloué.
        if (size <= current_block->size && current_block->size > worst_block->size)
        {
            worst_block = current_block;
        }

        current_block = current_block->next;
    }

    if (size <= worst_block->size){
        return worst_block;
    }

    return NULL;
}
