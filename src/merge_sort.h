/*
 * merge_sort.h
 * Interface da biblioteca de ordenação do projeto (Merge Sort sequencial e paralelo).
 * Arquitetura de Computadores II (AED N2) - PUC Goiás - Grupo 6
 *
 * Aqui ficam apenas as DECLARAÇÕES (nomes, parâmetros e o que cada função faz).
 * A implementação está em merge_sort.c.
 */
#ifndef MERGE_SORT_H
#define MERGE_SORT_H

/* Tamanho mínimo de um trecho para criar tasks OpenMP.
 * Abaixo disso, o trecho é ordenado sequencialmente (criar tasks muito
 * pequenas custa mais do que o ganho do paralelismo). */
#define CUTOFF 5000

/*
 * preencher_aleatorio
 * Preenche o vetor v (n elementos) com valores pseudoaleatórios.
 * Usa um gerador próprio (LCG) para que o vetor seja IDÊNTICO no Windows
 * e no Linux, o que torna os testes comparáveis.
 *   v       - vetor a preencher
 *   n       - quantidade de elementos
 *   semente - valor inicial do gerador (mesma semente = mesmo vetor)
 */
void preencher_aleatorio(int *v, long n, unsigned int semente);

/*
 * ordenar_sequencial
 * Ordena a[0..n-1] em ordem crescente com Merge Sort sequencial.
 *   a   - vetor a ordenar (modificado no lugar)
 *   tmp - vetor auxiliar com pelo menos n elementos
 *   n   - quantidade de elementos
 */
void ordenar_sequencial(int *a, int *tmp, long n);

/*
 * ordenar_paralelo
 * Ordena a[0..n-1] em ordem crescente com Merge Sort paralelo (OpenMP tasks).
 *   a       - vetor a ordenar (modificado no lugar)
 *   tmp     - vetor auxiliar com pelo menos n elementos
 *   n       - quantidade de elementos
 *   threads - número de threads da região paralela
 */
void ordenar_paralelo(int *a, int *tmp, long n, int threads);

/*
 * esta_ordenado
 * Retorna 1 se a[0..n-1] está em ordem crescente; 0 caso contrário.
 */
int esta_ordenado(const int *a, long n);

/*
 * sao_iguais
 * Retorna 1 se a[0..n-1] e b[0..n-1] têm exatamente os mesmos valores
 * nas mesmas posições; 0 caso contrário.
 */
int sao_iguais(const int *a, const int *b, long n);

#endif /* MERGE_SORT_H */
