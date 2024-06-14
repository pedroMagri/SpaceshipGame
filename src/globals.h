#ifndef GLOBALS_H
#define GLOBALS_H

#include <semaphore.h>

#define MAX_NAVES 100
#define MAX_FOGUETES 100

typedef struct {
    int x, y;
    int ativa;
} Nave;

typedef struct {
    int x, y;
    int ativa;
} Foguete;

typedef struct {
    int x, y;
} Lancador;

extern Nave naves[MAX_NAVES];
extern Foguete foguetes[MAX_FOGUETES];
extern Lancador lancador;
extern int num_naves;
extern int num_foguetes;
extern int naves_abatidas;
extern int naves_atingiram_solo;
extern int k_foguetes;

extern sem_t sem_foguetes;
extern sem_t sem_recarga;
extern sem_t sem_tela;

#endif // GLOBALS_H
