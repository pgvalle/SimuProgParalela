# Isomorfismo de Grafos

Este repositório contém implementações para resolver o problema de [Isomorfismo de Grafos](https://en.wikipedia.org/wiki/Graph_isomorphism) utilizando força bruta, com versões sequencial e paralela (OpenMP).
O código sequencial original é do problema 1 da [Maratona de Programação Mackenzie de 2021](http://lspd.mackenzie.br/marathon/21/problems.html).

## Mudanças Implementadas

A abordagem original se utilizava do [Algoritmo de Heap](https://en.wikipedia.org/wiki/Heap%27s_algorithm) para gerar permutações dos grafos. Porém, o incremento da variável de iteração é irregular, o que inviabiliza a divisão do trabalho entre múltiplas threads.
Logo, foi necessário gerar permutações via [Decomposição Fatorial](https://jhafranco.com/2012/02/07/generating-permutations-from-factoradic-numbers/). Esta técnica mapeia diretamente um índice inteiro a uma permutação única, permitindo que a variável de iteração gere as permutações incrementando regularmente. Com isso, a divisão do trabalho entre as threads se torna trivial.

## Estrutura do Projeto

- `seq.c`: Implementação sequencial em C.
- `omp.c`: Implementação paralela utilizando OpenMP.
- `benchmark.py`: Script Python para automatizar a compilação e execução de benchmarks.
- `inputs/`: Diretório contendo arquivos de teste. Cada arquivo contém um par de grafos que podem ou não ser isomórficos.

## Compilação

Embora o script de benchmark realize a compilação automaticamente, você pode compilar as implementações manualmente utilizando o `gcc`:

```bash
# Sequencial
gcc -O3 seq.c -o seq

# Paralela (OpenMP)
gcc -O3 -fopenmp omp.c -o omp
```

## Execução

Os programas leem a definição dos grafos a partir da entrada padrão (`stdin`). O formato esperado é o número de vértices e arestas, seguido pelos pares de adjacência.

Exemplo de execução:

```bash
./seq < inputs/1_iso.txt
./omp < inputs/1_iso.txt
```

O programa retornará o tempo de execução em segundos.

## Configuração de Saída

Os arquivos `seq.c` e `omp.c` possuem uma diretiva de pré-processador na função `main` para alternar a verbosidade da saída:

- `#if 1`: Modo padrão. Exibe apenas o tempo de execução (ideal para benchmarks).
- `#if 0`: Modo verboso. Exibe o tempo e uma mensagem indicando se os grafos são isomórficos ou não.

O tempo de execução é sempre impresso na saída padrão (`stdout`).

## Benchmarking

Você pode utilizar o script `benchmark.py` para compilar e testar automaticamente o desempenho de ambas as versões. O script exige o número de rodadas por teste como argumento:

```bash
python3 benchmark.py 5
```

O script automatiza o processo de compilação antes de iniciar os testes. Os resultados serão salvos em `results.csv`.

> **Importante:** Para que o script de benchmark funcione corretamente, os programas devem estar configurados para exibir **apenas o tempo de execução** (leia a seção de [Configuração de Saída](#configuração-de-saída)).
