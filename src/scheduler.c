#include "scheduler.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    CAPACIDADE_INICIAL_INSTANCIAS = 16
};

typedef struct {
    size_t indice_tarefa;
    int instante_chegada;
    int tempo_restante;
    int ativa;
} InstanciaExecucao;

typedef struct {
    InstanciaExecucao *itens;
    size_t quantidade;
    size_t capacidade;
} VetorInstancias;

static int adicionar_instancia(VetorInstancias *instancias,
                               const InstanciaExecucao *nova_instancia)
{
    if (instancias->quantidade == instancias->capacidade) {
        size_t nova_capacidade;
        InstanciaExecucao *novos_itens;

        if (instancias->capacidade == 0) {
            nova_capacidade = CAPACIDADE_INICIAL_INSTANCIAS;
        } else {
            if (instancias->capacidade > (size_t)-1 / 2) {
                return 0;
            }
            nova_capacidade = instancias->capacidade * 2;
        }

        if (nova_capacidade > (size_t)-1 / sizeof(*novos_itens)) {
            return 0;
        }

        novos_itens = realloc(instancias->itens,
                              nova_capacidade * sizeof(*novos_itens));
        if (novos_itens == NULL) {
            return 0;
        }

        instancias->itens = novos_itens;
        instancias->capacidade = nova_capacidade;
    }

    instancias->itens[instancias->quantidade++] = *nova_instancia;
    return 1;
}

static void remover_instancias_inativas(VetorInstancias *instancias)
{
    size_t origem;
    size_t destino = 0;

    for (origem = 0; origem < instancias->quantidade; origem++) {
        if (instancias->itens[origem].ativa) {
            instancias->itens[destino++] = instancias->itens[origem];
        }
    }

    instancias->quantidade = destino;
}

static int preparar_resumo(size_t quantidade_tarefas,
                           ResumoSimulacao *resumo)
{
    resumo->tempo_execucao_cpu = 0;
    resumo->tempo_ocioso = 0;
    resumo->quantidade_tarefas = quantidade_tarefas;
    resumo->conclusoes_por_tarefa = NULL;
    resumo->pendencias_por_tarefa = NULL;

    if (quantidade_tarefas == 0) {
        return 1;
    }

    if (quantidade_tarefas > (size_t)-1 / sizeof(size_t)) {
        return 0;
    }

    resumo->conclusoes_por_tarefa = calloc(quantidade_tarefas, sizeof(size_t));
    resumo->pendencias_por_tarefa = calloc(quantidade_tarefas, sizeof(size_t));

    if (resumo->conclusoes_por_tarefa == NULL ||
        resumo->pendencias_por_tarefa == NULL) {
        liberar_resumo_simulacao(resumo);
        return 0;
    }

    return 1;
}

void liberar_resumo_simulacao(ResumoSimulacao *resumo)
{
    free(resumo->conclusoes_por_tarefa);
    free(resumo->pendencias_por_tarefa);
    resumo->conclusoes_por_tarefa = NULL;
    resumo->pendencias_por_tarefa = NULL;
    resumo->quantidade_tarefas = 0;
    resumo->tempo_execucao_cpu = 0;
    resumo->tempo_ocioso = 0;
}

int simular_nucleo_basico(const EntradaSimulacao *entrada,
                          ResumoSimulacao *resumo,
                          char *mensagem_erro,
                          size_t tamanho_mensagem_erro)
{
    VetorInstancias instancias = {NULL, 0, 0};
    int tempo;
    size_t indice;

    if (!preparar_resumo(entrada->quantidade_tarefas, resumo)) {
        snprintf(mensagem_erro, tamanho_mensagem_erro, "memoria insuficiente");
        return 0;
    }

    for (tempo = 0; tempo < entrada->tempo_total; tempo++) {
        size_t indice_instancia_escolhida;

        remover_instancias_inativas(&instancias);

        for (indice = 0; indice < entrada->quantidade_tarefas; indice++) {
            const DefinicaoTarefa *tarefa = &entrada->tarefas[indice];

            if (tempo % tarefa->periodo == 0) {
                InstanciaExecucao nova_instancia;

                nova_instancia.indice_tarefa = indice;
                nova_instancia.instante_chegada = tempo;
                nova_instancia.tempo_restante = tarefa->tempo_cpu;
                nova_instancia.ativa = 1;

                if (!adicionar_instancia(&instancias, &nova_instancia)) {
                    snprintf(mensagem_erro,
                             tamanho_mensagem_erro,
                             "memoria insuficiente");
                    free(instancias.itens);
                    liberar_resumo_simulacao(resumo);
                    return 0;
                }
            }
        }

        indice_instancia_escolhida = instancias.quantidade;
        for (indice = 0; indice < instancias.quantidade; indice++) {
            if (instancias.itens[indice].ativa) {
                indice_instancia_escolhida = indice;
                break;
            }
        }

        if (indice_instancia_escolhida < instancias.quantidade) {
            InstanciaExecucao *escolhida =
                &instancias.itens[indice_instancia_escolhida];

            escolhida->tempo_restante--;
            resumo->tempo_execucao_cpu++;

            if (escolhida->tempo_restante == 0) {
                escolhida->ativa = 0;
                resumo->conclusoes_por_tarefa[escolhida->indice_tarefa]++;
            }
        } else {
            resumo->tempo_ocioso++;
        }
    }

    for (indice = 0; indice < instancias.quantidade; indice++) {
        if (instancias.itens[indice].ativa) {
            resumo->pendencias_por_tarefa[instancias.itens[indice].indice_tarefa]++;
        }
    }

    free(instancias.itens);
    return 1;
}

int executar_escalonador(AlgoritmoEscalonamento algoritmo,
                         const char *caminho_entrada)
{
    EntradaSimulacao entrada;
    ResumoSimulacao resumo;
    char mensagem_erro[256];

    (void)algoritmo;

    if (!carregar_arquivo_entrada(caminho_entrada,
                                 &entrada,
                                 mensagem_erro,
                                 sizeof(mensagem_erro))) {
        fprintf(stderr, "Erro: %s.\n", mensagem_erro);
        return EXIT_FAILURE;
    }

    if (!simular_nucleo_basico(&entrada,
                               &resumo,
                               mensagem_erro,
                               sizeof(mensagem_erro))) {
        fprintf(stderr, "Erro: %s.\n", mensagem_erro);
        liberar_entrada(&entrada);
        return EXIT_FAILURE;
    }

    liberar_resumo_simulacao(&resumo);
    liberar_entrada(&entrada);
    return EXIT_SUCCESS;
}
