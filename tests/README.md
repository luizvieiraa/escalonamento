# Testes

Execute a suite completa a partir da raiz do projeto:

```sh
make test
```

O script `run_tests.sh` cria um diretorio temporario, executa os testes e o
remove ao terminar. Assim, os arquivos `.out` da raiz do projeto nao sao
alterados pelos testes.

## Cobertura

- argumentos ausentes, excedentes e algoritmo invalido;
- arquivo ausente, ilegivel e vazio;
- campos ausentes, extras, nao numericos, nao positivos ou fora de `int`;
- violacoes de `C <= D <= P`;
- entradas com CRLF, tabulacoes, nomes longos e mais de oito tarefas;
- tarefa unica, varias tarefas e intervalos idle;
- multiplas instancias da mesma tarefa;
- prioridade, desempate e preempcao de RATE;
- deadline absoluto, desempate e preempcao de EDF;
- conclusao exatamente no deadline;
- deadline no instante final e deadline simultaneo a uma nova chegada;
- multiplas perdas de deadline;
- `Killed` para tarefa executando e para tarefa apenas aguardando;
- formato completo dos arquivos `rate_lhcv.out` e `edf_lhcv.out`;
- falha ao criar o arquivo de saida;
- silencio em `stdout` durante execucoes normais.
- busca sistematica e reproducao de um conjunto em que RATE perde deadlines e
  EDF nao perde.

Os arquivos em `expected/` sao comparados byte a byte com `cmp`.

O programa `buscar_comparativo.c` percorre combinacoes pequenas que respeitam
`C <= D <= P`, executa os dois algoritmos e exige zero tarefas `Killed`. A busca
encontrou o caso permanente `cases/comparativo_rate_edf.txt`.
