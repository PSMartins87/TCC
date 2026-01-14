ALNS para o Problema de Roteamento de Veículos Capacitado (CVRP)
Este repositório contém a implementação da metaheurística Adaptive Large Neighborhood Search (ALNS) aplicada ao Problema de Roteamento de Veículos Capacitado (CVRP).
O projeto foi desenvolvido no âmbito de um Trabalho de Conclusão de Curso (TCC), focando numa implementação fiel do ALNS "puro" (sem busca local externa), utilizando Simulated Annealing como critério de aceitação e uma validação estatística rigorosa baseada em amostragem de população infinita.
📋 Sobre o Projeto
O CVRP é um problema clássico de otimização combinatória onde uma frota de veículos deve atender a um conjunto de clientes com demandas específicas, minimizando o custo total da rota.
Esta implementação utiliza o algoritmo ALNS, que destrói e repara iterativamente a solução para explorar o espaço de busca, adaptando os pesos dos operadores conforme o seu desempenho durante a execução.
Características Principais
Linguagem: C++17 (foco em performance e precisão).
Padrão de Distância: Distâncias Euclidianas arredondadas para inteiros (padrão TSPLIB/Académico).
Metaheurística: ALNS guiado por Simulated Annealing.
Ferramentas Auxiliares: Scripts em Python para visualização gráfica das rotas e validação estatística automatizada.
⚙️ Metodologia e Operadores
O algoritmo alterna dinamicamente entre diferentes heurísticas. A probabilidade de escolha de cada operador é ajustada por uma "Roleta Viciada" baseada no sucesso de iterações anteriores.
Operadores de Remoção (Destroy)
Random Removal: Remove clientes aleatoriamente (diversificação).
Worst Removal: Remove clientes com alto custo marginal, utilizando uma função de aleatoriedade controlada ($y^p$) para evitar determinismo.
Shaw Removal (Relatedness): Remove clientes baseados em similaridade de distância, preservando estruturas geográficas.
Operadores de Inserção (Repair)
Greedy Insertion (com Ruído): Insere clientes na posição de menor custo. Um fator de ruído aleatório é aplicado na avaliação para evitar ótimos locais.
k-Regret Insertion (com Ruído): Insere clientes que causariam maior "arrependimento" (custo extra) se deixados para depois.
🚀 Como Executar
Pré-requisitos
Compilador C++ (g++, clang ou MSVC) com suporte a C++17.
Python 3.x (para scripts auxiliares).
Biblioteca Python: matplotlib (para gráficos).
1. Compilação (C++)
No terminal, compile o código principal:
g++ -O3 -std=c++17 main.cpp -o alns_cvrp


Nota: A flag -O3 é recomendada para otimização máxima.
2. Execução Simples
Para rodar o algoritmo numa única instância (formato TSPLIB .vrp):
# Linux/Mac
./alns_cvrp Instancias/A-n32-k5.vrp

# Windows
alns_cvrp.exe Instancias\A-n32-k5.vrp


O programa irá gerar um arquivo solution_data.txt com as coordenadas da melhor rota encontrada.
3. Visualização das Rotas
Após executar o código C++, use o script Python para ver o gráfico da solução:
python plot_routes.py


📊 Validação Estatística (TCC)
Como o ALNS é um algoritmo estocástico, os resultados variam a cada execução. Para garantir a confiabilidade dos dados apresentados no trabalho académico, foi implementado um script de validação baseado no cálculo de amostra para população infinita.
Para reproduzir os resultados estatísticos (MDO, Tempo Médio, Intervalo de Confiança):
Abra o arquivo run_statistical_validation.py.
Configure a variável BKS_LITERATURA com o valor ótimo conhecido da instância.
Defina NUM_EXECUCOES com o número $n$ calculado estatisticamente.
Execute:
python run_statistical_validation.py


O script executará o algoritmo $n$ vezes e gerará um relatório completo com o Gap Médio (%) e o Intervalo de Confiança da solução.
🔧 Configuração de Parâmetros
Os hiperparâmetros do algoritmo podem ser ajustados diretamente no início da função main no arquivo main.cpp:
maxIter: Número máximo de iterações (Padrão sugerido: 50.000).
maxTimeSeconds: Tempo limite de execução.
coolingRate: Taxa de resfriamento do Simulated Annealing (Crítico para convergência).
minRemPct / maxRemPct: Porcentagem da solução a ser destruída a cada iteração.
📝 Autores
Este projeto utiliza instâncias de teste padrão da biblioteca TSPLIB.
