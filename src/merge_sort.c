/*
 * merge_sort.c
 * Implementação do Merge Sort sequencial e paralelo (OpenMP) e das
 * funções de verificação. As declarações estão em merge_sort.h.
 * Arquitetura de Computadores II (AED N2) - PUC Goiás - Grupo 6
 */
#include <string.h>
#include <omp.h>
#include "merge_sort.h"

/* ===================== Geração do vetor ===================== */

void preencher_aleatorio(int *v, long n, unsigned int semente) {
    for (long i = 0; i < n; i++) {
        semente = semente * 1664525u + 1013904223u;   /* LCG clássico */
        v[i] = (int)(semente >> 8);                   /* valores de 0 a 16.777.215 */
    }
}

/* ===================== Funções internas (static) ===================== */

/*
 * intercalar
 * Junta duas metades já ordenadas, [lo, mid) e [mid, hi), em uma só.
 * Compara o menor elemento de cada metade e copia para tmp; depois
 * copia o resultado de volta para a.
 */
static void intercalar(int *a, int *tmp, long lo, long mid, long hi) {
    long i = lo, j = mid, k = lo;
    while (i < mid && j < hi)
        tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi)  tmp[k++] = a[j++];
    memcpy(a + lo, tmp + lo, (size_t)(hi - lo) * sizeof(int));
}

/*
 * merge_sort_seq
 * Merge Sort recursivo sequencial sobre o intervalo [lo, hi).
 * Divide ao meio, ordena cada metade e intercala.
 */
static void merge_sort_seq(int *a, int *tmp, long lo, long hi) {
    if (hi - lo < 2) return;                 /* 0 ou 1 elemento: já ordenado */
    long mid = lo + (hi - lo) / 2;
    merge_sort_seq(a, tmp, lo, mid);
    merge_sort_seq(a, tmp, mid, hi);
    intercalar(a, tmp, lo, mid, hi);
}

/*
 * merge_sort_par
 * Mesma lógica da versão sequencial, mas cada metade vira uma TASK OpenMP,
 * que qualquer thread livre pode executar.
 */
static void merge_sort_par(int *a, int *tmp, long lo, long hi) {
    if (hi - lo < 2) return;
    if (hi - lo < CUTOFF) {                  /* trecho pequeno: sem tasks */
        merge_sort_seq(a, tmp, lo, hi);
        return;
    }
    long mid = lo + (hi - lo) / 2;

    #pragma omp task                         /* metade esquerda = uma tarefa */
    merge_sort_par(a, tmp, lo, mid);

    #pragma omp task                         /* metade direita = outra tarefa */
    merge_sort_par(a, tmp, mid, hi);

    #pragma omp taskwait                     /* espera as duas antes de intercalar */
    intercalar(a, tmp, lo, mid, hi);
}

/* ===================== Funções públicas ===================== */

void ordenar_sequencial(int *a, int *tmp, long n) {
    merge_sort_seq(a, tmp, 0, n);
}

void ordenar_paralelo(int *a, int *tmp, long n, int threads) {
    #pragma omp parallel num_threads(threads)   /* cria as threads */
    {
        #pragma omp single                      /* só UMA thread inicia a recursão; */
        merge_sort_par(a, tmp, 0, n);           /* as demais executam as tasks      */
    }
}

int esta_ordenado(const int *a, long n) {
    for (long i = 1; i < n; i++)
        if (a[i - 1] > a[i]) return 0;
    return 1;
}

int sao_iguais(const int *a, const int *b, long n) {
    return memcmp(a, b, (size_t)n * sizeof(int)) == 0;
}
