#ifndef THREADS_H
#define THREADS_H

void* thread_principal();
void* thread_entrada_jogador();
void* thread_movimentacao_naves();
void* thread_controle_foguetes();
void* thread_recarga();

#endif
