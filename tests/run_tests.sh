#!/bin/sh

set -eu

diretorio_projeto=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
escalonador="$diretorio_projeto/scheduler"
entrada_valida="$diretorio_projeto/tests/cases/minimal.txt"
entrada_multipla="$diretorio_projeto/tests/cases/multiple.txt"
entrada_preempcao_rate="$diretorio_projeto/tests/cases/rate_preempcao.txt"
entrada_empate_rate="$diretorio_projeto/tests/cases/rate_empate.txt"
entrada_empate_edf="$diretorio_projeto/tests/cases/edf_empate_chegada.txt"
entrada_multiplas_perdas="$diretorio_projeto/tests/cases/multiplas_perdas.txt"
entrada_killed_aguardando="$diretorio_projeto/tests/cases/killed_aguardando.txt"
entrada_muitas_tarefas="$diretorio_projeto/tests/cases/muitas_tarefas.txt"
entrada_tarefa_unica_idle="$diretorio_projeto/tests/cases/tarefa_unica_idle.txt"
saida_rate_esperada="$diretorio_projeto/tests/expected/minimal_rate.out"
saida_edf_esperada="$diretorio_projeto/tests/expected/minimal_edf.out"
saida_preempcao_rate_esperada="$diretorio_projeto/tests/expected/rate_preempcao.out"
saida_exemplo_rate_esperada="$diretorio_projeto/tests/expected/exemplo_rate.out"
saida_exemplo_edf_esperada="$diretorio_projeto/tests/expected/exemplo_edf.out"
saida_empate_rate_esperada="$diretorio_projeto/tests/expected/rate_empate.out"
saida_empate_edf_esperada="$diretorio_projeto/tests/expected/edf_empate_chegada.out"
saida_multiplas_perdas_esperada="$diretorio_projeto/tests/expected/multiplas_perdas_rate.out"
saida_killed_aguardando_esperada="$diretorio_projeto/tests/expected/killed_aguardando_rate.out"
saida_tarefa_unica_idle_esperada="$diretorio_projeto/tests/expected/tarefa_unica_idle_rate.out"
diretorio_teste=$(mktemp -d)

trap 'rm -rf "$diretorio_teste"' EXIT

falhar()
{
    echo "FALHA: $1" >&2
    exit 1
}

esperar_erro_interface()
{
    nome_teste=$1
    shift

    if (cd "$diretorio_teste" && "$escalonador" "$@" >stdout.txt 2>stderr.txt); then
        falhar "$nome_teste retornou codigo de saida zero"
    fi

    [ ! -s "$diretorio_teste/stdout.txt" ] || falhar "$nome_teste escreveu em stdout"
    [ -s "$diretorio_teste/stderr.txt" ] || falhar "$nome_teste nao escreveu em stderr"
    [ ! -e "$diretorio_teste/rate_lhcv.out" ] || falhar "$nome_teste criou rate_lhcv.out"
    [ ! -e "$diretorio_teste/edf_lhcv.out" ] || falhar "$nome_teste criou edf_lhcv.out"
}

executar_com_sucesso()
{
    nome_teste=$1
    algoritmo=$2
    entrada=$3
    arquivo_saida="$diretorio_teste/${algoritmo}_lhcv.out"

    rm -f "$arquivo_saida"
    if ! (cd "$diretorio_teste" &&
          "$escalonador" "$algoritmo" "$entrada" >stdout.txt 2>stderr.txt); then
        falhar "$nome_teste encerrou com erro"
    fi

    [ ! -s "$diretorio_teste/stdout.txt" ] || falhar "$nome_teste escreveu em stdout"
    [ ! -s "$diretorio_teste/stderr.txt" ] || falhar "$nome_teste escreveu em stderr"
    [ -f "$arquivo_saida" ] || falhar "$nome_teste nao criou o arquivo de saida"
}

executar_e_comparar()
{
    nome_teste=$1
    algoritmo=$2
    entrada=$3
    saida_esperada=$4

    executar_com_sucesso "$nome_teste" "$algoritmo" "$entrada"
    cmp -s "$diretorio_teste/${algoritmo}_lhcv.out" "$saida_esperada" || \
        falhar "$nome_teste produziu uma saida diferente da esperada"
}

[ -x "$escalonador" ] || falhar "executavel scheduler nao encontrado; execute make primeiro"

esperar_erro_interface "argumentos ausentes"
esperar_erro_interface "argumentos extras" rate "$entrada_valida" extra
esperar_erro_interface "algoritmo invalido" fifo "$entrada_valida"

printf 'nao-e-numero\n' >"$diretorio_teste/total-invalido.txt"
printf '%s\n' '-1' >"$diretorio_teste/total-negativo.txt"
printf '2147483648\n' >"$diretorio_teste/total-fora-do-int.txt"
printf '0\n' >"$diretorio_teste/total-nao-positivo.txt"
printf '10 extra\n' >"$diretorio_teste/campo-extra-total.txt"
printf '10\nTAREFA 5 4\n' >"$diretorio_teste/campo-ausente-tarefa.txt"
printf '10\nTAREFA cinco 4 2\n' >"$diretorio_teste/campo-nao-numerico.txt"
printf '10\nTAREFA 5 0 0\n' >"$diretorio_teste/campo-nao-positivo.txt"
printf '10\nTAREFA 0 1 1\n' >"$diretorio_teste/periodo-zero.txt"
printf '10\nTAREFA 5 0 1\n' >"$diretorio_teste/deadline-zero.txt"
printf '10\nTAREFA 5 4 0\n' >"$diretorio_teste/burst-zero.txt"
printf '10\nTAREFA 5 quatro 2\n' >"$diretorio_teste/deadline-nao-numerico.txt"
printf '10\nTAREFA 5 4 dois\n' >"$diretorio_teste/burst-nao-numerico.txt"
printf '10\nTAREFA 5 6 2\n' >"$diretorio_teste/deadline-maior-periodo.txt"
printf '10\nTAREFA 5 4 5\n' >"$diretorio_teste/burst-maior-deadline.txt"
printf '10\nTAREFA 5 4 2 extra\n' >"$diretorio_teste/campo-extra-tarefa.txt"
printf '10\n\n' >"$diretorio_teste/linha-vazia.txt"
: >"$diretorio_teste/arquivo-vazio.txt"
printf '3\r\nTAB\t3\t3\t1\r\n' >"$diretorio_teste/espacos-validos.txt"
nome_longo=$(printf '%0150d' 0 | tr '0' 'A')
printf '2\n%s 2 2 1\n' "$nome_longo" >"$diretorio_teste/nome-longo.txt"

esperar_erro_interface "arquivo ausente" rate "$diretorio_teste/nao-existe.txt"
esperar_erro_interface "arquivo ilegivel" rate "$diretorio_teste"
esperar_erro_interface "arquivo vazio" rate "$diretorio_teste/arquivo-vazio.txt"
esperar_erro_interface "tempo nao numerico" rate "$diretorio_teste/total-invalido.txt"
esperar_erro_interface "tempo negativo" rate "$diretorio_teste/total-negativo.txt"
esperar_erro_interface "tempo fora do limite de int" rate "$diretorio_teste/total-fora-do-int.txt"
esperar_erro_interface "tempo nao positivo" rate "$diretorio_teste/total-nao-positivo.txt"
esperar_erro_interface "campo extra no tempo" rate "$diretorio_teste/campo-extra-total.txt"
esperar_erro_interface "campo ausente na tarefa" rate "$diretorio_teste/campo-ausente-tarefa.txt"
esperar_erro_interface "campo nao numerico" rate "$diretorio_teste/campo-nao-numerico.txt"
esperar_erro_interface "campo nao positivo" rate "$diretorio_teste/campo-nao-positivo.txt"
esperar_erro_interface "periodo zero" rate "$diretorio_teste/periodo-zero.txt"
esperar_erro_interface "deadline zero" rate "$diretorio_teste/deadline-zero.txt"
esperar_erro_interface "burst zero" rate "$diretorio_teste/burst-zero.txt"
esperar_erro_interface "deadline nao numerico" rate "$diretorio_teste/deadline-nao-numerico.txt"
esperar_erro_interface "burst nao numerico" rate "$diretorio_teste/burst-nao-numerico.txt"
esperar_erro_interface "deadline maior que periodo" rate "$diretorio_teste/deadline-maior-periodo.txt"
esperar_erro_interface "burst maior que deadline" rate "$diretorio_teste/burst-maior-deadline.txt"
esperar_erro_interface "campo extra na tarefa" rate "$diretorio_teste/campo-extra-tarefa.txt"
esperar_erro_interface "linha de tarefa vazia" rate "$diretorio_teste/linha-vazia.txt"

if (cd /proc &&
    "$escalonador" rate "$entrada_valida" \
        >"$diretorio_teste/stdout.txt" 2>"$diretorio_teste/stderr.txt"); then
    falhar "falha de criacao da saida retornou codigo zero"
fi
[ ! -s "$diretorio_teste/stdout.txt" ] || falhar "falha de saida escreveu em stdout"
[ -s "$diretorio_teste/stderr.txt" ] || falhar "falha de saida nao escreveu em stderr"

if ! (cd "$diretorio_teste" && "$escalonador" rate "$entrada_valida" >stdout.txt 2>stderr.txt); then
    falhar "comando rate valido falhou"
fi
[ ! -s "$diretorio_teste/stdout.txt" ] || falhar "comando rate valido escreveu em stdout"
[ ! -s "$diretorio_teste/stderr.txt" ] || falhar "comando rate valido escreveu em stderr"
[ -f "$diretorio_teste/rate_lhcv.out" ] || falhar "comando rate nao criou rate_lhcv.out"
cmp -s "$diretorio_teste/rate_lhcv.out" "$saida_rate_esperada" || \
    falhar "saida rate nao corresponde ao formato esperado"

if ! (cd "$diretorio_teste" && "$escalonador" edf "$entrada_valida" >stdout.txt 2>stderr.txt); then
    falhar "comando edf valido falhou"
fi
[ ! -s "$diretorio_teste/stdout.txt" ] || falhar "comando edf valido escreveu em stdout"
[ ! -s "$diretorio_teste/stderr.txt" ] || falhar "comando edf valido escreveu em stderr"
[ -f "$diretorio_teste/edf_lhcv.out" ] || falhar "comando edf nao criou edf_lhcv.out"
cmp -s "$diretorio_teste/edf_lhcv.out" "$saida_edf_esperada" || \
    falhar "saida edf nao corresponde ao formato esperado"

if ! (cd "$diretorio_teste" && "$escalonador" rate "$entrada_multipla" >stdout.txt 2>stderr.txt); then
    falhar "entrada valida com varias tarefas falhou"
fi
[ ! -s "$diretorio_teste/stdout.txt" ] || falhar "entrada com varias tarefas escreveu em stdout"
[ ! -s "$diretorio_teste/stderr.txt" ] || falhar "entrada com varias tarefas escreveu em stderr"
cmp -s "$diretorio_teste/rate_lhcv.out" "$saida_exemplo_rate_esperada" || \
    falhar "saida RATE do exemplo do enunciado esta incorreta"

if ! (cd "$diretorio_teste" && "$escalonador" edf "$entrada_multipla" >stdout.txt 2>stderr.txt); then
    falhar "execucao EDF do exemplo do enunciado falhou"
fi
[ ! -s "$diretorio_teste/stdout.txt" ] || falhar "exemplo EDF escreveu em stdout"
[ ! -s "$diretorio_teste/stderr.txt" ] || falhar "exemplo EDF escreveu em stderr"
cmp -s "$diretorio_teste/edf_lhcv.out" "$saida_exemplo_edf_esperada" || \
    falhar "saida EDF do exemplo do enunciado esta incorreta"

if ! (cd "$diretorio_teste" && "$escalonador" rate "$entrada_preempcao_rate" >stdout.txt 2>stderr.txt); then
    falhar "cenario de preempcao RATE falhou"
fi
[ ! -s "$diretorio_teste/stdout.txt" ] || falhar "cenario RATE escreveu em stdout"
[ ! -s "$diretorio_teste/stderr.txt" ] || falhar "cenario RATE escreveu em stderr"
cmp -s "$diretorio_teste/rate_lhcv.out" "$saida_preempcao_rate_esperada" || \
    falhar "saida de preempcao RATE esta incorreta"

executar_e_comparar "desempate RATE" \
                    rate \
                    "$entrada_empate_rate" \
                    "$saida_empate_rate_esperada"
executar_e_comparar "desempate EDF com nova chegada" \
                    edf \
                    "$entrada_empate_edf" \
                    "$saida_empate_edf_esperada"
executar_e_comparar "multiplas perdas da mesma tarefa" \
                    rate \
                    "$entrada_multiplas_perdas" \
                    "$saida_multiplas_perdas_esperada"
executar_e_comparar "Killed de tarefa aguardando" \
                    rate \
                    "$entrada_killed_aguardando" \
                    "$saida_killed_aguardando_esperada"
executar_e_comparar "tarefa unica com CPU idle" \
                    rate \
                    "$entrada_tarefa_unica_idle" \
                    "$saida_tarefa_unica_idle_esperada"
executar_com_sucesso "crescimento do vetor de tarefas" \
                     rate \
                     "$entrada_muitas_tarefas"
executar_com_sucesso "entrada CRLF com tabulacoes" \
                     edf \
                     "$diretorio_teste/espacos-validos.txt"
executar_com_sucesso "nome maior que o buffer inicial" \
                     rate \
                     "$diretorio_teste/nome-longo.txt"

gcc -I"$diretorio_projeto/src" -std=c11 -Wall -Wextra -Wpedantic \
    "$diretorio_projeto/tests/teste_nucleo.c" \
    "$diretorio_projeto/src/scheduler.c" \
    "$diretorio_projeto/src/parser.c" \
    "$diretorio_projeto/src/output.c" \
    -o "$diretorio_teste/teste_nucleo"

"$diretorio_teste/teste_nucleo"

echo "Todos os testes ate a etapa 9 passaram."
