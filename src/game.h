#ifndef GAME_H
#define GAME_H

// Definições de constantes
#define NUM_POSICOES 5
#define VELOCIDADE_FOGUETE 1

// Definições de tipos
typedef struct {
    int x, y;
    int ativa;
} Nave;

typedef struct {
    int x, y;
    int ativa;
} Foguete;

// Funções auxiliares
void inicializa_jogo(int dificuldade);

#endif // GAME_H
