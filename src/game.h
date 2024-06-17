#ifndef GAME_H
#define GAME_H

#include <pthread.h>
#include <ncurses.h>

void inicializa_jogo(int grau_dificuldade);
void* thread_principal();
void* thread_entrada_jogador();
void* thread_movimentacao_naves();
void* thread_controle_foguetes();
void* thread_recarga();
void atualiza_tela();

#endif // GAME_H
