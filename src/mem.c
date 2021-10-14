
#include "mem.h"
#include "common.h"
#include "mem_os.h"
#include <stdio.h>

//-------------------------------------------------------------
// get_head
//-------------------------------------------------------------
header *get_head()
{
    return (header *)get_memory_adr();
}

//-------------------------------------------------------------
// mem_init
//-------------------------------------------------------------
void mem_init()
{
    void *mem_adr = get_memory_adr();
    size_t mem_size = get_memory_size();

    // On récupère l'entête qui stockera nos informations globales
    header *g_head = get_head();

    // On crée le premier bloc libre qui occupe "toute" la mémoire
    fb *first_block = (fb *)(mem_adr + sizeof(header) + sizeof(fb));

    // On crée notre élément fictif en début de tas
    fb *fake_block = (fb *)(mem_adr + sizeof(header));

    // On initialise la taille du bloc libre à la mémoire restante
    first_block->size = mem_size - sizeof(header) - sizeof(fb);
    first_block->next = NULL;

    // On initialise l'entête
    g_head->fb_head = fake_block;

    // On initialise notre élément fictif
    fake_block->size = sizeof(fb);
    fake_block->next = first_block;

    mem_fit(&mem_first_fit);
    return;
}

//-------------------------------------------------------------
// mem_alloc
//-------------------------------------------------------------
void *mem_alloc(size_t size)
{
    // On récupère l'entête qui stockera nos informations globales
    header *g_head = get_head();

    // On aligne la taille demandé par l'utilisateur
    size_t aligned_size = get_align(size);
    // On calcule la taille totale de notre bloc alloué (entête compris)
    size_t bb_size = aligned_size + sizeof(bb);

    // On trouve la zone libre qui pourrait contenir notre bloc alloué
    fb *precedent_fb = g_head->fit_func(g_head->fb_head, bb_size);

    // Si on n'a pas trouvé de bloc libre ou bien si la valeur demandée par l'utilisateur n'est pas positive
    if (precedent_fb == NULL || (int)size < 0)
        return NULL;

    fb *current_fb = precedent_fb->next;

    // On calcule la taille totale de notre zone libre trouvé (entête compris)
    size_t fb_size = current_fb->size;

    // Si le résidu restant ne peut contenir la taille d'un entête + un entier
    // alors on augmente la taille du bloc que l'on va allouer par la taille du résidu.
    fb *new_block;

    if (fb_size - bb_size < sizeof(fb) + sizeof(size_t))
    {
        bb_size += fb_size - bb_size;
        new_block = current_fb->next;
    }
    else
    {
        // Sinon on peut scinder notre bloc libre
        fb *new_fb = (fb *)((void *)current_fb + bb_size);

        *new_fb = (fb){
            fb_size - bb_size,
            current_fb->next};

        new_block = new_fb;
    }

    // Si le bloc libre que l'on vient d'allouer était le bloc de tête alors on change le bloc de tête
    if (current_fb == g_head->fb_head->next)
    {
        g_head->fb_head->next = new_block;
    }
    else
    {
        precedent_fb->next = new_block;
    }

    // On définit la taille du bloc alloué ainsi que son pointeur (pour les vérifications d'accès invalides)
    bb *busy_block = (bb *)current_fb;
    busy_block->size = bb_size;
    busy_block->ptr = (void *)busy_block;

    return (void *)busy_block + sizeof(bb);
}

//-------------------------------------------------------------
// mem_copy_data
//-------------------------------------------------------------
void mem_copy_data(void *source, void *destination)
{
    int i;
    char *src = (char *)source + sizeof(bb);
    char *dest = (char *)destination + sizeof(bb);

    size_t src_size = ((bb *)source)->size - sizeof(bb);
    size_t dest_size = ((bb *)destination)->size - sizeof(bb);

    for (i = 0; i < src_size && i < dest_size; i++)
    {
        dest[i] = src[i];
    }
}

//-------------------------------------------------------------
// mem_realloc
//-------------------------------------------------------------
void *mem_realloc(void *zone, size_t size)
{
    // Dans le cas où zone NULL alors mem_realloc agit comme mem_alloc (d'après la specification de realloc)
    if (zone == NULL)
        return mem_alloc(size);

    header *g_head = get_head();

    bb *current_bb = (bb *)(zone - sizeof(bb));

    size_t aligned_size = get_align(size);

    fb *current_fb = g_head->fb_head;
    fb *last_fb = g_head->fb_head;

    while (current_fb != NULL && (void *)current_fb < (void *)current_bb)
    {
        last_fb = current_fb;
        current_fb = current_fb->next;
    }

    size_t corrupt = (void *)current_bb < get_memory_adr() || (void *)current_bb > get_memory_adr() + get_memory_size() || !(current_bb->ptr == current_bb);
    if (corrupt)
    {
        printf("Erreur accès \n");
        return NULL;
    }

    size_t current_bb_size = current_bb->size - sizeof(bb);
    size_t new_current_bb_size = aligned_size + sizeof(bb);

    // Si notre bloc alloué peut déjà contenir la taille que l'utilisateur demande
    if (current_bb_size >= aligned_size)
    {
        // On scinde le reste du bloc alloué pour créer un bloc libre
        if (current_bb->size - new_current_bb_size > sizeof(fb) + sizeof(size_t))
        {
            fb *new_fb = (fb *)((void *)current_bb + new_current_bb_size);

            *new_fb = (fb){
                current_bb->size - new_current_bb_size,
                current_fb};

            last_fb->next = new_fb;

            // On peut fusionner le bloc que l'on vient de scinder avec le bloc suivant qui est libre
            if (current_fb != NULL && (void *)new_fb + new_fb->size == (void *)current_fb)
            {
                new_fb->size += current_fb->size;
                new_fb->next = current_fb->next;
            }

            // on change la taille de notre bloc alloué avec notre nouvelle taille
            current_bb->size = new_current_bb_size;
        }

        return zone;
    }

    // Si le bloc suivant est libre et que la taille ajouté à notre bloc alloué est suffisante pour stocker la taille demandée
    if (current_fb != NULL && (void *)current_bb + current_bb->size == (void *)current_fb && (current_fb->size - sizeof(fb)) + (current_bb->size - sizeof(bb)) >= aligned_size)
    {
        current_bb->size += current_fb->size;
        fb *new_block = current_fb->next;

        // On scinde le reste du bloc alloué pour créer un bloc libre
        if (current_bb->size - new_current_bb_size > sizeof(fb) + sizeof(size_t))
        {
            fb *new_fb = (fb *)((void *)current_bb + aligned_size + sizeof(bb));

            *new_fb = (fb){
                current_bb->size - new_current_bb_size,
                current_fb->next};

            new_block = new_fb;

            current_bb->size = new_current_bb_size;
        }

        last_fb->next = new_block;

        return zone;
    }

    // Sinon on fait un alloc classique et on copie nos données (partie simple)
    void *new_block = mem_alloc(aligned_size);

    if (new_block == NULL)
        return NULL;

    bb *new_bb = (bb *)(new_block - sizeof(bb));

    mem_copy_data(current_bb, new_bb);

    mem_free(zone);

    return (void *)new_bb + sizeof(bb);
}

//-------------------------------------------------------------
// mem_remove_data
//-------------------------------------------------------------
void mem_remove_data(void *block)
{
    int i;
    char *bb_block_data = (char *)block + sizeof(bb);
    size_t bb_block_size = ((bb *)block)->size - sizeof(bb);

    // On supprime les données contenues dans la partie retournée à l'utilisateur
    for (i = 0; i < bb_block_size; i++)
    {
        bb_block_data[i] = 0;
    }
}

//-------------------------------------------------------------
// mem_free
//-------------------------------------------------------------
void mem_free(void *zone)
{
    // On récupère l'entête qui stockera nos informations globales
    header *g_head = get_head();

    bb *current_bb = (bb *)(zone - sizeof(bb));

    fb *current_fb = g_head->fb_head;
    fb *last_fb = g_head->fb_head;

    while (current_fb != NULL && (void *)current_fb < (void *)current_bb)
    {
        last_fb = current_fb;
        current_fb = current_fb->next;
    }

    size_t corrupt = (void *)current_bb < get_memory_adr() || (void *)current_bb > get_memory_adr() + get_memory_size() || !(current_bb->ptr == current_bb);
    if (corrupt)
    {
        printf("Erreur accès \n");
        return;
    }

    // On supprime par sécurité les données utilisateur contenues dans le bloc que l'on va libérer
    mem_remove_data((void *)current_bb);

    fb *new_block = (fb *)current_bb;

    // On réordonne notre liste chaînée de zones libres
    fb *temp = last_fb->next;
    last_fb->next = new_block;
    new_block->next = temp;

    // On fusionne nos blocs libres si nécessaire
    size_t need_fusion_left = (last_fb != g_head->fb_head && (void *)last_fb + last_fb->size == (void *)new_block);
    size_t need_fusion_right = ((void *)new_block + new_block->size == (void *)new_block->next);

    if (need_fusion_right)
    {
        new_block->size += new_block->next->size;
        new_block->next = new_block->next->next;
    }

    if (need_fusion_left)
    {
        last_fb->size += new_block->size;
        last_fb->next = new_block->next;
    }

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

    fb *free_block = get_head()->fb_head;

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
    get_head()->fit_func = mff;
    return;
}

//-------------------------------------------------------------
// Stratégies d'allocation
//-------------------------------------------------------------
struct fb *mem_first_fit(struct fb *head, size_t size)
{
    fb *current_block = head->next;
    fb *last_block = head;

    while (current_block != NULL)
    {
        // On récupère le premier bloc libre pouvant stocker notre bloc alloué
        if (size <= current_block->size)
        {
            return last_block;
        }

        last_block = current_block;
        current_block = current_block->next;
    }
    return NULL;
}
//-------------------------------------------------------------
struct fb *mem_best_fit(struct fb *head, size_t size)
{
    fb *current_block = head;
    fb *before_best_block = head;

    while (current_block->next != NULL)
    {
        //On récupère le bloc précédent le premier bloc qui à la taille minimum suffisante pour stocker notre bloc alloué.
        if (current_block->next->size >= size && (current_block->next->size < before_best_block->next->size || before_best_block->next->size < size))
        {
            before_best_block = current_block;
        }
        current_block = current_block->next;
    }
    //On vérifie qu'on a un bloc valide, et pas la valeur par défaut
    if (before_best_block->next != NULL && size <= before_best_block->next->size)
    {
        return before_best_block;
    }

    return NULL;
}
//-------------------------------------------------------------
struct fb *mem_worst_fit(struct fb *head, size_t size)
{
    fb *current_block = head;
    fb *before_worst_block = head;

    while (current_block->next != NULL)
    {
        //On récupère le premier bloc qui à la taille maximum suffisante pour stocker notre bloc alloué.
        if (size <= current_block->next->size && current_block->next->size > before_worst_block->next->size)
        {
            before_worst_block = current_block;
        }
        current_block = current_block->next;
    }
    //On vérifie qu'on a un bloc valide, et pas la valeur par défaut
    if (before_worst_block->next != NULL && size <= before_worst_block->next->size)
    {
        return before_worst_block;
    }

    return NULL;
}