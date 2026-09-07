#include "scheduler.h"

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

int executar_escalonador(AlgoritmoEscalonamento algoritmo,
                         const char *caminho_entrada)
{
    EntradaSimulacao entrada;
    char mensagem_erro[256];

    (void)algoritmo;

    if (!carregar_arquivo_entrada(caminho_entrada,
                                 &entrada,
                                 mensagem_erro,
                                 sizeof(mensagem_erro))) {
        fprintf(stderr, "Erro: %s.\n", mensagem_erro);
        return EXIT_FAILURE;
    }

    liberar_entrada(&entrada);
    return EXIT_SUCCESS;
}
