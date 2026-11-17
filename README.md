# Relatório – Merge Sort Sequencial e Paralelo com OpenMP

**Disciplina:** Arquitetura de Computadores II (AED N2) – PUC Goiás – Grupo 6
**Integrantes:** Dillan Fernandes de Almeida Silva, Lázaro Mateus Quixabeira Santos, Kauá dos Santos Leão, Leonardo Anacleto Maciel
**N:** 240.000 elementos (24 letras × 10.000)

## 1. Ambientes de teste
*(Preencher com os comandos de hardware do README. Não use valores presumidos.)*

| Item | Local (Windows 11) | GitHub Codespaces |
|---|---|---|
| Processador | | |
| Núcleos / threads lógicas | | |
| Memória RAM | | |
| Versão do GCC / OpenMP | | |

## 2. Metodologia
Merge Sort sequencial e paralelo (OpenMP tasks, cutoff = 5000). Vetor gerado com semente fixa, média de 10 repetições com `omp_get_wtime()`, verificação de ordenação e de igualdade entre versões.

## 3. Resultados
*(Copie os valores de `results/*.csv`.)*

| Ambiente | Threads | T_seq (s) | T_par (s) | Speedup | Eficiência | p (Amdahl) |
|---|---|---|---|---|---|---|
| | 1 | | | | | n/a |
| | 2 | | | | | |
| | 4 | | | | | |
| | 8 | | | | | |

## 4. Análise
- O paralelismo melhorou o desempenho? Em quantas threads o speedup parou de crescer?
- A eficiência caiu com mais threads? Por quê (sobrecarga de tasks, merge final sequencial, memória)?
- Quantos processadores lógicos o ambiente realmente tinha? Threads acima disso não trazem ganho.
- Fração paralelizável estimada (Amdahl) e speedup máximo teórico 1/(1−p).
- Diferenças entre Windows local e Codespaces.

## 5. Conclusão
*(Escrever após analisar os resultados reais.)*
