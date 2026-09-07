#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scheduler.h"

static int interpretar_algoritmo(const char *argumento,
                                 AlgoritmoEscalonamento *algoritmo)
{
    if (strcmp(argumento, "rate") == 0) {
        *algoritmo = ESCALONADOR_RATE;
        return 1;
    }

    if (strcmp(argumento, "edf") == 0) {
        *algoritmo = ESCALONADOR_EDF;
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    AlgoritmoEscalonamento algoritmo;

    if (argc != 3) {
        fprintf(stderr, "Uso: %s <rate|edf> <arquivo_entrada>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (!interpretar_algoritmo(argv[1], &algoritmo)) {
        fprintf(stderr,
                "Erro: algoritmo invalido '%s'; use 'rate' ou 'edf'.\n",
                argv[1]);
        return EXIT_FAILURE;
    }

    return executar_escalonador(algoritmo, argv[2]);
}
