#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>

#include "parser.h"

typedef enum {
    ESCALONADOR_RATE,
    ESCALONADOR_EDF
} AlgoritmoEscalonamento;

typedef enum {
    TRECHO_EM_EXECUCAO,
    TRECHO_FINALIZADO,
    TRECHO_INTERROMPIDO,
    TRECHO_DEADLINE_PERDIDO,
    TRECHO_MORTO,
    TRECHO_OCIOSO
} SituacaoTrecho;

typedef struct {
    size_t indice_tarefa;
    size_t identificador_instancia;
    int duracao;
    SituacaoTrecho situacao;
} TrechoExecucao;

typedef struct {
    int tempo_execucao_cpu;
    int tempo_ocioso;
    size_t quantidade_tarefas;
    size_t *conclusoes_por_tarefa;
    size_t *deadlines_perdidos_por_tarefa;
    size_t *pendencias_por_tarefa;
    TrechoExecucao *historico;
    size_t quantidade_trechos;
} ResumoSimulacao;

int simular_escalonamento(const EntradaSimulacao *entrada,
                          AlgoritmoEscalonamento algoritmo,
                          ResumoSimulacao *resumo,
                          char *mensagem_erro,
                          size_t tamanho_mensagem_erro);
void liberar_resumo_simulacao(ResumoSimulacao *resumo);

int executar_escalonador(AlgoritmoEscalonamento algoritmo,
                         const char *caminho_entrada);

#endif
