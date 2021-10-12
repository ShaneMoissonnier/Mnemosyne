#if !defined(mem_os_h)
#define mem_os_h

/* -----------------------------------------------*/
/* Interface de gestion de votre allocateur       */
/* -----------------------------------------------*/
// Initialisation
void mem_init(void);

// Définition du type mem_fit_function_t
// type des fonctions d'allocation
typedef struct fb *(mem_fit_function_t)(struct fb *, size_t);

// Choix de la fonction d'allocation
// = choix de la stratégie de l'allocation
void mem_fit(mem_fit_function_t *);

// Stratégies de base (fonctions) d'allocation
mem_fit_function_t mem_first_fit;
mem_fit_function_t mem_worst_fit;
mem_fit_function_t mem_best_fit;

// Structure de gestion d'un bloc libre
typedef struct fb fb;
struct fb
{
    size_t size;
    fb *next;
};

typedef struct bb bb;
struct bb
{
    size_t size;
    bb *ptr;
};

// Structure de l'entête au début du tas
typedef struct
{
    mem_fit_function_t *fit_func;
    fb *fb_head;
} header;

// Permet de récupérer l'entête
header *get_head();

enum HeadType
{
    BUSY,
    FREE
};

#endif /* mem_os_h */
