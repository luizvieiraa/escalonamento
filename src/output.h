#ifndef OUTPUT_H
#define OUTPUT_H

#include <stddef.h>

#include "scheduler.h"

int gravar_saida_simulacao(const char *caminho,
                           AlgoritmoEscalonamento algoritmo,
                           const EntradaSimulacao *entrada,
                           const ResumoSimulacao *resumo,
                           char *mensagem_erro,
                           size_t tamanho_mensagem_erro);

#endif
