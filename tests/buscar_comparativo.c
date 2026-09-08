#include <stdio.h>
#include <stdlib.h>

#include "scheduler.h"

static int calcular_mdc(int primeiro, int segundo)
{
    while (segundo != 0) {
        int resto = primeiro % segundo;
        primeiro = segundo;
        segundo = resto;
    }

    return primeiro;
}

static size_t somar(const size_t *valores, size_t quantidade)
{
    size_t indice;
    size_t total = 0;

    for (indice = 0; indice < quantidade; indice++) {
        total += valores[indice];
    }

    return total;
}

int main(void)
{
    int periodo_primeira;
    int periodo_segunda;

    for (periodo_primeira = 2; periodo_primeira <= 8; periodo_primeira++) {
        for (periodo_segunda = periodo_primeira + 1;
             periodo_segunda <= 10;
             periodo_segunda++) {
            int deadline_primeira;
            int deadline_segunda;

            for (deadline_primeira = 1;
                 deadline_primeira <= periodo_primeira;
                 deadline_primeira++) {
                for (deadline_segunda = 1;
                     deadline_segunda <= periodo_segunda;
                     deadline_segunda++) {
                    int cpu_primeira;
                    int cpu_segunda;

                    for (cpu_primeira = 1;
                         cpu_primeira <= deadline_primeira;
                         cpu_primeira++) {
                        for (cpu_segunda = 1;
                             cpu_segunda <= deadline_segunda;
                             cpu_segunda++) {
                            DefinicaoTarefa tarefas[] = {
                                {"T1", periodo_primeira, deadline_primeira,
                                 cpu_primeira, 0},
                                {"T2", periodo_segunda, deadline_segunda,
                                 cpu_segunda, 1}
                            };
                            EntradaSimulacao entrada;
                            ResumoSimulacao rate;
                            ResumoSimulacao edf;
                            char erro[256];
                            int mmc = periodo_primeira / calcular_mdc(
                                periodo_primeira, periodo_segunda) * periodo_segunda;

                            entrada.tempo_total = 2 * mmc;
                            entrada.tarefas = tarefas;
                            entrada.quantidade_tarefas = 2;

                            if (!simular_escalonamento(&entrada,
                                                       ESCALONADOR_RATE,
                                                       &rate,
                                                       erro,
                                                       sizeof(erro))) {
                                fprintf(stderr, "Erro na busca RATE: %s\n", erro);
                                return EXIT_FAILURE;
                            }
                            if (!simular_escalonamento(&entrada,
                                                       ESCALONADOR_EDF,
                                                       &edf,
                                                       erro,
                                                       sizeof(erro))) {
                                liberar_resumo_simulacao(&rate);
                                fprintf(stderr, "Erro na busca EDF: %s\n", erro);
                                return EXIT_FAILURE;
                            }

                            if (somar(rate.deadlines_perdidos_por_tarefa, 2) > 0 &&
                                somar(edf.deadlines_perdidos_por_tarefa, 2) == 0 &&
                                somar(rate.mortas_por_tarefa, 2) == 0 &&
                                somar(edf.mortas_por_tarefa, 2) == 0) {
                                printf("%d\n", entrada.tempo_total);
                                printf("T1 %d %d %d\n",
                                       periodo_primeira,
                                       deadline_primeira,
                                       cpu_primeira);
                                printf("T2 %d %d %d\n",
                                       periodo_segunda,
                                       deadline_segunda,
                                       cpu_segunda);
                                printf("RATE_LOST=%zu EDF_LOST=%zu\n",
                                       somar(rate.deadlines_perdidos_por_tarefa, 2),
                                       somar(edf.deadlines_perdidos_por_tarefa, 2));

                                liberar_resumo_simulacao(&rate);
                                liberar_resumo_simulacao(&edf);
                                return EXIT_SUCCESS;
                            }

                            liberar_resumo_simulacao(&rate);
                            liberar_resumo_simulacao(&edf);
                        }
                    }
                }
            }
        }
    }

    fputs("Nenhum conjunto comparativo foi encontrado.\n", stderr);
    return EXIT_FAILURE;
}
