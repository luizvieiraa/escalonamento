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

static ResumoSimulacao executar_com_algoritmo(const EntradaSimulacao *entrada,
                                              AlgoritmoEscalonamento algoritmo)
{
    ResumoSimulacao resumo;
    char mensagem_erro[256];

    exigir(simular_escalonamento(entrada,
                                 algoritmo,
                                 &resumo,
                                 mensagem_erro,
                                 sizeof(mensagem_erro)),
           mensagem_erro);
    return resumo;
}

static ResumoSimulacao executar(const EntradaSimulacao *entrada)
{
    return executar_com_algoritmo(entrada, ESCALONADOR_RATE);
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
    exigir(resumo.mortas_por_tarefa[0] == 0,
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
    exigir(resumo.mortas_por_tarefa[0] == 1,
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
    exigir(resumo.mortas_por_tarefa[0] == 1,
           "segunda instancia de BAIXA deveria ficar pendente");

    liberar_resumo_simulacao(&resumo);
}

static void testar_conclusao_exatamente_no_deadline(void)
{
    DefinicaoTarefa tarefas[] = {
        {"EXATA", 10, 3, 3, 0}
    };
    EntradaSimulacao entrada = {5, tarefas, 1};
    ResumoSimulacao resumo = executar(&entrada);

    exigir(resumo.conclusoes_por_tarefa[0] == 1,
           "conclusao exatamente no deadline deveria ser aceita");
    exigir(resumo.deadlines_perdidos_por_tarefa[0] == 0,
           "tarefa concluida no deadline nao deveria ser perdida");
    exigir(resumo.mortas_por_tarefa[0] == 0,
           "tarefa concluida nao deveria ser morta");

    liberar_resumo_simulacao(&resumo);
}

static void testar_deadline_no_limite_final(void)
{
    DefinicaoTarefa tarefas[] = {
        {"PRIMEIRA", 10, 3, 2, 0},
        {"SEGUNDA", 10, 3, 2, 1}
    };
    EntradaSimulacao entrada = {3, tarefas, 2};
    ResumoSimulacao resumo = executar(&entrada);
    TrechoExecucao *ultimo = &resumo.historico[resumo.quantidade_trechos - 1];

    exigir(resumo.conclusoes_por_tarefa[0] == 1,
           "primeira tarefa deveria terminar antes do limite");
    exigir(resumo.deadlines_perdidos_por_tarefa[1] == 1,
           "deadline igual ao fim da simulacao deveria ser perdido");
    exigir(resumo.mortas_por_tarefa[1] == 0,
           "deadline no limite nao deveria ser contado como Killed");
    exigir(ultimo->indice_tarefa == 1 &&
               ultimo->situacao == TRECHO_DEADLINE_PERDIDO,
           "ultimo trecho deveria terminar com indicador L");

    liberar_resumo_simulacao(&resumo);
}

static void testar_deadline_e_chegada_no_mesmo_instante(void)
{
    DefinicaoTarefa tarefas[] = {
        {"PRIMEIRA", 3, 3, 2, 0},
        {"SEGUNDA", 3, 3, 2, 1}
    };
    EntradaSimulacao entrada = {4, tarefas, 2};
    ResumoSimulacao resumo = executar(&entrada);

    exigir(resumo.deadlines_perdidos_por_tarefa[1] == 1,
           "instancia antiga deveria ser descartada no instante 3");
    exigir(resumo.mortas_por_tarefa[0] == 1 &&
               resumo.mortas_por_tarefa[1] == 1,
           "novas instancias do instante 3 deveriam existir no fim");
    exigir(resumo.quantidade_trechos == 3 &&
               resumo.historico[1].situacao == TRECHO_DEADLINE_PERDIDO,
           "descarte deveria ocorrer antes das chegadas do mesmo instante");

    liberar_resumo_simulacao(&resumo);
}

static void testar_edf_com_deadlines_absolutos(void)
{
    DefinicaoTarefa tarefas[] = {
        {"ATT", 20, 12, 8, 0},
        {"NAV", 50, 30, 15, 1}
    };
    EntradaSimulacao entrada = {32, tarefas, 2};
    ResumoSimulacao resumo =
        executar_com_algoritmo(&entrada, ESCALONADOR_EDF);

    exigir(resumo.quantidade_trechos == 4,
           "cenario EDF parcial deveria possuir quatro trechos");
    exigir(resumo.historico[1].indice_tarefa == 1 &&
               resumo.historico[1].duracao == 15 &&
               resumo.historico[1].situacao == TRECHO_FINALIZADO,
           "NAV deveria continuar no instante 20 por ter deadline 30");
    exigir(resumo.historico[2].indice_tarefa == 0 &&
               resumo.historico[2].duracao == 8 &&
               resumo.historico[2].situacao == TRECHO_FINALIZADO,
           "nova ATT deveria executar depois de NAV e terminar em 31");
    exigir(resumo.deadlines_perdidos_por_tarefa[0] == 0 &&
               resumo.deadlines_perdidos_por_tarefa[1] == 0,
           "EDF nao deveria perder deadlines nesse intervalo");

    liberar_resumo_simulacao(&resumo);
}

static void testar_desempate_edf_pela_ordem(void)
{
    DefinicaoTarefa tarefas[] = {
        {"PRIMEIRA", 10, 5, 1, 0},
        {"SEGUNDA", 8, 5, 1, 1}
    };
    EntradaSimulacao entrada = {2, tarefas, 2};
    ResumoSimulacao resumo =
        executar_com_algoritmo(&entrada, ESCALONADOR_EDF);

    exigir(resumo.historico[0].indice_tarefa == 0 &&
               resumo.historico[1].indice_tarefa == 1,
           "deadlines absolutos iguais deveriam respeitar a ordem do arquivo");

    liberar_resumo_simulacao(&resumo);
}

int main(void)
{
    testar_tarefa_unica_periodica();
    testar_varias_tarefas();
    testar_cpu_totalmente_ociosa();
    testar_instancia_pendente_no_final();
    testar_rate_com_preempcao();
    testar_conclusao_exatamente_no_deadline();
    testar_deadline_no_limite_final();
    testar_deadline_e_chegada_no_mesmo_instante();
    testar_edf_com_deadlines_absolutos();
    testar_desempate_edf_pela_ordem();

    puts("Testes do nucleo temporal passaram.");
    return EXIT_SUCCESS;
}
