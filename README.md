# Simulador de Memória Virtual

Trabalho Prático 2 — Sistemas Operacionais

## Objetivo

Simular o funcionamento de um sistema de memória virtual, implementando diferentes algoritmos de substituição de páginas e estruturas de tabelas de páginas. O programa lê um arquivo `.log` com acessos à memória (endereço hexadecimal e operação `R`/`W`) e avalia o desempenho de cada estrutura em termos de tempo de acesso e consumo de memória.

## Algoritmos de Substituição de Páginas

| Algoritmo | Descrição |
|-----------|-----------|
| `lru`     | Least Recently Used — substitui a página menos recentemente acessada |
| `clock`   | Segunda Chance — percorre os quadros em ordem circular usando o bit de referência |
| `nru`     | Not Recently Used — substitui baseado nas classes de prioridade dos bits `R` e `M` |
| `random`  | Seleciona aleatoriamente um quadro para substituição |

## Estruturas de Tabela de Páginas

| Tabela      | Flag       | Descrição |
|-------------|------------|-----------|
| Densa       | `densa`    | Array contíguo com uma entrada por página virtual |
| 2 Níveis    | `2niveis`  | Tabela hierárquica com diretório e tabelas de 2º nível alocadas sob demanda |
| 3 Níveis    | `3niveis`  | Tabela hierárquica com três níveis alocados sob demanda |
| Invertida   | `invertida`| Tabela hash indexada por quadro físico |

## Compilação

```bash
make
```

## Execução

```
./simulator <algoritmo> <arquivo.log> <tam_pagina_kb> <tam_mem_kb> [tabela] [debug]
```

### Sem flags adicionais — todas as tabelas

Executa a simulação comparando todas as quatro estruturas simultaneamente:

```bash
./simulator lru logs/arquivo.log 4 128
```

### Especificando uma tabela

Executa a simulação usando apenas a estrutura de tabela indicada:

```bash
./simulator lru logs/arquivo.log 4 128 densa
./simulator lru logs/arquivo.log 4 128 2niveis
./simulator lru logs/arquivo.log 4 128 3niveis
./simulator lru logs/arquivo.log 4 128 invertida
```

### Modo debug — sem tabela específica

Exibe o detalhamento de cada acesso (PAGE HIT / PAGE FAULT, quadro alocado, página despejada) para todas as tabelas:

```bash
./simulator lru logs/arquivo.log 4 128 debug
```

### Modo debug — com tabela específica

Exibe o detalhamento de cada acesso apenas para a tabela selecionada:

```bash
./simulator lru logs/arquivo.log 4 128 densa debug
./simulator lru logs/arquivo.log 4 128 2niveis debug
./simulator lru logs/arquivo.log 4 128 3niveis debug
./simulator lru logs/arquivo.log 4 128 invertida debug
```

## Saída

A saída é composta de duas partes: primeiro o resumo do simulador, depois o relatório de desempenho de cada tabela ativa.

### Resumo do simulador

Exibe os parâmetros da execução e o resultado da simulação de substituição de páginas:

```
Executando o simulador...
Arquivo de entrada: logs/arquivo.log
Tamanho da memoria: 128 KB
Tamanho das paginas: 4 KB
Tecnica de reposicao: lru
Paginas lidas: 48254
Paginas escritas: 9674
```

- **Páginas lidas** — total de page faults ocorridos (páginas carregadas da memória secundária)
- **Páginas escritas** — total de páginas sujas (modificadas) que foram despejadas e precisaram ser gravadas de volta

### Relatório de desempenho por tabela

Em seguida, para cada tabela ativa, é exibido o tempo total de acesso e o consumo de memória:

```
======================================================
 Avaliação de Desempenho — Tabela: Densa
======================================================
Tempo total         : 17756426 ns
Memória utilizada   : 4194304 bytes
Acessos simulados   : 1000000
Tempo médio/acesso  : 17 ns
======================================================
```