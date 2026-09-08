#!/bin/sh

set -u

diretorio_projeto=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
diretorio_temporario=$(mktemp -d)
trap 'rm -rf "$diretorio_temporario"' EXIT

cd "$diretorio_projeto" || exit 1
set -x

date
whoami
pwd
uname -a
gcc --version
make --version
GIT_PAGER=cat git --no-pager log --reverse --date=iso \
    --pretty=tformat:'%h | %ad | %s' -10

# Reproducao do erro de ligacao identificado quando output.c foi introduzido.
if gcc -Isrc -std=c11 -Wall -Wextra -Wpedantic \
       src/main.c src/parser.c src/scheduler.c \
       -o "$diretorio_temporario/scheduler-sem-output"; then
    codigo_erro_ligacao=0
else
    codigo_erro_ligacao=$?
fi
printf 'CODIGO_ERRO_LIGACAO=%d\n' "$codigo_erro_ligacao"
test "$codigo_erro_ligacao" -ne 0

set -e

make clean
make
make test

./scheduler rate tests/cases/comparativo_rate_edf.txt
cat rate_lhcv.out
./scheduler edf tests/cases/comparativo_rate_edf.txt
cat edf_lhcv.out

printf '10\nTAREFA 5 6 2\n' >"$diretorio_temporario/entrada-invalida.txt"
cd "$diretorio_temporario" || exit 1
if "$diretorio_projeto/scheduler" rate entrada-invalida.txt; then
    codigo_entrada_invalida=0
else
    codigo_entrada_invalida=$?
fi
printf 'CODIGO_ENTRADA_INVALIDA=%d\n' "$codigo_entrada_invalida"
test "$codigo_entrada_invalida" -ne 0
test ! -e rate_lhcv.out
test ! -e edf_lhcv.out
set +x
