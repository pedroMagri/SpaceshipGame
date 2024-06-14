#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <semaphore.h>
#include "game.h"
#include "globals.h"

// Definição das variáveis globais
Nave naves[MAX_NAVES];
Foguete foguetes[MAX_FOGUETES];
Lancador lancador;
int num_naves;
int num_foguetes;
int naves_abatidas = 0;
int naves_atingiram_solo = 0;
int k_foguetes;

sem_t sem_foguetes;
sem_t sem_recarga;
sem_t sem_tela;

void inicializa_jogo(int grau_dificuldade) {
    num_naves = (grau_dificuldade + 1) * 10;
    num_foguetes = 0;
    k_foguetes = (grau_dificuldade + 1) * 5;
    naves_abatidas = 0;
    naves_atingiram_solo = 0;

    srand(time(NULL));
    for (int i = 0; i < num_naves; i++) {
        naves[i].x = rand() % COLS;
        naves[i].y = 0;
        naves[i].ativa = 1;
    }

    for (int i = 0; i < k_foguetes; i++) {
        foguetes[i].ativa = 0;
    }

    lancador.x = COLS / 2;
    lancador.y = LINES - 1;

    sem_init(&sem_foguetes, 0, 1);
    sem_init(&sem_recarga, 0, 1);
    sem_init(&sem_tela, 0, 1);
}

void* thread_principal(void* arg) {
    while (1) {
        sem_wait(&sem_tela);
        if (naves_abatidas >= num_naves / 2) {
            mvprintw(0, 0, "Vitoria!");
            refresh();
            sem_post(&sem_tela);
            break;
        }
        if (naves_atingiram_solo >= num_naves / 2) {
            mvprintw(0, 0, "Derrota!");
            refresh();
            sem_post(&sem_tela);
            break;
        }
        sem_post(&sem_tela);
        usleep(100000);
    }
    return NULL;
}

void* thread_entrada_jogador(void* arg) {
    int ch;
    while ((ch = getch()) != 'q') {
        sem_wait(&sem_foguetes);
        switch (ch) {
            case KEY_LEFT:
                if (lancador.x > 0)
                    lancador.x--;
                break;
            case KEY_RIGHT:
                if (lancador.x < COLS - 1)
                    lancador.x++;
                break;
            case ' ':
                if (num_foguetes < k_foguetes) {
                    foguetes[num_foguetes].x = lancador.x;
                    foguetes[num_foguetes].y = lancador.y;
                    foguetes[num_foguetes].ativa = 1;
                    num_foguetes++;
                }
                break;
        }
        sem_post(&sem_foguetes);
    }
    return NULL;
}

void* thread_movimentacao_naves(void* arg) {
    while (1) {
        sem_wait(&sem_tela);
        for (int i = 0; i < num_naves; i++) {
            if (naves[i].ativa) {
                naves[i].y++;
                if (naves[i].y >= LINES) {
                    naves[i].ativa = 0;
                    naves_atingiram_solo++;
                }
            }
        }
        sem_post(&sem_tela);
        usleep(500000);
    }
    return NULL;
}

void* thread_controle_foguetes(void* arg) {
    while (1) {
        sem_wait(&sem_tela);
        for (int i = 0; i < num_foguetes; i++) {
            if (foguetes[i].ativa) {
                foguetes[i].y--;
                if (foguetes[i].y < 0) {
                    foguetes[i].ativa = 0;
                } else {
                    for (int j = 0; j < num_naves; j++) {
                        if (naves[j].ativa && foguetes[i].x == naves[j].x && foguetes[i].y == naves[j].y) {
                            naves[j].ativa = 0;
                            foguetes[i].ativa = 0;
                            naves_abatidas++;
                            break;
                        }
                    }
                }
            }
        }
        sem_post(&sem_tela);
        usleep(100000);
    }
    return NULL;
}

void* thread_recarga(void* arg) {
    while (1) {
        sem_wait(&sem_recarga);
        usleep(5000000);
        sem_wait(&sem_foguetes);
        num_foguetes = 0;
        for (int i = 0; i < k_foguetes; i++) {
            foguetes[i].ativa = 0;
        }
        sem_post(&sem_foguetes);
        sem_post(&sem_recarga);
    }
    return NULL;
}

void atualiza_tela() {
    sem_wait(&sem_tela);
    clear();
    // Desenhar naves
    for (int i = 0; i < num_naves; i++) {
        if (naves[i].ativa) {
            mvprintw(naves[i].y, naves[i].x, "N");
        }
    }
    // Desenhar foguetes
    for (int i = 0; i < num_foguetes; i++) {
        if (foguetes[i].ativa) {
            mvprintw(foguetes[i].y, foguetes[i].x, "|");
        }
    }
    // Desenhar lançador
    mvprintw(lancador.y, lancador.x, "A");
    // Informações do jogo
    mvprintw(0, 0, "Foguetes: %d/%d", num_foguetes, k_foguetes);
    mvprintw(1, 0, "Naves abatidas: %d", naves_abatidas);
    mvprintw(2, 0, "Naves atingiram o solo: %d", naves_atingiram_solo);
    refresh();
    sem_post(&sem_tela);
}

int main() {
    int grau_dificuldade = 1; // Fácil
    pthread_t t_principal, t_entrada, t_naves, t_foguetes, t_recarga;

    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);

    inicializa_jogo(grau_dificuldade);

    pthread_create(&t_principal, NULL, thread_principal, NULL);
    pthread_create(&t_entrada, NULL, thread_entrada_jogador, NULL);
    pthread_create(&t_naves, NULL, thread_movimentacao_naves, NULL);
    pthread_create(&t_foguetes, NULL, thread_controle_foguetes, NULL);
    pthread_create(&t_recarga, NULL, thread_recarga, NULL);

    while (1) {
        atualiza_tela();
        usleep(100000);
    }

    pthread_join(t_principal, NULL);
    pthread_join(t_entrada, NULL);
    pthread_join(t_naves, NULL);
    pthread_join(t_foguetes, NULL);
    pthread_join(t_recarga, NULL);

    endwin();

    sem_destroy(&sem_foguetes);
    sem_destroy(&sem_recarga);
    sem_destroy(&sem_tela);

    return 0;
}
