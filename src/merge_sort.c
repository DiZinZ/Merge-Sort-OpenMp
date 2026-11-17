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
 * ANÁLISE DE DEPENDÊNCIAS DE DADOS (relatório, seção 3)
 *   RAW = leitura depois de escrita (dependência verdadeira)
 *   WAR = escrita depois de leitura (dependência de nome / anti-dependência)
 *   WAW = escrita depois de escrita (dependência de nome / de saída)
 * Os comentários [RAW-n], [WAR-n] e [WAW-n] abaixo apontam cada ocorrência.
 */

/*
 * intercalar
 * Junta duas metades já ordenadas, [lo, mid) e [mid, hi), em uma só.
 * Copia os menores elementos para tmp e depois copia tmp de volta para a.
 */
static void intercalar(int *a, int *tmp, long lo, long mid, long hi) {
    long i = lo, j = mid, k = lo;

    /* [RAW-1] Este trecho LÊ a[lo..hi), escrito pelas duas chamadas recursivas
     *         das metades. A intercalação só pode começar depois delas.
     * [RAW-2] i, j e k são lidos e escritos a cada volta: a volta seguinte usa
     *         o valor que a anterior escreveu, e a escolha do próximo elemento
     *         depende de i e j. Por isso este laço é SEQUENCIAL.
     * [WAR-2] a[i] <= a[j] LÊ i e j; logo depois i++ ou j++ os ESCREVE
     *         (dependência de nome em escalares, resolvida com registradores).
     * [WAW-2] tmp[k] só é escrito na faixa [lo, hi) deste trecho. Faixas de
     *         trechos diferentes não se cruzam, então duas tasks nunca
     *         escrevem na mesma posição de tmp.
     */
    while (i < mid && j < hi)
        tmp[k++] = (a[i] <= a[j]) ? a[i++] : a[j++];
    while (i < mid) tmp[k++] = a[i++];
    while (j < hi)  tmp[k++] = a[j++];

    /* [RAW-3] memcpy LÊ tmp[lo..hi), escrito pelos laços acima.
     * [WAR-1] memcpy ESCREVE em a[lo..hi) depois que os laços LERAM a[i] e a[j];
     *         copiar antes destruiria valores ainda não lidos.
     * [WAW-1] a[lo..hi) já foi escrito pelas intercalações das metades; esta
     *         cópia o ESCREVE de novo, e a ordem das escritas precisa ser
     *         mantida (o pai escreve por último).
     */
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
 * Mesma lógica da versão sequencial, mas cada metade vira uma TASK OpenMP.
 *
 * O QUE PODE SER PARALELO: as duas metades. Uma lê e escreve só em a[lo..mid)
 * e tmp[lo..mid); a outra, só em a[mid..hi) e tmp[mid..hi). Os conjuntos de
 * leitura e escrita são disjuntos: NÃO há RAW, WAR nem WAW entre as duas tasks.
 *
 * O QUE É SEQUENCIAL: a intercalação (RAW-1, RAW-2, RAW-3, WAR-1, WAW-1).
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

    /* taskwait respeita [RAW-1] e [WAW-1]: o pai só intercala e escreve em
     * a[lo..hi) depois que as duas tasks filhas terminaram. */
    #pragma omp taskwait
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
