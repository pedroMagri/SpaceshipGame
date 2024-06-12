#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include "game.h"
#include "threads.h"
#include "globals.h"

// Definições das variáveis globais
int dificuldade;
int num_foguetes;
int num_naves;
int foguetes_disponiveis;
Nave *naves;
Foguete *foguetes;
pthread_mutex_t mutex;
pthread_cond_t cond_recarga;

int main() {
    int grau_dificuldade;
    pthread_t t_principal, t_entrada, t_naves, t_foguetes, t_recarga;

    // Inicializa ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);

    // Configurações iniciais
    grau_dificuldade = 1; // Fácil
    inicializa_jogo(grau_dificuldade);

    // Criação das threads
    pthread_create(&t_principal, NULL, thread_principal, NULL);
    pthread_create(&t_entrada, NULL, thread_entrada_jogador, NULL);
    pthread_create(&t_naves, NULL, thread_movimentacao_naves, NULL);
    pthread_create(&t_foguetes, NULL, thread_controle_foguetes, NULL);
    pthread_create(&t_recarga, NULL, thread_recarga, NULL);

    // Aguarda as threads
    pthread_join(t_principal, NULL);
    pthread_join(t_entrada, NULL);
    pthread_join(t_naves, NULL);
    pthread_join(t_foguetes, NULL);
    pthread_join(t_recarga, NULL);

    // Finaliza ncurses
    endwin();

    return 0;
}

void inicializa_jogo(int dificuldade) {
    switch (dificuldade) {
        case 1: // Fácil
            num_foguetes = 10;
            num_naves = 5;
            break;
        case 2: // Médio
            num_foguetes = 7;
            num_naves = 10;
            break;
        case 3: // Difícil
            num_foguetes = 5;
            num_naves = 15;
            break;
        default:
            num_foguetes = 10;
            num_naves = 5;
            break;
    }
    foguetes_disponiveis = num_foguetes;
    naves = (Nave *)malloc(num_naves * sizeof(Nave));
    foguetes = (Foguete *)malloc(num_foguetes * sizeof(Foguete));
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_recarga, NULL);

    // Inicializa as naves em posições aleatórias no topo da tela
    srand(time(NULL));
    for (int i = 0; i < num_naves; i++) {
        naves[i].x = rand() % COLS; // COLS é o número de colunas da tela (definido pelo ncurses)
        naves[i].y = 0;
        naves[i].ativa = 1;
    }

    // Inicializa os foguetes como inativos
    for (int i = 0; i < num_foguetes; i++) {
        foguetes[i].ativa = 0;
    }
}

void *thread_principal(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        int naves_abatidas = 0;
        int naves_no_solo = 0;

        for (int i = 0; i < num_naves; i++) {
            if (!naves[i].ativa) {
                if (naves[i].y >= LINES) {
                    naves_no_solo++;
                } else {
                    naves_abatidas++;
                }
            }
        }

        if (naves_abatidas >= num_naves / 2) {
            // Vitória do jogador
            break;
        }
        if (naves_no_solo > num_naves / 2) {
            // Derrota do jogador
            break;
        }

        pthread_mutex_unlock(&mutex);
        usleep(50000); // Espera por 50 ms
    }
    pthread_exit(NULL);
}

void *thread_entrada_jogador(void *arg) {
    while (1) {
        int ch = getch();
        pthread_mutex_lock(&mutex);
        // Processa entrada do jogador
        if (ch == ' ') {
            // Disparar foguete
            for (int i = 0; i < num_foguetes; i++) {
                if (!foguetes[i].ativa && foguetes_disponiveis > 0) {
                    foguetes[i].x = COLS / 2; // Posição inicial do lançador
                    foguetes[i].y = LINES - 1; // Posição inicial do lançador
                    foguetes[i].ativa = 1;
                    foguetes_disponiveis--;
                    break;
                }
            }
        } else if (ch == 'r') {
            // Solicitar recarga
            pthread_cond_signal(&cond_recarga);
        }
        pthread_mutex_unlock(&mutex);
        usleep(50000); // Espera por 50 ms
    }
    pthread_exit(NULL);
}

void *thread_movimentacao_naves(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < num_naves; i++) {
            if (naves[i].ativa) {
                naves[i].y += 1; // Move a nave para baixo
                if (naves[i].y >= LINES) { // LINES é o número de linhas da tela (definido pelo ncurses)
                    // A nave atingiu o solo
                    naves[i].ativa = 0;
                    // Lógica para verificar condição de derrota
                }
            }
        }
        pthread_mutex_unlock(&mutex);
        usleep(50000); // Espera por 50 ms
    }
    pthread_exit(NULL);
}

void *thread_controle_foguetes(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < num_foguetes; i++) {
            if (foguetes[i].ativa) {
                foguetes[i].y -= VELOCIDADE_FOGUETE; // Move o foguete para cima
                if (foguetes[i].y < 0) {
                    foguetes[i].ativa = 0; // Desativa o foguete se ele sair da tela
                } else {
                    // Verifica colisão com naves
                    for (int j = 0; j < num_naves; j++) {
                        if (naves[j].ativa && naves[j].x == foguetes[i].x && naves[j].y == foguetes[i].y) {
                            naves[j].ativa = 0; // Abate a nave
                            foguetes[i].ativa = 0; // Desativa o foguete
                            break;
                        }
                    }
                }
            }
        }
        pthread_mutex_unlock(&mutex);
        usleep(50000); // Espera por 50 ms
    }
    pthread_exit(NULL);
}

void *thread_recarga(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        // Espera por sinal de recarga
        pthread_cond_wait(&cond_recarga, &mutex);
        // Recarga de foguetes
        foguetes_disponiveis = num_foguetes;
        pthread_mutex_unlock(&mutex);
        usleep(50000); // Espera por 50 ms
    }
    pthread_exit(NULL);
}
