#include "parser.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    CAPACIDADE_INICIAL_LINHA = 128,
    CAPACIDADE_INICIAL_TAREFAS = 8
};

static void definir_erro(char *mensagem, size_t tamanho, const char *formato, ...)
{
    va_list argumentos;

    if (tamanho == 0) {
        return;
    }

    va_start(argumentos, formato);
    vsnprintf(mensagem, tamanho, formato, argumentos);
    va_end(argumentos);
}

static int ler_linha(FILE *arquivo, char **linha, size_t *capacidade)
{
    size_t comprimento = 0;
    int caractere;

    if (*linha == NULL) {
        *capacidade = CAPACIDADE_INICIAL_LINHA;
        *linha = malloc(*capacidade);
        if (*linha == NULL) {
            return -1;
        }
    }

    while ((caractere = fgetc(arquivo)) != EOF) {
        if (comprimento + 1 >= *capacidade) {
            size_t nova_capacidade;
            char *nova_linha;

            if (*capacidade > (size_t)-1 / 2) {
                return -1;
            }

            nova_capacidade = *capacidade * 2;
            nova_linha = realloc(*linha, nova_capacidade);
            if (nova_linha == NULL) {
                return -1;
            }

            *linha = nova_linha;
            *capacidade = nova_capacidade;
        }

        if (caractere == '\n') {
            break;
        }

        (*linha)[comprimento++] = (char)caractere;
    }

    if (ferror(arquivo)) {
        return -1;
    }

    if (caractere == EOF && comprimento == 0) {
        return 0;
    }

    (*linha)[comprimento] = '\0';
    return 1;
}

static size_t separar_campos(char *linha, char **campos, size_t maximo)
{
    size_t quantidade = 0;
    char *cursor = linha;

    while (*cursor != '\0') {
        while (isspace((unsigned char)*cursor)) {
            cursor++;
        }

        if (*cursor == '\0') {
            break;
        }

        if (quantidade == maximo) {
            return maximo + 1;
        }

        campos[quantidade++] = cursor;

        while (*cursor != '\0' && !isspace((unsigned char)*cursor)) {
            cursor++;
        }

        if (*cursor != '\0') {
            *cursor = '\0';
            cursor++;
        }
    }

    return quantidade;
}

static int interpretar_inteiro_positivo(const char *texto, int *valor)
{
    char *fim;
    long numero;

    errno = 0;
    numero = strtol(texto, &fim, 10);

    if (errno == ERANGE || *fim != '\0' || numero <= 0 || numero > INT_MAX) {
        return 0;
    }

    *valor = (int)numero;
    return 1;
}

static char *copiar_texto(const char *origem)
{
    size_t tamanho = strlen(origem) + 1;
    char *copia = malloc(tamanho);

    if (copia != NULL) {
        memcpy(copia, origem, tamanho);
    }

    return copia;
}

static int adicionar_tarefa(EntradaSimulacao *entrada,
                            size_t *capacidade,
                            const DefinicaoTarefa *tarefa)
{
    if (entrada->quantidade_tarefas == *capacidade) {
        size_t nova_capacidade;
        DefinicaoTarefa *novas_tarefas;

        if (*capacidade == 0) {
            nova_capacidade = CAPACIDADE_INICIAL_TAREFAS;
        } else {
            if (*capacidade > (size_t)-1 / 2) {
                return 0;
            }
            nova_capacidade = *capacidade * 2;
        }

        if (nova_capacidade > (size_t)-1 / sizeof(*novas_tarefas)) {
            return 0;
        }

        novas_tarefas = realloc(entrada->tarefas,
                                nova_capacidade * sizeof(*novas_tarefas));
        if (novas_tarefas == NULL) {
            return 0;
        }

        entrada->tarefas = novas_tarefas;
        *capacidade = nova_capacidade;
    }

    entrada->tarefas[entrada->quantidade_tarefas++] = *tarefa;
    return 1;
}

void liberar_entrada(EntradaSimulacao *entrada)
{
    size_t indice;

    for (indice = 0; indice < entrada->quantidade_tarefas; indice++) {
        free(entrada->tarefas[indice].nome);
    }

    free(entrada->tarefas);
    entrada->tarefas = NULL;
    entrada->quantidade_tarefas = 0;
    entrada->tempo_total = 0;
}

int carregar_arquivo_entrada(const char *caminho,
                             EntradaSimulacao *entrada,
                             char *mensagem_erro,
                             size_t tamanho_mensagem_erro)
{
    FILE *arquivo;
    char *linha = NULL;
    size_t capacidade_linha = 0;
    size_t capacidade_tarefas = 0;
    size_t numero_linha = 1;
    int resultado_leitura;
    char *campos[4];
    size_t quantidade_campos;

    entrada->tempo_total = 0;
    entrada->tarefas = NULL;
    entrada->quantidade_tarefas = 0;

    arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        definir_erro(mensagem_erro,
                     tamanho_mensagem_erro,
                     "nao foi possivel abrir o arquivo de entrada '%s': %s",
                     caminho,
                     strerror(errno));
        return 0;
    }

    resultado_leitura = ler_linha(arquivo, &linha, &capacidade_linha);
    if (resultado_leitura == 0) {
        definir_erro(mensagem_erro,
                     tamanho_mensagem_erro,
                     "o arquivo de entrada esta vazio");
        goto falha;
    }
    if (resultado_leitura < 0) {
        definir_erro(mensagem_erro,
                     tamanho_mensagem_erro,
                     "nao foi possivel ler o arquivo de entrada");
        goto falha;
    }

    quantidade_campos = separar_campos(linha, campos, 1);
    if (quantidade_campos != 1 ||
        !interpretar_inteiro_positivo(campos[0], &entrada->tempo_total)) {
        definir_erro(mensagem_erro,
                     tamanho_mensagem_erro,
                     "a linha 1 deve conter um unico tempo de simulacao inteiro positivo");
        goto falha;
    }

    numero_linha = 2;
    while ((resultado_leitura =
                ler_linha(arquivo, &linha, &capacidade_linha)) > 0) {
        DefinicaoTarefa tarefa;

        quantidade_campos = separar_campos(linha, campos, 4);
        if (quantidade_campos != 4) {
            definir_erro(mensagem_erro,
                         tamanho_mensagem_erro,
                         "a linha %zu deve conter NOME PERIODO DEADLINE BURST",
                         numero_linha);
            goto falha;
        }

        if (!interpretar_inteiro_positivo(campos[1], &tarefa.periodo)) {
            definir_erro(mensagem_erro,
                         tamanho_mensagem_erro,
                         "a linha %zu possui periodo positivo invalido: '%s'",
                         numero_linha,
                         campos[1]);
            goto falha;
        }
        if (!interpretar_inteiro_positivo(campos[2],
                                          &tarefa.deadline_relativo)) {
            definir_erro(mensagem_erro,
                         tamanho_mensagem_erro,
                         "a linha %zu possui deadline positivo invalido: '%s'",
                         numero_linha,
                         campos[2]);
            goto falha;
        }
        if (!interpretar_inteiro_positivo(campos[3], &tarefa.tempo_cpu)) {
            definir_erro(mensagem_erro,
                         tamanho_mensagem_erro,
                         "a linha %zu possui burst positivo invalido: '%s'",
                         numero_linha,
                         campos[3]);
            goto falha;
        }
        if (tarefa.deadline_relativo > tarefa.periodo) {
            definir_erro(mensagem_erro,
                         tamanho_mensagem_erro,
                         "a linha %zu viola D <= P",
                         numero_linha);
            goto falha;
        }
        if (tarefa.tempo_cpu > tarefa.deadline_relativo) {
            definir_erro(mensagem_erro,
                         tamanho_mensagem_erro,
                         "a linha %zu viola C <= D",
                         numero_linha);
            goto falha;
        }

        tarefa.nome = copiar_texto(campos[0]);
        tarefa.ordem_entrada = entrada->quantidade_tarefas;
        if (tarefa.nome == NULL ||
            !adicionar_tarefa(entrada, &capacidade_tarefas, &tarefa)) {
            free(tarefa.nome);
            definir_erro(mensagem_erro, tamanho_mensagem_erro, "memoria insuficiente");
            goto falha;
        }

        numero_linha++;
    }

    if (resultado_leitura < 0) {
        definir_erro(mensagem_erro,
                     tamanho_mensagem_erro,
                     "nao foi possivel ler o arquivo de entrada");
        goto falha;
    }

    free(linha);
    fclose(arquivo);
    return 1;

falha:
    free(linha);
    fclose(arquivo);
    liberar_entrada(entrada);
    return 0;
}
