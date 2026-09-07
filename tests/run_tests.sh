#!/bin/sh

set -eu

diretorio_projeto=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
escalonador="$diretorio_projeto/scheduler"
entrada_valida="$diretorio_projeto/tests/cases/minimal.txt"
entrada_multipla="$diretorio_projeto/tests/cases/multiple.txt"
entrada_preempcao_rate="$diretorio_projeto/tests/cases/rate_preempcao.txt"
saida_rate_esperada="$diretorio_projeto/tests/expected/minimal_rate.out"
saida_edf_esperada="$diretorio_projeto/tests/expected/minimal_edf.out"
saida_preempcao_rate_esperada="$diretorio_projeto/tests/expected/rate_preempcao.out"
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

[ -x "$escalonador" ] || falhar "executavel scheduler nao encontrado; execute make primeiro"

esperar_erro_interface "argumentos ausentes"
esperar_erro_interface "argumentos extras" rate "$entrada_valida" extra
esperar_erro_interface "algoritmo invalido" fifo "$entrada_valida"

printf 'nao-e-numero\n' >"$diretorio_teste/total-invalido.txt"
printf '0\n' >"$diretorio_teste/total-nao-positivo.txt"
printf '10 extra\n' >"$diretorio_teste/campo-extra-total.txt"
printf '10\nTAREFA 5 4\n' >"$diretorio_teste/campo-ausente-tarefa.txt"
printf '10\nTAREFA cinco 4 2\n' >"$diretorio_teste/campo-nao-numerico.txt"
printf '10\nTAREFA 5 0 0\n' >"$diretorio_teste/campo-nao-positivo.txt"
printf '10\nTAREFA 5 6 2\n' >"$diretorio_teste/deadline-maior-periodo.txt"
printf '10\nTAREFA 5 4 5\n' >"$diretorio_teste/burst-maior-deadline.txt"
printf '10\nTAREFA 5 4 2 extra\n' >"$diretorio_teste/campo-extra-tarefa.txt"
printf '10\n\n' >"$diretorio_teste/linha-vazia.txt"

esperar_erro_interface "arquivo ausente" rate "$diretorio_teste/nao-existe.txt"
esperar_erro_interface "arquivo ilegivel" rate "$diretorio_teste"
esperar_erro_interface "arquivo vazio" rate "$diretorio_teste/stdout.txt"
esperar_erro_interface "tempo nao numerico" rate "$diretorio_teste/total-invalido.txt"
esperar_erro_interface "tempo nao positivo" rate "$diretorio_teste/total-nao-positivo.txt"
esperar_erro_interface "campo extra no tempo" rate "$diretorio_teste/campo-extra-total.txt"
esperar_erro_interface "campo ausente na tarefa" rate "$diretorio_teste/campo-ausente-tarefa.txt"
esperar_erro_interface "campo nao numerico" rate "$diretorio_teste/campo-nao-numerico.txt"
esperar_erro_interface "campo nao positivo" rate "$diretorio_teste/campo-nao-positivo.txt"
esperar_erro_interface "deadline maior que periodo" rate "$diretorio_teste/deadline-maior-periodo.txt"
esperar_erro_interface "burst maior que deadline" rate "$diretorio_teste/burst-maior-deadline.txt"
esperar_erro_interface "campo extra na tarefa" rate "$diretorio_teste/campo-extra-tarefa.txt"
esperar_erro_interface "linha de tarefa vazia" rate "$diretorio_teste/linha-vazia.txt"

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

if ! (cd "$diretorio_teste" && "$escalonador" rate "$entrada_preempcao_rate" >stdout.txt 2>stderr.txt); then
    falhar "cenario de preempcao RATE falhou"
fi
[ ! -s "$diretorio_teste/stdout.txt" ] || falhar "cenario RATE escreveu em stdout"
[ ! -s "$diretorio_teste/stderr.txt" ] || falhar "cenario RATE escreveu em stderr"
cmp -s "$diretorio_teste/rate_lhcv.out" "$saida_preempcao_rate_esperada" || \
    falhar "saida de preempcao RATE esta incorreta"

gcc -I"$diretorio_projeto/src" -std=c11 -Wall -Wextra -Wpedantic \
    "$diretorio_projeto/tests/teste_nucleo.c" \
    "$diretorio_projeto/src/scheduler.c" \
    "$diretorio_projeto/src/parser.c" \
    "$diretorio_projeto/src/output.c" \
    -o "$diretorio_teste/teste_nucleo"

"$diretorio_teste/teste_nucleo"

echo "Todos os testes ate a etapa 6 passaram."
