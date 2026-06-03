Trabalho de sistemas operacionais

Objetivo:

Simular o funcionamento de um sistema de memória virtual, implementando diferentes algoritmos de substituição de páginas e tabelas de páginas. Para tanto, o programa recebe como entrada o tipo de algoritmo a ser utilizado, um arquivo .log contendo acessos à memória, o tamanho da página em kilobytes (KB) e o tamanho da memória em kilobytes (KB).

Algoritmos:

- lru (least recently used)
- clock
- nru (not recently used)
- random

Exemplo de execução:

```bash
./simulator lru logs/arquivo.log 4 16
```