#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

typedef struct {
    char *nome;
    int periodo;
    int deadline_relativo;
    int tempo_cpu;
    size_t ordem_entrada;
} DefinicaoTarefa;

typedef struct {
    int tempo_total;
    DefinicaoTarefa *tarefas;
    size_t quantidade_tarefas;
} EntradaSimulacao;

int carregar_arquivo_entrada(const char *caminho,
                             EntradaSimulacao *entrada,
                             char *mensagem_erro,
                             size_t tamanho_mensagem_erro);
void liberar_entrada(EntradaSimulacao *entrada);

#endif
