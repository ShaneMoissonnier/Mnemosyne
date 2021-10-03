
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
    g_head->fb_head = first_block;

    mem_fit(&mem_first_fit);
    return;
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
void *mem_alloc(size_t size)
{
    // On aligne la taille demandé par l'utilisateur
    size_t aligned_size = get_align(size);
    // On calcule la taille totale de notre bloc alloué (entête comprise)
    size_t bb_block_size = aligned_size + sizeof(fb);

    // On trouve la zone libre qui pourrait contenir notre bloc alloué
    fb *f_b = g_head->fit_func(g_head->fb_head, bb_block_size);

    // Si on n'a pas trouvé de bloc libre
    if (f_b == NULL)
        return NULL;

    // On calcule la taille totale de notre zone libre trouvé (entête comprise)
    size_t fb_block_size = f_b->size;

    // On récupère le dernier bloc alloué se situant juste avant notre zone libre (last_busy_block)
    fb *current_block = g_head->bb_head;
    fb *last_busy_block = g_head->bb_head;

    while (current_block != NULL && current_block < f_b)
    {
        last_busy_block = current_block;
        current_block = current_block->next;
    }

    // Si le résidu restant ne peut contenir la taille d'une entête + un entier
    // alors on augmente la taille du bloc que l'on va allouer par la taille du résidu.
    fb *new_head;

    if (fb_block_size - bb_block_size < sizeof(fb) + sizeof(size_t))
    {
        bb_block_size += fb_block_size - bb_block_size;
        new_head = f_b->next;
    }
    else
    {
        // Sinon on peut scinder notre bloc libre
        fb *new_fb = (fb *)((void *)f_b + bb_block_size);

        *new_fb = (fb){
            f_b->size - bb_block_size,
            f_b->next};

        new_head = new_fb;
    }

    // Si le bloc libre que l'on vient d'allouer était le bloc de tête alors on change le bloc de tête
    if (f_b == g_head->fb_head)
        g_head->fb_head = new_head;

    // On définit la taille du bloc alloué
    f_b->size = bb_block_size;

    // Si la liste chaîné de blocs alloués est vide, ce nouveau bloc (occupé) en devient la tête
    if (last_busy_block == NULL)
    {
        g_head->bb_head = f_b;
        return (void *)f_b + sizeof(fb);
    }

    // Si le dernier bloc alloué se situe avant notre nouveau bloc alloué
    if (last_busy_block < f_b)
    {
        void *temp = last_busy_block->next;
        last_busy_block->next = f_b;
        f_b->next = (fb *)temp;
    }
    else
    {
        g_head->bb_head = f_b;
        f_b->next = last_busy_block;
    }
    return (void *)f_b + sizeof(fb);
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
void mem_free(void *zone)
{
    return;
}

//-------------------------------------------------------------
// Itérateur(parcours) sur le contenu de l'allocateur
// mem_show
//-------------------------------------------------------------
void mem_show(void (*print)(void *, size_t, int free))
{
    void *mem_end = get_memory_adr() + get_memory_size();
    void *current_block = (void *)(get_memory_adr() + sizeof(header));

    fb *free_block = g_head->fb_head;

    // On parcourt les blocs mémoires jusqu'à l'adresse de fin de mémoire
    while (current_block < mem_end)
    {
        // On récupère le type de la zone (libre (1) ou occupée (0)) ainsi que sa taille
        size_t block_state = (current_block == free_block);
        size_t size = ((fb *)current_block)->size;

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

    if (size <= best_block->size)
    {
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

    if (size <= worst_block->size)
    {
        return worst_block;
    }

    return NULL;
}
