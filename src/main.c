/*
 * main.c
 * Programa principal: lê as opções, mede os tempos (sequencial x paralelo),
 * verifica os resultados, calcula speedup/eficiência/Amdahl e grava o CSV.
 * A ordenação em si está em merge_sort.c / merge_sort.h.
 *
 * Análise de Desempenho do Algoritmo Merge Sort Sequencial e Paralelo com OpenMP em C
 * Arquitetura de Computadores II (AED N2) - PUC Goiás - Grupo 6
 *
 * Compilar:
 *     gcc -O2 -fopenmp -Wall -Wextra -o merge_sort src/main.c src/merge_sort.c
 *
 * Executar (padrão: N = 240000, 10 repetições, threads 1,2,4,8):
 *     ./merge_sort -a local -o results/resultados_local.csv
 *
 * Opções:
 *     -n <N>       quantidade de elementos        (padrão 240000)
 *     -r <REPS>    repetições de cada medição     (padrão 10)
 *     -t <lista>   threads separadas por vírgula  (padrão 1,2,4,8)
 *     -a <nome>    nome do ambiente (ex.: local, codespaces)
 *     -o <arquivo> arquivo CSV de saída (os resultados são acrescentados)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "merge_sort.h"
#ifdef _WIN32
#include <windows.h>   /* para acentos corretos no terminal do Windows */
#endif

#define N_PADRAO      240000   /* 24 letras dos nomes x 10.000 */
#define REPS_PADRAO   10
#define SEMENTE       12345u   /* semente fixa: mesmo vetor em todas as execuções */

/* Imprime os integrantes e o cálculo de N .
   Quantidade de letras escrita à mão, pois os nomes têm acentos. */
static void imprimir_identificacao(long n_usado) {
    const char *nomes[] = {"Dillan", "Lázaro", "Kauá", "Leonardo"};
    const int letras[]  = {6, 6, 4, 8};
    int total = 0;
    printf("Grupo 6 - Arquitetura de Computadores II (AED N2) - PUC Goiás\n");
    printf("Integrantes (primeiro nome = letras):\n");
    for (int i = 0; i < 4; i++) {
        printf("  %s = %d\n", nomes[i], letras[i]);
        total += letras[i];
    }
    printf("Total de letras = %d  ->  N = %d x 10.000 = %d elementos\n",
           total, total, total * 10000);
    if (n_usado != (long)total * 10000)
        printf("ATENÇÃO: N usado nesta execução (%ld) difere do N do grupo.\n", n_usado);
    printf("\n");
}

/* Imprime no terminal as informações do ambiente (copie para o relatório). */
static void imprimir_ambiente(const char *ambiente, long n, int reps) {
    printf("=== Merge Sort: sequencial x paralelo (OpenMP) ===\n");
    printf("Ambiente: %s\n", ambiente);
    printf("N = %ld | repetições = %d\n", n, reps);
    printf("Processadores lógicos visíveis (omp_get_num_procs): %d\n", omp_get_num_procs());
#ifdef _OPENMP
    printf("Versão OpenMP (_OPENMP): %d\n", _OPENMP);
#endif
#ifdef __VERSION__
    printf("Compilador: %s\n", __VERSION__);
#endif
}

/* Abre o CSV em modo "acrescentar" e escreve o cabeçalho se o arquivo estiver vazio. */
static FILE *abrir_csv(const char *caminho) {
    FILE *csv = fopen(caminho, "a+");
    if (!csv) return NULL;
    fseek(csv, 0, SEEK_END);
    if (ftell(csv) == 0)
        fprintf(csv, "ambiente,n,repeticoes,threads,tempo_seq_s,tempo_par_s,"
                     "speedup,eficiencia,fracao_paralelizavel_amdahl,"
                     "diferenca_s,diferenca_percentual,ordenado_seq,ordenado_par,iguais\n");
    return csv;
}

int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);   /* mostra "á", "ç" etc. corretamente */
#endif
    long n = N_PADRAO;
    int reps = REPS_PADRAO;
    int lista_threads[16] = {1, 2, 4, 8};
    int qtd_threads = 4;
    const char *ambiente = "local";
    const char *arq_csv = NULL;

    /* ---- Leitura das opções da linha de comando ---- */
    for (int i = 1; i < argc - 1; i += 2) {
        if      (!strcmp(argv[i], "-n")) n = atol(argv[i + 1]);
        else if (!strcmp(argv[i], "-r")) reps = atoi(argv[i + 1]);
        else if (!strcmp(argv[i], "-a")) ambiente = argv[i + 1];
        else if (!strcmp(argv[i], "-o")) arq_csv = argv[i + 1];
        else if (!strcmp(argv[i], "-t")) {
            qtd_threads = 0;
            char copia[128];
            strncpy(copia, argv[i + 1], sizeof(copia) - 1);
            copia[sizeof(copia) - 1] = '\0';
            for (char *p = strtok(copia, ","); p && qtd_threads < 16; p = strtok(NULL, ","))
                lista_threads[qtd_threads++] = atoi(p);
        } else {
            fprintf(stderr, "Opção desconhecida: %s\n", argv[i]);
            return 1;
        }
    }
    if (n < 1 || reps < 1 || qtd_threads < 1) {
        fprintf(stderr, "Parâmetros inválidos.\n");
        return 1;
    }

    /* ---- Alocação: original (entrada fixa), seq, par e tmp (auxiliar) ---- */
    int *original = malloc((size_t)n * sizeof(int));
    int *seq      = malloc((size_t)n * sizeof(int));
    int *par      = malloc((size_t)n * sizeof(int));
    int *tmp      = malloc((size_t)n * sizeof(int));
    if (!original || !seq || !par || !tmp) {
        fprintf(stderr, "Memória insuficiente.\n");
        return 1;
    }
    preencher_aleatorio(original, n, SEMENTE);

    imprimir_identificacao(n);
    imprimir_ambiente(ambiente, n, reps);

    /* ---- Tempo sequencial (mesma base para todas as quantidades de threads) ---- */
    /* Aquecimento (NÃO medido): a primeira execução paga o custo de tocar a memória
       pela primeira vez (page faults) e de "acordar" a CPU, o que inflaria T_seq. */
    memcpy(seq, original, (size_t)n * sizeof(int));
    ordenar_sequencial(seq, tmp, n);

    double soma_seq = 0.0;
    int ok_seq = 1;
    for (int r = 0; r < reps; r++) {
        memcpy(seq, original, (size_t)n * sizeof(int));
        double t0 = omp_get_wtime();
        ordenar_sequencial(seq, tmp, n);
        soma_seq += omp_get_wtime() - t0;
        if (!esta_ordenado(seq, n)) ok_seq = 0;
    }
    double tempo_seq = soma_seq / reps;
    printf("\n(Cada medição é precedida por 1 execução de aquecimento, não contada.)");
    printf("\nSequencial: tempo medio = %.6f s | ordenado = %s\n", tempo_seq, ok_seq ? "SIM" : "NAO");

    FILE *csv = NULL;
    if (arq_csv) {
        csv = abrir_csv(arq_csv);
        if (!csv) { fprintf(stderr, "Não foi possível abrir %s\n", arq_csv); return 1; }
    }

    printf("\n%-8s %-12s %-12s %-9s %-11s %-9s %-9s %-7s\n",
           "Threads", "T_seq (s)", "T_par (s)", "Speedup", "Eficiencia",
           "Amdahl p", "Ordenado", "Igual");

    /* ---- Paralelo para cada quantidade de threads ---- */
    for (int c = 0; c < qtd_threads; c++) {
        int t = lista_threads[c];
        double soma_par = 0.0;
        int ok_par = 1, iguais = 1;

        /* Aquecimento (NÃO medido): cria as threads e prepara o escalonador de tasks. */
        memcpy(par, original, (size_t)n * sizeof(int));
        ordenar_paralelo(par, tmp, n, t);

        for (int r = 0; r < reps; r++) {
            memcpy(par, original, (size_t)n * sizeof(int));
            double t0 = omp_get_wtime();
            ordenar_paralelo(par, tmp, n, t);
            soma_par += omp_get_wtime() - t0;
            if (!esta_ordenado(par, n)) ok_par = 0;
            if (!sao_iguais(seq, par, n)) iguais = 0;
        }
        double tempo_par = soma_par / reps;
        double speedup = tempo_seq / tempo_par;
        double eficiencia = speedup / t;
        /* Lei de Amdahl: S = 1 / ((1-p) + p/t)  =>  p = (1 - 1/S) / (1 - 1/t)  (t > 1) */
        double p = (t > 1) ? (1.0 - 1.0 / speedup) / (1.0 - 1.0 / t) : 0.0;
        double dif = tempo_seq - tempo_par;
        double dif_pct = dif / tempo_seq * 100.0;

        if (t > 1)
            printf("%-8d %-12.6f %-12.6f %-9.3f %-11.3f %-9.3f %-9s %-7s\n",
                   t, tempo_seq, tempo_par, speedup, eficiencia, p,
                   ok_par ? "SIM" : "NAO", iguais ? "SIM" : "NAO");
        else
            printf("%-8d %-12.6f %-12.6f %-9.3f %-11.3f %-9s %-9s %-7s\n",
                   t, tempo_seq, tempo_par, speedup, eficiencia, "n/a",
                   ok_par ? "SIM" : "NAO", iguais ? "SIM" : "NAO");

        if (csv)
            fprintf(csv, "%s,%ld,%d,%d,%.6f,%.6f,%.4f,%.4f,%.4f,%.6f,%.2f,%d,%d,%d\n",
                    ambiente, n, reps, t, tempo_seq, tempo_par, speedup, eficiencia,
                    p, dif, dif_pct, ok_seq, ok_par, iguais);
    }

    if (csv) { fclose(csv); printf("\nResultados acrescentados em %s\n", arq_csv); }
    free(original); free(seq); free(par); free(tmp);
    return 0;
}
