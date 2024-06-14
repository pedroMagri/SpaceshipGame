#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
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
int game_over = 0;

pthread_mutex_t mutex;
pthread_cond_t cond_recarga;

// Enum para direções de disparo
typedef enum {
    VERTICAL,
    DIAGONAL_ESQUERDA,
    DIAGONAL_DIREITA,
    HORIZONTAL_ESQUERDA,
    HORIZONTAL_DIREITA
} Direcao;

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
    lancador.direcao = VERTICAL;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_recarga, NULL);
}

void* thread_principal(void* arg) {
    while (!game_over) {
        pthread_mutex_lock(&mutex);
        if (naves_abatidas >= num_naves / 2) {
            game_over = 1;
        }
        if (naves_atingiram_solo >= num_naves / 2) {
            game_over = 1;
        }
        pthread_mutex_unlock(&mutex);
        usleep(100000);
    }

    // Manter a mensagem na tela por 30 segundos
    clear();
    const char *message = (naves_abatidas >= num_naves / 2) ? "Vitoria!" : "Derrota!";
    int row = LINES / 2;
    int col = (COLS - strlen(message)) / 2;
    mvprintw(row, col, "%s", message);
    refresh();
    sleep(30);

    return NULL;
}

void* thread_entrada_jogador(void* arg) {
    int ch;
    while (!game_over && (ch = getch()) != 'q') {
        pthread_mutex_lock(&mutex);
        switch (ch) {
            case 'w': // Vertical
                lancador.direcao = VERTICAL;
                break;
            case 'a': // Diagonal esquerda
                lancador.direcao = DIAGONAL_ESQUERDA;
                break;
            case 'd': // Diagonal direita
                lancador.direcao = DIAGONAL_DIREITA;
                break;
            case 'z': // Horizontal esquerda
                lancador.direcao = HORIZONTAL_ESQUERDA;
                break;
            case 'c': // Horizontal direita
                lancador.direcao = HORIZONTAL_DIREITA;
                break;
            case ' ':
                if (num_foguetes < k_foguetes) {
                    foguetes[num_foguetes].x = lancador.x;
                    foguetes[num_foguetes].y = lancador.y;
                    foguetes[num_foguetes].direcao = lancador.direcao;
                    foguetes[num_foguetes].ativa = 1;
                    num_foguetes++;
                }
                break;
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* thread_movimentacao_naves(void* arg) {
    while (!game_over) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < num_naves; i++) {
            if (naves[i].ativa) {
                naves[i].y++;
                if (naves[i].y >= LINES) {
                    naves[i].ativa = 0;
                    naves_atingiram_solo++;
                }
            }
        }
        pthread_mutex_unlock(&mutex);
        usleep(500000);
    }
    return NULL;
}

void* thread_controle_foguetes(void* arg) {
    while (!game_over) {
        pthread_mutex_lock(&mutex);
        for (int i = 0; i < num_foguetes; i++) {
            if (foguetes[i].ativa) {
                // Movimentação dos foguetes baseado na direção
                switch (foguetes[i].direcao) {
                    case VERTICAL:
                        foguetes[i].y--;
                        break;
                    case DIAGONAL_ESQUERDA:
                        foguetes[i].y--;
                        foguetes[i].x--;
                        break;
                    case DIAGONAL_DIREITA:
                        foguetes[i].y--;
                        foguetes[i].x++;
                        break;
                    case HORIZONTAL_ESQUERDA:
                        foguetes[i].x--;
                        break;
                    case HORIZONTAL_DIREITA:
                        foguetes[i].x++;
                        break;
                }
                if (foguetes[i].y < 0 || foguetes[i].x < 0 || foguetes[i].x >= COLS) {
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
        pthread_mutex_unlock(&mutex);
        usleep(100000);
    }
    return NULL;
}

void* thread_recarga(void* arg) {
    while (!game_over) {
        pthread_mutex_lock(&mutex);
        if (num_foguetes == k_foguetes) {
            pthread_cond_wait(&cond_recarga, &mutex);
        }
        usleep(2000000);
        num_foguetes = 0;
        for (int i = 0; i < k_foguetes; i++) {
            foguetes[i].ativa = 0;
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void atualiza_tela() {
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

    while (!game_over) {
        pthread_mutex_lock(&mutex);
        atualiza_tela();
        pthread_mutex_unlock(&mutex);
        usleep(100000);
    }

}
