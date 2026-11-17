# Análise de Desempenho do Algoritmo Merge Sort Sequencial e Paralelo com OpenMP em C

Projeto de **Arquitetura de Computadores II (AED N2)** – PUC Goiás – **Grupo 6**

**Integrantes:** Dillan Fernandes de Almeida Silva, Lázaro Mateus Quixabeira Santos, Kauá dos Santos Leão, Leonardo Anacleto Maciel

**Tamanho do vetor:** 6 + 6 + 4 + 8 = 24 letras → N = 24 × 10.000 = **240.000 elementos**.

## Estrutura do repositório

| Caminho | Função |
|---|---|
| `src/main.c` | Programa principal: lê as opções, mede os tempos, calcula speedup/eficiência/Amdahl e grava o CSV |
| `src/merge_sort.h` | Interface (declarações e descrição de cada função) da parte de ordenação |
| `src/merge_sort.c` | Implementação do Merge Sort sequencial e paralelo (OpenMP) e das verificações |
| `results/resultados_local.csv` | Resultados reais medidos no Windows 11 (gerado pelo programa) |
| `results/resultados_codespaces.csv` | Resultados reais medidos no GitHub Codespaces (gerado pelo programa) |
| `docs/relatorio.md` / `docs/relatorio.pdf` | Relatório nos 8 itens da atividade, com dependências RAW/WAR/WAW, paralelização, resultados e EXTRA-EXTRA |
| `docs/img/` | Prints reais das execuções e gráfico de speedup |
| `README.md` | Este guia |
| `.gitignore` | Evita enviar executáveis ao Git |

## Como o código funciona

- **Saída do programa:** imprime os nomes dos integrantes, a conta de N (24 × 10.000 = 240.000) e as informações do ambiente.
- **Dependências:** os comentários `[RAW-n]`, `[WAR-n]` e `[WAW-n]` em `src/merge_sort.c` apontam cada dependência de dados; a explicação está na seção 3 do relatório.
- **Organização:** `merge_sort.h/.c` guardam a ordenação e as verificações; `main.c` cuida só de opções, tempos, cálculos e CSV.
- **Sequencial:** `merge_sort_seq` divide o vetor ao meio, ordena cada metade e intercala (`intercalar`).
- **Paralelo:** `merge_sort_par` faz o mesmo, mas cada metade vira uma `#pragma omp task`; `#pragma omp taskwait` espera as duas antes de intercalar. Em `ordenar_paralelo`, `#pragma omp parallel` cria as threads e `#pragma omp single` faz com que apenas uma delas inicie a recursão (as outras executam as tasks).
- **CUTOFF (5000):** abaixo desse tamanho a recursão volta ao modo sequencial, pois criar tasks minúsculas custa mais do que ganha.
- **Verificações:** `esta_ordenado` confere a ordem; `sao_iguais` compara o resultado sequencial com o paralelo.
- **Medição:** `omp_get_wtime()`, média de várias repetições (padrão 10), cada uma precedida por 1 execução de aquecimento não contada. A mesma entrada (semente fixa) é usada em todas as execuções.
- **Cálculos:** Speedup = T_seq / T_par; Eficiência = Speedup / threads; Amdahl: p = (1 − 1/S) / (1 − 1/t), para t > 1.

## Compilar e executar

### Linux / GitHub Codespaces
```bash
sudo apt update && sudo apt upgrade -y
sudo apt install -y build-essential
gcc --version
echo | gcc -fopenmp -dM -E - | grep -i _OPENMP     # deve mostrar #define _OPENMP ...

gcc -O2 -fopenmp -Wall -Wextra -o merge_sort src/main.c src/merge_sort.c
./merge_sort -a codespaces -o results/resultados_codespaces.csv
```

Registre o hardware do Codespaces (cole no relatório):
```bash
nproc
lscpu | grep -E "Model name|^CPU\(s\)|Thread|Core|Socket|MHz"
free -h
cat /etc/os-release | head -2
```

### Windows 11 (MSYS2 + GCC)
1. Instale o MSYS2 (PowerShell): `winget install MSYS2.MSYS2`
2. Abra **MSYS2 UCRT64** e rode:
```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain
```
3. Instale o Git no Windows, se necessário: `winget install Git.Git`.
4. Adicione `C:\msys64\ucrt64\bin` ao PATH do Windows e reabra o VS Code/terminal. Se `gcc --version` mostrar uma versão antiga (ex.: MinGW.org 6.3.0), coloque o GCC novo na frente nesta sessão com `$env:Path = "C:\msys64\ucrt64\bin;" + $env:Path`.
5. No terminal do VS Code (PowerShell), dentro da pasta do projeto (se os acentos aparecerem errados, rode `chcp 65001` antes):
```powershell
gcc --version
gcc -O2 -fopenmp -Wall -Wextra -o merge_sort.exe src/main.c src/merge_sort.c
.\merge_sort.exe -a local -o results/resultados_local.csv
```
Hardware no Windows:
```powershell
Get-CimInstance Win32_Processor | Select-Object Name, NumberOfCores, NumberOfLogicalProcessors, MaxClockSpeed
Get-CimInstance Win32_ComputerSystem | Select-Object @{n="RAM_GB";e={[math]::Round($_.TotalPhysicalMemory/1GB,1)}}
```

### Opções do programa
`-n N` (padrão 240000) · `-r REPS` (padrão 10) · `-t 1,2,4,8` · `-a ambiente` · `-o arquivo.csv` (acrescenta linhas; cria o cabeçalho se o arquivo estiver vazio).

> **Dica:** feche programas pesados durante a medição e rode o teste mais de uma vez. Se rodar de novo no mesmo CSV, as linhas são acrescentadas; apague o conteúdo (mantendo o cabeçalho) para recomeçar.

## Resultados

Tempos reais medidos com `omp_get_wtime()`: **mediana de 3 execuções** do programa (cada uma com 10 repetições, após 1 execução de aquecimento não contada), N = 240.000. Os dados completos estão em `results/` e a análise em `docs/relatorio.md` (PDF em `docs/relatorio.pdf`).

### Local (Windows 11)

Hardware: AMD Ryzen 9 5900X 12-Core, **12 núcleos / 24 processadores lógicos**, 31,9 GB de RAM, GCC 16.2.0 (MSYS2 UCRT64), OpenMP 5.1. Tempo sequencial mediano: 0,014300 s.

| Threads | T_par mediano (s) | Speedup | Faixa (3 rodadas) | Eficiência | p (Amdahl) |
|---|---|---|---|---|---|
| 1 | 0,014000 | 1,021 | 1,014 a 1,021 | 1,021 | n/a |
| 2 | 0,008100 | 1,765 | 1,765 a 1,844 | 0,883 | 0,867 |
| 4 | 0,005600 | 2,554 | 2,509 a 2,704 | 0,638 | 0,811 |
| 8 | 0,003900 | 3,667 | 3,575 a 3,744 | 0,458 | 0,831 |

### GitHub Codespaces

Hardware: AMD EPYC 7763 64-Core Processor, **2 processadores lógicos** (1 núcleo físico, 2 threads por núcleo), 7,8 GiB de RAM, GCC 13.3.0 (Ubuntu 24.04), OpenMP 4.5. Tempo sequencial mediano: 0,021263 s.

| Threads | T_par mediano (s) | Speedup | Faixa (3 rodadas) | Eficiência | p (Amdahl) |
|---|---|---|---|---|---|
| 1 | 0,021251 | 1,001 | 0,858 a 1,146 | 1,001 | n/a |
| 2 | 0,021047 | 1,010 | 0,990 a 1,135 | 0,505 | 0,020 |
| 4 | 0,019660 | 1,082 | 0,906 a 1,603 | 0,270 | 0,101 |
| 8 | 0,015403 | 1,380 | 1,238 a 1,557 | 0,173 | 0,315 |

Em todas as execuções o vetor ficou ordenado e idêntico ao resultado sequencial.

### Resumo

- **Local:** o tempo cai em todas as etapas (72,7% menor com 8 threads, speedup 3,667) e as 3 execuções ficaram próximas. A fração paralelizável estimada fica entre 0,81 e 0,87.
- **Codespaces:** o ganho é pequeno e instável (speedup 1,010 com 2 threads e 1,380 com 8), pois a VM compartilhada tem só 2 processadores lógicos. A Lei de Amdahl não descreve bem esse ambiente.
- O computador local ordena sequencialmente em menos tempo (0,0143 s contra 0,0213 s) e escala bem melhor. Detalhes e limitações em `docs/relatorio.md`.