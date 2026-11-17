# Relatório – Merge Sort Sequencial e Paralelo com OpenMP

**Disciplina:** Arquitetura de Computadores II (AED N2) – PUC Goiás – Grupo 6
**Integrantes:** Dillan Fernandes de Almeida Silva, Lázaro Mateus Quixabeira Santos, Kauá dos Santos Leão, Leonardo Anacleto Maciel
**N:** 240.000 elementos (24 letras × 10.000)

## 1. Ambientes de teste

| Item | Local (Windows 11) | GitHub Codespaces |
|---|---|---|
| Processador | AMD Ryzen 9 5900X 12-Core | AMD EPYC 7763 64-Core |
| Núcleos físicos | 12 | 1 |
| Processadores lógicos (OpenMP) | 24 | 2 |
| Clock máximo informado | 3701 MHz | não informado |
| Memória RAM | 31,9 GB | 7,8 GiB |
| Compilador / OpenMP | GCC 16.2.0 (MSYS2 UCRT64) / OpenMP 5.1 (`_OPENMP` = 202111) | GCC 13.3.0 (Ubuntu 24.04) / OpenMP 4.5 (`_OPENMP` = 201511) |

## 2. Metodologia

- Merge Sort sequencial e paralelo (OpenMP tasks com `parallel`, `single`, `task` e `taskwait`), com cutoff de 5.000 elementos.
- Vetor de 240.000 inteiros gerado com semente fixa, igual em todas as execuções e em ambos os ambientes.
- Tempo medido com `omp_get_wtime()`; cada resultado é a média de 10 repetições.
- Verificação em todas as repetições: vetor ordenado e resultado paralelo idêntico ao sequencial.
- Compilação: `gcc -O2 -fopenmp -Wall -Wextra -o merge_sort src/main.c src/merge_sort.c`
- Fórmulas: Speedup = T_seq / T_par; Eficiência = Speedup / threads; fração paralelizável (Amdahl) p = (1 − 1/S) / (1 − 1/t).

## 3. Resultados

### 3.1 Local – Windows 11 (`results/resultados_local.csv`)

Tempo sequencial médio: **0,014000 s**.

| Threads | T_seq (s) | T_par (s) | Speedup | Eficiência | Redução de tempo | p (Amdahl) | Ordenado | Igual ao seq. |
|---|---|---|---|---|---|---|---|---|
| 1 | 0,014000 | 0,013700 | 1,022 | 1,022 | 2,1% | n/a | SIM | SIM |
| 2 | 0,014000 | 0,007600 | 1,842 | 0,921 | 45,7% | 0,914 | SIM | SIM |
| 4 | 0,014000 | 0,005300 | 2,642 | 0,660 | 62,1% | 0,829 | SIM | SIM |
| 8 | 0,014000 | 0,003900 | 3,590 | 0,449 | 72,1% | 0,824 | SIM | SIM |

### 3.2 GitHub Codespaces (`results/resultados_codespaces.csv`)

Tempo sequencial médio: **0,021005 s**.

| Threads | T_seq (s) | T_par (s) | Speedup | Eficiência | Redução de tempo | p (Amdahl) | Ordenado | Igual ao seq. |
|---|---|---|---|---|---|---|---|---|
| 1 | 0,021005 | 0,020707 | 1,014 | 1,014 | 1,4% | n/a | SIM | SIM |
| 2 | 0,021005 | 0,014390 | 1,460 | 0,730 | 31,5% | 0,630 | SIM | SIM |
| 4 | 0,021005 | 0,014030 | 1,497 | 0,374 | 33,2% | 0,443 | SIM | SIM |
| 8 | 0,021005 | 0,016976 | 1,237 | 0,155 | 19,2% | 0,219 | SIM | SIM |

### 3.3 Comparação

| Aspecto | Local (Ryzen 9 5900X) | Codespaces (EPYC 7763) |
|---|---|---|
| Tempo sequencial | 0,014000 s | 0,021005 s |
| Melhor tempo paralelo | 0,003900 s (8 threads) | 0,014030 s (4 threads) |
| Melhor speedup | 3,590 (8 threads) | 1,497 (4 threads) |
| Eficiência com 2 threads | 0,921 | 0,730 |
| Comportamento de 1 a 8 threads | ganho crescente em todas as etapas | ganho some com 4 threads e piora com 8 |

## 4. Análise

### 4.1 Local (Windows 11)

- **O paralelismo melhorou o desempenho?** Sim, e de forma consistente. O tempo caiu em todas as etapas: 45,7% com 2 threads, 62,1% com 4 e 72,1% com 8 (speedup de 3,59).
- **1 thread:** o tempo é praticamente igual ao sequencial (speedup 1,022), ou seja, a sobrecarga de criar as tasks é desprezível.
- **Eficiência:** cai com o aumento das threads (0,921 → 0,660 → 0,449). Isso é esperado: o ganho por thread adicionada diminui. Como o computador tem 24 processadores lógicos, as 8 threads couberam no hardware e o tempo continuou caindo.
- **Lei de Amdahl:** a fração paralelizável estimada foi 0,914 (2 threads), 0,829 (4) e 0,824 (8). Os valores de 4 e 8 threads são bem próximos, então p ≈ 0,82 é uma estimativa razoável. Isso corresponde a um speedup teórico máximo de 1 / (1 − 0,824) ≈ 5,7. Uma explicação provável para a parte que não paraleliza é que a última intercalação junta o vetor inteiro em uma única thread (e a anterior usa apenas 2). A largura de banda de memória e a criação de tasks também podem contribuir, mas isso não foi medido separadamente.
- **Corretude:** em todas as configurações o vetor ficou ordenado e igual ao resultado sequencial.

### 4.2 Codespaces

- **O paralelismo melhorou o desempenho?** Sim, até certo ponto. Com 2 threads o tempo caiu 31,5% (speedup 1,46). O melhor tempo foi com 4 threads (0,014030 s), mas a diferença para 2 threads é muito pequena e pode ser ruído de medição.
- **Limite do hardware:** o ambiente tem só 2 processadores lógicos, que são 2 threads de um único núcleo físico. Com 4 e 8 threads há mais threads do que processadores, e o ganho some ou piora (com 8 threads o speedup cai para 1,237).
- **Lei de Amdahl:** p variou de 0,630 (2 threads) para 0,443 (4) e 0,219 (8). Como deveria ser constante, isso mostra que, nesse ambiente, o limite do hardware pesa mais que a parte sequencial do algoritmo. A estimativa mais confiável é a de 2 threads, **p ≈ 0,63**, pois é a única configuração que cabe no hardware.

### 4.3 Comparação entre os ambientes

- O computador local ordena sequencialmente cerca de 33% mais rápido (0,0140 s contra 0,0210 s) e escala muito melhor: speedup de 3,59 contra 1,50 no máximo do Codespaces.
- A diferença principal é a quantidade de processadores lógicos: 24 contra 2. No Codespaces o paralelismo satura com 2 threads; no computador local o desempenho continua melhorando até 8 threads.
- As estimativas de Amdahl são mais estáveis e mais altas no ambiente local (≈ 0,82) do que no Codespaces, onde o valor depende de quantas threads cabem no hardware.

### 4.4 Limitações

- Os tempos são curtos (4 a 21 ms), então pequenas variações do sistema influenciam o resultado.
- No Windows os tempos médios aparecem arredondados em 0,1 ms (por exemplo, 0,014000 e 0,013700), o que aparentemente indica resolução limitada do relógio; as médias de 10 repetições ficam menos precisas que no Linux.
- O Codespaces é um ambiente compartilhado, e sua carga pode variar entre execuções.
- Os compiladores e as versões de OpenMP são diferentes entre os ambientes (GCC 16.2.0 e 13.3.0).

## 5. Conclusão

O Merge Sort paralelo com OpenMP tasks produziu resultados corretos em todas as execuções e melhorou o desempenho nos dois ambientes. No computador local (Ryzen 9 5900X, 24 processadores lógicos) o tempo caiu de 0,0140 s para 0,0039 s com 8 threads, speedup de 3,59 e eficiência de 0,449. No Codespaces (2 processadores lógicos) o melhor speedup foi 1,50 e o ganho acabou com 2 threads, pois o hardware limita o paralelismo. A fração paralelizável estimada foi de cerca de 0,82 no ambiente local e cerca de 0,63 no Codespaces, o que indica que o algoritmo tem uma parte sequencial relevante (as últimas intercalações) e que o hardware disponível define até onde o paralelismo compensa. Os resultados mostram que mais threads só ajudam enquanto existirem processadores para executá-las e que a eficiência por thread diminui conforme o número de threads aumenta.
