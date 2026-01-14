# ALNS para o Problema de Roteamento de Veículos Capacitado (CVRP)

Este repositório contém a implementação da metaheurística **Adaptive Large Neighborhood Search (ALNS)** aplicada ao **Problema de Roteamento de Veículos Capacitado (CVRP)**.

O projeto foi desenvolvido como parte de um **Trabalho de Conclusão de Curso (TCC)**, com foco em uma implementação fiel do **ALNS “puro”** (sem busca local externa), utilizando **Simulated Annealing** como critério de aceitação e uma **validação estatística rigorosa** baseada em amostragem de população infinita.

---

## 📋 Sobre o Projeto

O **CVRP** é um problema clássico de otimização combinatória no qual uma frota de veículos deve atender um conjunto de clientes com demandas conhecidas, minimizando o custo total das rotas e respeitando a capacidade dos veículos.

Esta implementação utiliza o algoritmo **ALNS**, que destrói e repara iterativamente a solução para explorar o espaço de busca, adaptando dinamicamente os pesos dos operadores conforme o seu desempenho histórico (mecanismo de **“Roleta Viciada”**).

---

## ✨ Características Principais

- **Linguagem:** C++17 (foco em performance e precisão)
- **Padrão de Distância:** Distâncias Euclidianas arredondadas para inteiros (padrão TSPLIB / acadêmico)
- **Metaheurística:** ALNS guiado por Simulated Annealing
- **Ferramentas Auxiliares:** Scripts em Python para visualização gráfica das rotas e validação estatística automatizada

---

## ⚙️ Metodologia e Operadores

O algoritmo alterna dinamicamente entre diferentes heurísticas de **destruição** e **reparo**.

### 🔴 Operadores de Remoção (Destroy)

- **Random Removal**  
  Remove `q` clientes aleatoriamente (diversificação)

- **Worst Removal**  
  Remove clientes com alto custo marginal, utilizando uma função de aleatoriedade controlada (`y^p`) para evitar determinismo

- **Shaw Removal (Relatedness)**  
  Remove clientes com base na similaridade (principalmente distância), preservando estruturas geográficas

### 🟢 Operadores de Inserção (Repair)

- **Greedy Insertion (com Ruído)**  
  Insere clientes na posição de menor custo global, aplicando ruído aleatório para evitar ótimos locais

- **k-Regret Insertion (com Ruído)**  
  Prioriza clientes cujo adiamento causaria maior “arrependimento” (diferença de custo entre as melhores inserções)

---

## 🚀 Como Executar

### ✅ Pré-requisitos

- Compilador C++ com suporte a **C++17** (`g++`, `clang` ou `MSVC`)
- **Python 3.x**
- Biblioteca Python:
  - `matplotlib`

---

### 1️⃣ Compilação (C++)

```bash
g++ -O3 -std=c++17 main.cpp -o alns_cvrp
```

> **Nota:** A flag `-O3` é recomendada para otimização de performance.

---

### 2️⃣ Execução Simples

Execução em uma instância no formato **TSPLIB (`.vrp`)**:

#### Linux / macOS
```bash
./alns_cvrp Instancias/A-n32-k5.vrp
```

#### Windows
```bash
alns_cvrp.exe Instancias\A-n32-k5.vrp
```

O programa gera o arquivo `solution_data.txt` com as coordenadas da melhor solução encontrada.

---

### 3️⃣ Visualização das Rotas

```bash
python plot_routes.py
```

---

## 📊 Validação Estatística (Metodologia do TCC)

Como o ALNS é um algoritmo **estocástico**, os resultados variam a cada execução (população infinita). Para garantir a confiabilidade dos resultados apresentados, foi implementado um processo de validação estatística automatizado.

### Passos:

1. Abra `run_statistical_validation.py`
2. Configure:
   - `BKS_LITERATURA` (valor ótimo conhecido)
   - `NUM_EXECUCOES` (n calculado via fórmula estatística)
3. Execute:
```bash
python run_statistical_validation.py
```

O script executa o algoritmo `n` vezes e gera:
- Gap Médio (%)
- Intervalo de Confiança
- Arquivo CSV com os dados

---

## 🔧 Configuração de Parâmetros

Os hiperparâmetros são definidos no início da função `main` em `main.cpp`.

Valores padrão:

- `maxIter`: `50.000`
- `maxTimeSeconds`: tempo limite
- `coolingRate`: `0.99985`
- `minRemPct / maxRemPct`: `5% – 45%`

---

## 📝 Autores

- **[Seu Nome]** – Pesquisa e Desenvolvimento

---

## 📚 Referências

Este projeto utiliza instâncias da biblioteca **TSPLIB**, amplamente adotada em benchmarks acadêmicos para problemas de roteamento de veículos.
