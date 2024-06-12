#ifndef THREADS_H
#define THREADS_H

// Funções de thread
void *thread_principal(void *arg);
void *thread_entrada_jogador(void *arg);
void *thread_movimentacao_naves(void *arg);
void *thread_controle_foguetes(void *arg);
void *thread_recarga(void *arg);

#endif // THREADS_H
