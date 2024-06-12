#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>

// Variáveis globais
extern int dificuldade;
extern int num_foguetes;
extern int num_naves;
extern int foguetes_disponiveis;
extern Nave *naves;
extern Foguete *foguetes;
extern pthread_mutex_t mutex;
extern pthread_cond_t cond_recarga;

#endif // GLOBALS_H
