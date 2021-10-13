#include "../common.h"
#include "../mem.h"
#include "../mem_os.h"

#include <stdio.h>

#include "CuTest.h"
#include "AllTests.h"

void afficher_tas(void *adresse, size_t taille, int free)
{
  switch (free)
  {
  case 0:
    printf("%11d |%11s |%11ld |%11d |%11ld\n", (int)(adresse - get_memory_adr()), "BUSY", taille, (int)((adresse + sizeof(bb)) - get_memory_adr()), taille - sizeof(bb));
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

void testAlloc(CuTest *tc)
{
  int block_size = 1024;

  // Si on execute un mem_alloc et puis un mem_free sur la zone alloué
  // alors par précaution toutes les données utilisateurs sont nettoyées (mem_remove_data dans mem_free)
  mem_init();
  void *ptr = mem_alloc(block_size);
  CuAssertPtrNotNull(tc, ptr);

  char *a = (char *)ptr;
  int i = 0;
  for (i = 0; i < block_size; i++)
  {
    a[i] = 'a';
  }

  mem_free(ptr);

  char *b = (char *)ptr;

  for (i = 0; i < block_size; i++)
  {
    CuAssert(tc, "Data must be erased", b[i] != 'a');
  }

  // Si on alloue un bloc de taille négative alors un bloc de taille 0 est alloué
  void *p = mem_alloc(-8);
  CuAssertPtrEquals_Msg(tc, "Pointer must be null", NULL, p);

  // On alloue des blocs et on libère des blocs au milieu de nos allocations
  int nb_alloc = 10;
  int nb_free = 2;
  void *allocs[nb_alloc];

  for (int i = 0; i < nb_alloc; i++)
  {
    allocs[i] = mem_alloc(block_size);
  }

  for (int i = nb_alloc / 2; i < nb_alloc / 2 + nb_free; i++)
  {
    mem_free(allocs[i]);
  }

  ptr = allocs[nb_alloc / 2];
  fb *free_block = (fb *)(ptr - sizeof(fb));

  size_t actual_value = free_block->size;
  size_t expected_value = nb_free * (block_size + sizeof(fb));

  CuAssertIntEquals_Msg(tc, "Free block size is not correct ", expected_value, actual_value);

  // On alloue des blocs et on libère des blocs à la fin de notre allocations

  mem_init();

  for (int i = 0; i < nb_alloc; i++)
  {
    allocs[i] = mem_alloc(block_size);
  }

  for (int i = nb_alloc - nb_free; i < nb_alloc; i++)
  {
    mem_free(allocs[i]);
  }

  ptr = allocs[nb_alloc - nb_free];
  free_block = (fb *)(ptr - sizeof(fb));

  actual_value = free_block->size;
  expected_value = get_memory_size() - sizeof(header) - sizeof(fb) - ((nb_alloc - nb_free) * (get_align(block_size) + sizeof(bb)));

  // afficher_entete();
  // mem_show(afficher_tas);

  CuAssertIntEquals_Msg(tc, "Free block size is not correct ", expected_value, actual_value);
}

void testRealloc(CuTest *tc)
{
  int block_size = 1024;

  mem_init();

  // Si on fait un realloc et que le bloc suivant n'est pas libre
  // on crée un nouveau bloc et on copie nos données
  void *ptr = mem_alloc(block_size);
  void *ptr2 = mem_alloc(block_size);
  void *ptr3 = mem_alloc(block_size);

  CuAssertPtrNotNull(tc, ptr);
  CuAssertPtrNotNull(tc, ptr2);
  CuAssertPtrNotNull(tc, ptr3);

  // On écris des données dans le premier bloc alloué (ptr)
  char *a = (char *)ptr;
  int i = 0;
  for (i = 0; i < block_size; i++)
  {
    a[i] = 'a';
  }

  void *ptr4 = mem_realloc(ptr, block_size + 2);

  char *b = (char *)ptr4;

  for (i = 0; i < block_size; i++)
  {
    CuAssert(tc, "Data must be the same ", b[i] == 'a');
  }

  mem_free(ptr2);
  mem_free(ptr3);
  mem_free(ptr4);

  // Si on essaie de réallouer une mémoire plus petite que celle déjà alloué
  // alors le bloc est redimensionné et si il nous reste de la place on crée un bloc libre
  // suivant notre zone alloué et on la fusionne avec la zone suivant cette zone si elle est libre

  mem_init();

  ptr = mem_alloc(block_size);
  ptr2 = mem_alloc(block_size);
  ptr3 = mem_alloc(block_size);

  ptr4 = mem_realloc(ptr, 6);

  void *begin_block_ptr4 = ptr4 - sizeof(bb);

  size_t bb_block_size = ((bb *)(begin_block_ptr4))->size;

  void *ptr5 = (void *)(begin_block_ptr4 + bb_block_size);

  size_t expected_value = block_size + sizeof(bb) - bb_block_size;

  size_t actual_value = ((fb *)ptr5)->size;

  // afficher_entete();
  // mem_show(afficher_tas);

  CuAssertIntEquals_Msg(tc, "Free block size not correct ", expected_value, actual_value);

  // Si on réalloue la taille identique
  mem_init();

  ptr = mem_alloc(block_size);
  ptr2 = mem_alloc(block_size);
  ptr3 = mem_alloc(block_size);

  ptr4 = mem_realloc(ptr, block_size);
  ptr5 = mem_realloc(ptr2, block_size);
  void *ptr6 = mem_realloc(ptr3, block_size);

  begin_block_ptr4 = ptr4 - sizeof(bb);
  size_t bb_block_size_ptr4 = ((bb *)(begin_block_ptr4))->size;

  void *begin_block_ptr5 = ptr5 - sizeof(bb);
  size_t bb_block_size_ptr5 = ((bb *)(begin_block_ptr5))->size;

  void *begin_block_ptr6 = ptr6 - sizeof(bb);
  size_t bb_block_size_ptr6 = ((bb *)(begin_block_ptr6))->size;

  CuAssertIntEquals_Msg(tc, "First realloc size is not correct ", block_size + sizeof(bb), bb_block_size_ptr4);
  CuAssertIntEquals_Msg(tc, "Second realloc size is not correct ", block_size + sizeof(bb), bb_block_size_ptr5);
  CuAssertIntEquals_Msg(tc, "Third realloc size is not correct ", block_size + sizeof(bb), bb_block_size_ptr6);
}

CuSuite *SuiteTest()
{
  CuSuite *suite = CuSuiteNew();
  SUITE_ADD_TEST(suite, testAlloc);
  SUITE_ADD_TEST(suite, testRealloc);
  return suite;
}

void RunAllTests(void)
{
  CuString *output = CuStringNew();
  CuSuite *suite = CuSuiteNew();

  CuSuiteAddSuite(suite, SuiteTest());

  CuSuiteRun(suite);
  CuSuiteDetails(suite, output);
  printf("%s\n", output->buffer);
}

int main(int argc, char *argv[])
{
  RunAllTests();
  return 0;
}