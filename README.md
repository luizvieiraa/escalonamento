# Simulador de escalonamento

Simulador em C de tarefas periodicas de tempo real utilizando os algoritmos
preemptivos Rate-Monotonic (`rate`) e Earliest-Deadline-First (`edf`).

## Requisitos

- Linux ou WSL;
- GCC com suporte a C11;
- GNU Make;
- shell compativel com POSIX para executar os testes.

O projeto foi implementado e validado no WSL2, com Ubuntu, GCC 15.2.0 e GNU
Make 4.4.1.

## Compilacao

Na raiz do projeto, execute:

```sh
make clean
make
```

O alvo padrao produz um unico executavel chamado `scheduler`.

## Formato da entrada

A primeira linha contem o tempo total da simulacao. Cada linha seguinte define
uma tarefa:

```text
[NOME] [PERIODO] [DEADLINE] [BURST]
```

Todos os valores numericos devem ser inteiros positivos e respeitar
`BURST <= DEADLINE <= PERIODO`.

Exemplo:

```text
100
ATT 20 12 8
NAV 50 30 15
```

## Execucao

Rate-Monotonic:

```sh
./scheduler rate tests/cases/multiple.txt
```

Earliest-Deadline-First:

```sh
./scheduler edf tests/cases/multiple.txt
```

Durante uma execucao normal, nada e escrito em `stdout`. O resultado e gravado
em `rate_lhcv.out` ou `edf_lhcv.out`, conforme o algoritmo escolhido.

## Testes

Para compilar e executar toda a suite:

```sh
make test
```

A suite cobre parser, erros de interface, tarefas periodicas, CPU idle,
preempcao, desempates, deadlines, conclusoes, `Killed`, RATE, EDF, casos de
fronteira e comparacao automatica das saidas.

O caso `tests/cases/comparativo_rate_edf.txt` foi encontrado por busca
sistematica. Nele, RATE perde dois deadlines e EDF nao perde nenhum. O programa
`tests/buscar_comparativo.c` reproduz a busca.

## Tratamento de erros

Entradas invalidas produzem uma mensagem em `stderr`, codigo de saida diferente
de zero e nenhum arquivo `.out`. Sao tratados argumentos incorretos, algoritmo
invalido, arquivo ausente ou ilegivel, campos malformados, valores nao positivos
e violacoes de `BURST <= DEADLINE <= PERIODO`.

## Arquivos C

- `src/main.c`: valida a interface de linha de comando e seleciona o algoritmo.
- `src/parser.c`: le e valida o arquivo, alocando as definicoes das tarefas.
- `src/scheduler.c`: controla relogio, instancias, prioridades, preempcao,
  deadlines, conclusoes e tarefas mortas.
- `src/output.c`: grava o historico e os contadores no formato exigido.
- `tests/teste_nucleo.c`: testes unitarios do nucleo temporal.
- `tests/buscar_comparativo.c`: busca um conjunto que diferencia RATE e EDF.

Os arquivos `.h` em `src/` declaram os tipos e as interfaces compartilhadas.

## Limpeza

```sh
make clean
```

Esse alvo remove executaveis e os arquivos `.out` gerados na raiz.
