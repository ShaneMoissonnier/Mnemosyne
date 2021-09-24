# Mnemosyne (TP-Memoire)

Cette archive contient :
*  le sujet du TP : TD-TP-Memoire.pdf
*  les sources d'un squelette de votre allocateur : src/
*  des tests pour votre allocateur : src/tests/

## Contenu de src/
- `mem.h` et `mem_os.h` : l'interface de votre allocateur. 
  `mem.h` définit les fonctions utilisateur (`mem_alloc`, `mem_free`), 
   alors que `mem_os.h` définit les fonctions définissant la stratégie d'allocation
- `common.h` et `common.c` : définissent la mémoire à gérer et des fonctions utilitaires pour conna^tre sa taille et son adresse de début.
  Ces fonctions seront utilisées dans la fonction `mem_init`.
- `memshell.c` ; un interpreteur simple de commandes d'allocation et de libération 
   vous permettant de tester votre allocateur de manière intéractive
-`mem.c` : le squelette de votre allocateur, le fichier que vous devez modifier.
- `malloc_stub.h` et `malloc_stub.c` : utilisés pour la génération d'une bibliothèque permettant
  de remplacer la `libc` et de tester votre allocateur avec des programmes existant standard
- des fichiers de test : `test_*.c`
- `Makefile` simple
- des exemples de séquences courtes d'allocations et de libérations `alloc*.in`. Vous pouvez les passer en redirigeant l'entrée de votre memshell.
