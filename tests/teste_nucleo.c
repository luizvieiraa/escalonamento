#include <stdio.h>
#include <stdlib.h>

#include "scheduler.h"

static void exigir(int condicao, const char *mensagem)
{
    if (!condicao) {
        fprintf(stderr, "FALHA: %s\n", mensagem);
        exit(EXIT_FAILURE);
    }
}

static ResumoSimulacao executar(const EntradaSimulacao *entrada)
{
    ResumoSimulacao resumo;
    char mensagem_erro[256];

    exigir(simular_escalonamento(entrada,
                                 ESCALONADOR_RATE,
                                 &resumo,
                                 mensagem_erro,
                                 sizeof(mensagem_erro)),
           mensagem_erro);
    return resumo;
}

static void testar_tarefa_unica_periodica(void)
{
    DefinicaoTarefa tarefas[] = {
        {"TAREFA", 5, 5, 2, 0}
    };
    EntradaSimulacao entrada = {12, tarefas, 1};
    ResumoSimulacao resumo = executar(&entrada);

    exigir(resumo.tempo_execucao_cpu == 6,
           "tarefa unica deveria executar por 6 unidades");
    exigir(resumo.tempo_ocioso == 6,
           "tarefa unica deveria deixar 6 unidades ociosas");
    exigir(resumo.conclusoes_por_tarefa[0] == 3,
           "as tres instancias deveriam terminar");
    exigir(resumo.pendencias_por_tarefa[0] == 0,
           "nao deveria restar instancia pendente");
    exigir(resumo.deadlines_perdidos_por_tarefa[0] == 0,
           "o nucleo basico nao deveria registrar deadlines perdidos");
    exigir(resumo.quantidade_trechos == 5,
           "historico da tarefa unica deveria possuir cinco trechos");
    exigir(resumo.historico[0].duracao == 2 &&
               resumo.historico[0].situacao == TRECHO_FINALIZADO,
           "primeiro trecho deveria terminar apos duas unidades");
    exigir(resumo.historico[1].duracao == 3 &&
               resumo.historico[1].situacao == TRECHO_OCIOSO,
           "primeiro intervalo idle deveria ser agrupado");

    liberar_resumo_simulacao(&resumo);
}

static void testar_varias_tarefas(void)
{
    DefinicaoTarefa tarefas[] = {
        {"PRIMEIRA", 4, 4, 1, 0},
        {"SEGUNDA", 4, 4, 1, 1}
    };
    EntradaSimulacao entrada = {8, tarefas, 2};
    ResumoSimulacao resumo = executar(&entrada);

    exigir(resumo.tempo_execucao_cpu == 4,
           "duas tarefas deveriam executar por 4 unidades");
    exigir(resumo.tempo_ocioso == 4,
           "duas tarefas deveriam deixar 4 unidades ociosas");
    exigir(resumo.conclusoes_por_tarefa[0] == 2,
           "a primeira tarefa deveria concluir duas instancias");
    exigir(resumo.conclusoes_por_tarefa[1] == 2,
           "a segunda tarefa deveria concluir duas instancias");
    exigir(resumo.quantidade_trechos == 6,
           "historico de duas tarefas deveria possuir seis trechos");
    exigir(resumo.historico[0].indice_tarefa == 0 &&
               resumo.historico[1].indice_tarefa == 1,
           "historico deveria preservar a ordem provisoria das tarefas");

    liberar_resumo_simulacao(&resumo);
}

static void testar_cpu_totalmente_ociosa(void)
{
    EntradaSimulacao entrada = {5, NULL, 0};
    ResumoSimulacao resumo = executar(&entrada);

    exigir(resumo.tempo_execucao_cpu == 0,
           "CPU sem tarefas nao deveria executar");
    exigir(resumo.tempo_ocioso == 5,
           "CPU sem tarefas deveria ficar ociosa por todo o tempo");
    exigir(resumo.conclusoes_por_tarefa == NULL,
           "nao deveria existir vetor de conclusoes sem tarefas");
    exigir(resumo.quantidade_trechos == 1 &&
               resumo.historico[0].duracao == 5 &&
               resumo.historico[0].situacao == TRECHO_OCIOSO,
           "todo o periodo idle deveria formar um unico trecho");

    liberar_resumo_simulacao(&resumo);
}

static void testar_instancia_pendente_no_final(void)
{
    DefinicaoTarefa tarefas[] = {
        {"TAREFA", 5, 5, 2, 0}
    };
    EntradaSimulacao entrada = {11, tarefas, 1};
    ResumoSimulacao resumo = executar(&entrada);

    exigir(resumo.conclusoes_por_tarefa[0] == 2,
           "somente duas instancias deveriam terminar");
    exigir(resumo.pendencias_por_tarefa[0] == 1,
           "uma instancia deveria permanecer pendente no final");
    exigir(resumo.tempo_execucao_cpu + resumo.tempo_ocioso == 11,
           "cada instante deve ser execucao ou idle");
    exigir(resumo.historico[resumo.quantidade_trechos - 1].duracao == 1 &&
               resumo.historico[resumo.quantidade_trechos - 1].situacao ==
                   TRECHO_MORTO,
           "ultimo trecho pendente deveria terminar com indicador K");

    liberar_resumo_simulacao(&resumo);
}

static void testar_rate_com_preempcao(void)
{
    DefinicaoTarefa tarefas[] = {
        {"BAIXA", 10, 10, 6, 0},
        {"ALTA", 4, 4, 1, 1}
    };
    EntradaSimulacao entrada = {12, tarefas, 2};
    ResumoSimulacao resumo = executar(&entrada);

    exigir(resumo.quantidade_trechos == 7,
           "cenario RATE deveria possuir sete trechos");
    exigir(resumo.historico[0].indice_tarefa == 1 &&
               resumo.historico[0].situacao == TRECHO_FINALIZADO,
           "menor periodo deveria executar primeiro");
    exigir(resumo.historico[1].indice_tarefa == 0 &&
               resumo.historico[1].duracao == 3 &&
               resumo.historico[1].situacao == TRECHO_INTERROMPIDO,
           "tarefa de menor prioridade deveria ser preemptada no instante 4");
    exigir(resumo.historico[2].indice_tarefa == 1 &&
               resumo.historico[2].situacao == TRECHO_FINALIZADO,
           "nova instancia prioritaria deveria assumir imediatamente");
    exigir(resumo.conclusoes_por_tarefa[0] == 1 &&
               resumo.conclusoes_por_tarefa[1] == 3,
           "contagem de conclusoes RATE incorreta");
    exigir(resumo.pendencias_por_tarefa[0] == 1,
           "segunda instancia de BAIXA deveria ficar pendente");

    liberar_resumo_simulacao(&resumo);
}

int main(void)
{
    testar_tarefa_unica_periodica();
    testar_varias_tarefas();
    testar_cpu_totalmente_ociosa();
    testar_instancia_pendente_no_final();
    testar_rate_com_preempcao();

    puts("Testes do nucleo temporal passaram.");
    return EXIT_SUCCESS;
}
