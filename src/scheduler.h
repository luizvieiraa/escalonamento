#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef enum {
    ESCALONADOR_RATE,
    ESCALONADOR_EDF
} AlgoritmoEscalonamento;

int executar_escalonador(AlgoritmoEscalonamento algoritmo,
                         const char *caminho_entrada);

#endif
