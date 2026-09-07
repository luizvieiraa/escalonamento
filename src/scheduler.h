#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>

#include "parser.h"

typedef enum {
    ESCALONADOR_RATE,
    ESCALONADOR_EDF
} AlgoritmoEscalonamento;

typedef struct {
    int tempo_execucao_cpu;
    int tempo_ocioso;
    size_t quantidade_tarefas;
    size_t *conclusoes_por_tarefa;
    size_t *pendencias_por_tarefa;
} ResumoSimulacao;

int simular_nucleo_basico(const EntradaSimulacao *entrada,
                          ResumoSimulacao *resumo,
                          char *mensagem_erro,
                          size_t tamanho_mensagem_erro);
void liberar_resumo_simulacao(ResumoSimulacao *resumo);

int executar_escalonador(AlgoritmoEscalonamento algoritmo,
                         const char *caminho_entrada);

#endif
