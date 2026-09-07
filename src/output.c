#include "output.h"

#include <stdarg.h>
#include <stdio.h>

static int escrever(FILE *arquivo, const char *formato, ...)
{
    int resultado;
    va_list argumentos;

    va_start(argumentos, formato);
    resultado = vfprintf(arquivo, formato, argumentos);
    va_end(argumentos);

    return resultado >= 0;
}

static char indicador_situacao(SituacaoTrecho situacao)
{
    switch (situacao) {
        case TRECHO_FINALIZADO:
            return 'F';
        case TRECHO_INTERROMPIDO:
            return 'H';
        case TRECHO_DEADLINE_PERDIDO:
            return 'L';
        case TRECHO_MORTO:
            return 'K';
        case TRECHO_EM_EXECUCAO:
        case TRECHO_OCIOSO:
            return '?';
    }

    return '?';
}

static int escrever_historico(FILE *arquivo,
                              const EntradaSimulacao *entrada,
                              const ResumoSimulacao *resumo)
{
    size_t indice;

    for (indice = 0; indice < resumo->quantidade_trechos; indice++) {
        const TrechoExecucao *trecho = &resumo->historico[indice];

        if (trecho->situacao == TRECHO_OCIOSO) {
            if (!escrever(arquivo, "idle for %d units\n", trecho->duracao)) {
                return 0;
            }
        } else {
            const char *nome = entrada->tarefas[trecho->indice_tarefa].nome;

            if (!escrever(arquivo,
                          "[%s] for %d units - %c\n",
                          nome,
                          trecho->duracao,
                          indicador_situacao(trecho->situacao))) {
                return 0;
            }
        }
    }

    return 1;
}

static int escrever_contadores(FILE *arquivo,
                               const char *titulo,
                               const EntradaSimulacao *entrada,
                               const size_t *contadores)
{
    size_t indice;

    if (!escrever(arquivo, "\n%s\n", titulo)) {
        return 0;
    }

    for (indice = 0; indice < entrada->quantidade_tarefas; indice++) {
        if (!escrever(arquivo,
                      "[%s] %zu\n",
                      entrada->tarefas[indice].nome,
                      contadores[indice])) {
            return 0;
        }
    }

    return 1;
}

int gravar_saida_simulacao(const char *caminho,
                           AlgoritmoEscalonamento algoritmo,
                           const EntradaSimulacao *entrada,
                           const ResumoSimulacao *resumo,
                           char *mensagem_erro,
                           size_t tamanho_mensagem_erro)
{
    FILE *arquivo = fopen(caminho, "w");
    int sucesso = 1;

    if (arquivo == NULL) {
        snprintf(mensagem_erro,
                 tamanho_mensagem_erro,
                 "nao foi possivel criar o arquivo '%s'",
                 caminho);
        return 0;
    }

    sucesso = escrever(arquivo,
                       "EXECUTION BY %s\n",
                       algoritmo == ESCALONADOR_RATE ? "RATE" : "EDF");
    sucesso = sucesso && escrever_historico(arquivo, entrada, resumo);
    sucesso = sucesso && escrever_contadores(arquivo,
                                             "LOST DEADLINES",
                                             entrada,
                                             resumo->deadlines_perdidos_por_tarefa);
    sucesso = sucesso && escrever_contadores(arquivo,
                                             "COMPLETE EXECUTION",
                                             entrada,
                                             resumo->conclusoes_por_tarefa);
    sucesso = sucesso && escrever_contadores(arquivo,
                                             "KILLED",
                                             entrada,
                                             resumo->pendencias_por_tarefa);

    if (fclose(arquivo) != 0) {
        sucesso = 0;
    }

    if (!sucesso) {
        remove(caminho);
        snprintf(mensagem_erro,
                 tamanho_mensagem_erro,
                 "nao foi possivel gravar o arquivo '%s'",
                 caminho);
        return 0;
    }

    return 1;
}
