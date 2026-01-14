#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <random>
#include <iomanip>
#include <limits>
#include <chrono>

// ==========================================
// ESTRUTURAS DE DADOS
// ==========================================
struct Node
{
    int id;
    double x;
    double y;
    int demand;
};

struct Instance
{
    int dimension;
    int capacity;
    std::vector<Node> nodes;
    std::vector<std::vector<long long>> distMatrix;
};

struct Route
{
    std::vector<int> path; // Sequência de clientes
    int load;
    long long cost;
};

struct Solution
{
    std::vector<Route> routes;
    long long totalCost;
    std::vector<int> unassigned;
};

struct InsertionMove
{
    int customerNode;
    int routeIndex;
    int position;
    long long costIncrease;
};

// ==========================================
// VARIÁVEIS GLOBAIS E UTILITÁRIOS
// ==========================================

// Seed baseada no tempo para garantir aleatoriedade em cada execução
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

// Distância Euclidiana Arredondada (Padrão TSPLIB e literatura acadêmica)
long long calculateDistance(const Node &n1, const Node &n2)
{
    double dist = std::sqrt(std::pow(n1.x - n2.x, 2) + std::pow(n1.y - n2.y, 2));
    return static_cast<long long>(std::round(dist));
}

// Recalcula custo e carga de uma rota
void updateRoute(Route &r, const Instance &inst)
{
    long long dist = 0;
    int load = 0;

    if (r.path.empty())
    {
        r.cost = 0;
        r.load = 0;
        return;
    }

    // Depósito -> Primeiro
    dist += inst.distMatrix[0][r.path.front()];

    // Caminho
    for (size_t i = 0; i < r.path.size() - 1; ++i)
    {
        dist += inst.distMatrix[r.path[i]][r.path[i + 1]];
        load += inst.nodes[r.path[i]].demand;
    }
    // Último
    load += inst.nodes[r.path.back()].demand;

    // Último -> Depósito
    dist += inst.distMatrix[r.path.back()][0];

    r.load = load;
    r.cost = dist;
}

// Recalcula solução inteira
void updateSolution(Solution &sol, const Instance &inst)
{
    sol.totalCost = 0;
    for (auto &r : sol.routes)
    {
        updateRoute(r, inst);
        sol.totalCost += r.cost;
    }
    // Penalidade M (Big M) para clientes não atendidos
    if (!sol.unassigned.empty())
    {
        sol.totalCost += sol.unassigned.size() * 10000000;
    }
}

// ==========================================
// LEITURA E EXPORTAÇÃO
// ==========================================

Instance loadInstance(const std::string &filepath)
{
    Instance inst;
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir arquivo." << std::endl;
        exit(1);
    }

    std::string line;
    bool cs = false, ds = false;

    while (std::getline(file, line))
    {
        if (line.find("EOF") != std::string::npos)
            break;
        if (line.find("DIMENSION") != std::string::npos)
        {
            inst.dimension = std::stoi(line.substr(line.find(":") + 1));
            inst.nodes.resize(inst.dimension + 1);
        }
        else if (line.find("CAPACITY") != std::string::npos)
        {
            inst.capacity = std::stoi(line.substr(line.find(":") + 1));
        }
        else if (line.find("NODE_COORD_SECTION") != std::string::npos)
        {
            cs = true;
            ds = false;
            continue;
        }
        else if (line.find("DEMAND_SECTION") != std::string::npos)
        {
            cs = false;
            ds = true;
            continue;
        }

        std::stringstream ss(line);
        if (cs)
        {
            int id;
            double x, y;
            if (ss >> id >> x >> y)
            {
                int idx = id - 1;
                if (idx < inst.dimension)
                {
                    inst.nodes[idx].id = idx;
                    inst.nodes[idx].x = x;
                    inst.nodes[idx].y = y;
                }
            }
        }
        else if (ds)
        {
            int id, d;
            if (ss >> id >> d)
            {
                int idx = id - 1;
                if (idx < inst.dimension)
                    inst.nodes[idx].demand = d;
            }
        }
    }

    // Pré-calculo da matriz
    inst.distMatrix.resize(inst.dimension, std::vector<long long>(inst.dimension));
    for (int i = 0; i < inst.dimension; ++i)
        for (int j = 0; j < inst.dimension; ++j)
            inst.distMatrix[i][j] = calculateDistance(inst.nodes[i], inst.nodes[j]);

    return inst;
}

void exportSolution(const Solution &sol, const Instance &inst)
{
    std::ofstream out("solution_data.txt");
    for (size_t r = 0; r < sol.routes.size(); ++r)
    {
        out << r << " " << inst.nodes[0].x << " " << inst.nodes[0].y << "\n";
        for (int c : sol.routes[r].path)
        {
            out << r << " " << inst.nodes[c].x << " " << inst.nodes[c].y << "\n";
        }
        out << r << " " << inst.nodes[0].x << " " << inst.nodes[0].y << "\n";
    }
    out << "-1 " << inst.nodes[0].x << " " << inst.nodes[0].y << "\n"; // Marca depósito
}

// ==========================================
// SOLUÇÃO INICIAL (Nearest Neighbor)
// ==========================================

Solution initialSolution(const Instance &inst)
{
    Solution sol;
    std::vector<bool> visited(inst.dimension, false);
    visited[0] = true;
    int visitedCount = 1;

    while (visitedCount < inst.dimension)
    {
        Route currentRoute;
        int currentNode = 0;
        int currentLoad = 0;
        bool routeFinished = false;

        while (!routeFinished)
        {
            int bestNode = -1;
            long long bestDist = std::numeric_limits<long long>::max();

            for (int i = 1; i < inst.dimension; ++i)
            {
                if (!visited[i])
                {
                    if (currentLoad + inst.nodes[i].demand <= inst.capacity)
                    {
                        long long d = inst.distMatrix[currentNode][i];
                        if (d < bestDist)
                        {
                            bestDist = d;
                            bestNode = i;
                        }
                    }
                }
            }

            if (bestNode != -1)
            {
                currentRoute.path.push_back(bestNode);
                visited[bestNode] = true;
                currentLoad += inst.nodes[bestNode].demand;
                currentNode = bestNode;
                visitedCount++;
            }
            else
            {
                routeFinished = true;
            }
        }

        updateRoute(currentRoute, inst);
        sol.routes.push_back(currentRoute);
    }
    updateSolution(sol, inst);
    return sol;
}

// ==========================================
// OPERADORES DE DESTRUIÇÃO (REMOVAL)
// ==========================================

// 1. Random Removal
void destroyRandom(Solution &sol, int q, const Instance &inst)
{
    for (int k = 0; k < q; ++k)
    {
        if (sol.routes.empty())
            break;

        // Seleciona rotas não vazias
        std::vector<int> nonEmpty;
        for (size_t i = 0; i < sol.routes.size(); ++i)
            if (!sol.routes[i].path.empty())
                nonEmpty.push_back(i);
        if (nonEmpty.empty())
            break;

        int rIdx = nonEmpty[std::uniform_int_distribution<>(0, (int)nonEmpty.size() - 1)(rng)];
        int nodePos = std::uniform_int_distribution<>(0, (int)sol.routes[rIdx].path.size() - 1)(rng);

        sol.unassigned.push_back(sol.routes[rIdx].path[nodePos]);
        sol.routes[rIdx].path.erase(sol.routes[rIdx].path.begin() + nodePos);

        if (sol.routes[rIdx].path.empty())
            sol.routes.erase(sol.routes.begin() + rIdx);
    }
    updateSolution(sol, inst);
}

// 2. Worst Removal (Remove quem gera maior custo marginal)
void destroyWorst(Solution &sol, int q, const Instance &inst)
{
    struct CostItem
    {
        int rIdx;
        int nPos;
        long long savings;
    };
    std::vector<CostItem> costs;

    for (size_t r = 0; r < sol.routes.size(); ++r)
    {
        if (sol.routes[r].path.empty())
            continue;
        for (size_t p = 0; p < sol.routes[r].path.size(); ++p)
        {
            int c = sol.routes[r].path[p];
            int prev = (p == 0) ? 0 : sol.routes[r].path[p - 1];
            int next = (p == sol.routes[r].path.size() - 1) ? 0 : sol.routes[r].path[p + 1];

            // Economia = Custo com ele - Custo sem ele
            long long currentArc = inst.distMatrix[prev][c] + inst.distMatrix[c][next];
            long long newArc = inst.distMatrix[prev][next];
            costs.push_back({(int)r, (int)p, currentArc - newArc});
        }
    }

    // Ordena decrescente (Maior economia primeiro)
    // Randomização no sort para não ser 100% determinístico (Feature comum do ALNS)
    std::sort(costs.begin(), costs.end(), [](const auto &a, const auto &b)
              { return a.savings > b.savings; });

    // Introduz aleatoriedade na seleção do "Pior" (p param = 3 ou 4)
    std::vector<int> toRemove;
    for (int k = 0; k < q && !costs.empty(); ++k)
    {
        // Pega um dos top 20% piores ou determinístico
        int idx = 0; // Pegando o pior absoluto
        toRemove.push_back(sol.routes[costs[idx].rIdx].path[costs[idx].nPos]);
        costs.erase(costs.begin() + idx);
    }

    // Remove IDs (mais seguro que índices diretos)
    for (int id : toRemove)
    {
        bool removed = false;
        for (size_t r = 0; r < sol.routes.size(); ++r)
        {
            auto it = std::find(sol.routes[r].path.begin(), sol.routes[r].path.end(), id);
            if (it != sol.routes[r].path.end())
            {
                sol.routes[r].path.erase(it);
                sol.unassigned.push_back(id);
                if (sol.routes[r].path.empty())
                {
                    sol.routes.erase(sol.routes.begin() + r);
                    r--;
                }
                removed = true;
                break;
            }
        }
    }
    updateSolution(sol, inst);
}

// 3. Shaw Removal (Relatedness Removal)
void destroyShaw(Solution &sol, int q, const Instance &inst)
{
    if (sol.routes.empty())
        return;

    // Lista todos clientes
    std::vector<int> allCustomers;
    for (const auto &r : sol.routes)
        for (int c : r.path)
            allCustomers.push_back(c);
    if (allCustomers.empty())
        return;

    // Semente aleatória
    int seed = allCustomers[std::uniform_int_distribution<>(0, (int)allCustomers.size() - 1)(rng)];
    std::vector<int> removed = {seed};

    // Remove semente
    for (size_t r = 0; r < sol.routes.size(); ++r)
    {
        auto it = std::find(sol.routes[r].path.begin(), sol.routes[r].path.end(), seed);
        if (it != sol.routes[r].path.end())
        {
            sol.routes[r].path.erase(it);
            if (sol.routes[r].path.empty())
                sol.routes.erase(sol.routes.begin() + r);
            break;
        }
    }
    sol.unassigned.push_back(seed);

    // Remove os (q-1) mais relacionados à semente ou aos já removidos
    while ((int)removed.size() < q && !sol.routes.empty())
    {
        int rVal = removed[std::uniform_int_distribution<>(0, (int)removed.size() - 1)(rng)];

        int bestCand = -1;
        long long minRel = std::numeric_limits<long long>::max();

        for (const auto &r : sol.routes)
        {
            for (int c : r.path)
            {
                // Relatedness (Similiaridade): Distância
                long long rel = inst.distMatrix[rVal][c];
                if (rel < minRel)
                {
                    minRel = rel;
                    bestCand = c;
                }
            }
        }

        if (bestCand != -1)
        {
            removed.push_back(bestCand);
            sol.unassigned.push_back(bestCand);
            for (size_t r = 0; r < sol.routes.size(); ++r)
            {
                auto it = std::find(sol.routes[r].path.begin(), sol.routes[r].path.end(), bestCand);
                if (it != sol.routes[r].path.end())
                {
                    sol.routes[r].path.erase(it);
                    if (sol.routes[r].path.empty())
                        sol.routes.erase(sol.routes.begin() + r);
                    break;
                }
            }
        }
        else
            break;
    }
    updateSolution(sol, inst);
}

// ==========================================
// OPERADORES DE REPARO (INSERTION)
// ==========================================

// Auxiliar: Encontra melhor posição para UM cliente
InsertionMove findBestPosition(int cust, int rIdx, const Solution &sol, const Instance &inst)
{
    InsertionMove bestMove = {cust, rIdx, -1, std::numeric_limits<long long>::max()};
    const Route &r = sol.routes[rIdx];

    if (r.load + inst.nodes[cust].demand > inst.capacity)
        return bestMove;

    for (size_t p = 0; p <= r.path.size(); ++p)
    {
        int prev = (p == 0) ? 0 : r.path[p - 1];
        int next = (p == r.path.size()) ? 0 : r.path[p];

        long long costInc = (inst.distMatrix[prev][cust] + inst.distMatrix[cust][next]) - inst.distMatrix[prev][next];

        if (costInc < bestMove.costIncrease)
        {
            bestMove.costIncrease = costInc;
            bestMove.position = p;
        }
    }
    return bestMove;
}

// 1. Greedy Insertion
void repairGreedy(Solution &sol, const Instance &inst)
{
    // Embaralha para evitar viés de ordem
    std::shuffle(sol.unassigned.begin(), sol.unassigned.end(), rng);

    while (!sol.unassigned.empty())
    {
        int cust = sol.unassigned.back();
        sol.unassigned.pop_back();

        InsertionMove globalBest = {cust, -1, -1, std::numeric_limits<long long>::max()};

        // Tenta em todas as rotas existentes
        for (size_t r = 0; r < sol.routes.size(); ++r)
        {
            InsertionMove m = findBestPosition(cust, r, sol, inst);
            if (m.position != -1 && m.costIncrease < globalBest.costIncrease)
                globalBest = m;
        }

        // Tenta criar nova rota
        long long newRouteCost = inst.distMatrix[0][cust] + inst.distMatrix[cust][0];
        if (newRouteCost < globalBest.costIncrease)
        {
            globalBest = {cust, (int)sol.routes.size(), 0, newRouteCost};
        }

        // Aplica inserção
        if (globalBest.routeIndex == (int)sol.routes.size())
        {
            Route newR;
            newR.path.push_back(cust);
            updateRoute(newR, inst);
            sol.routes.push_back(newR);
        }
        else
        {
            sol.routes[globalBest.routeIndex].path.insert(
                sol.routes[globalBest.routeIndex].path.begin() + globalBest.position, cust);
            updateRoute(sol.routes[globalBest.routeIndex], inst);
        }
    }
    updateSolution(sol, inst);
}

// 2. Regret-2 Insertion
void repairRegret(Solution &sol, const Instance &inst)
{
    while (!sol.unassigned.empty())
    {
        int bestCandIdx = -1;
        long long maxRegret = -1;
        InsertionMove bestMove;

        // Para cada cliente não alocado
        for (size_t i = 0; i < sol.unassigned.size(); ++i)
        {
            int cust = sol.unassigned[i];
            std::vector<InsertionMove> moves;

            // Coleta melhor inserção em CADA rota possível
            for (size_t r = 0; r < sol.routes.size(); ++r)
            {
                InsertionMove m = findBestPosition(cust, r, sol, inst);
                if (m.position != -1)
                    moves.push_back(m);
            }
            // Opção nova rota
            moves.push_back({cust, (int)sol.routes.size(), 0, inst.distMatrix[0][cust] + inst.distMatrix[cust][0]});

            // Ordena os custos (menor para maior)
            std::sort(moves.begin(), moves.end(), [](const auto &a, const auto &b)
                      { return a.costIncrease < b.costIncrease; });

            // Regret = (Custo 2ª Opção) - (Custo Melhor Opção)
            long long regret = 0;
            if (moves.size() >= 2)
                regret = moves[1].costIncrease - moves[0].costIncrease;
            else
                regret = moves[0].costIncrease; // Prioridade máxima se só tem uma opção

            if (regret > maxRegret)
            {
                maxRegret = regret;
                bestCandIdx = i;
                bestMove = moves[0];
            }
        }

        // Insere o cliente com maior regret na sua melhor posição
        if (bestCandIdx != -1)
        {
            sol.unassigned.erase(sol.unassigned.begin() + bestCandIdx);

            if (bestMove.routeIndex == (int)sol.routes.size())
            {
                Route newR;
                newR.path.push_back(bestMove.customerNode);
                updateRoute(newR, inst);
                sol.routes.push_back(newR);
            }
            else
            {
                sol.routes[bestMove.routeIndex].path.insert(
                    sol.routes[bestMove.routeIndex].path.begin() + bestMove.position, bestMove.customerNode);
                updateRoute(sol.routes[bestMove.routeIndex], inst);
            }
        }
        else
            break; // Fallback (não deveria ocorrer)
    }
    updateSolution(sol, inst);
}

// ==========================================
// SELETOR DE OPERADORES (ROULETTE WHEEL)
// ==========================================

int selectOperator(const std::vector<double> &weights)
{
    double total = 0;
    for (double w : weights)
        total += w;
    std::uniform_real_distribution<> dis(0, total);
    double val = dis(rng);
    double acc = 0;
    for (size_t i = 0; i < weights.size(); ++i)
    {
        acc += weights[i];
        if (val <= acc)
            return i;
    }
    return weights.size() - 1;
}

// ==========================================
// MAIN (ALNS PURO)
// ==========================================

int main(int argc, char **argv)
{
    auto startTotal = std::chrono::steady_clock::now();
    if (argc < 2)
    {
        std::cout << "Uso: ./alns_cvrp <instancia.vrp>" << std::endl;
        return 1;
    }

    Instance inst = loadInstance(argv[1]);
    std::cout << "Instancia: " << inst.dimension << " nodes | Cap: " << inst.capacity << std::endl;

    // ------------------------------------------------------------------------
    // PARÂMETROS DE AJUSTE (ALNS PURO)
    // ------------------------------------------------------------------------
    // Para ALNS puro funcionar bem, precisamos de MUITAS iterações, pois ele
    // não tem busca local para limpar a "sujeira" deixada pela inserção gulosa.
    int maxIter = 50000;      // Aumentado para explorar bem (50k)
    int maxTimeSeconds = 120; // Tempo para garantir que não fique rodando eternamente

    // SA Parameters
    double startTempFactor = 0.05; // T0 = 5% do custo inicial
    double coolingRate = 0.9997;   // Resfriamento lento para permitir exploração

    // ALNS Parameters
    double minRemPct = 0.10; // Remover min 10%
    double maxRemPct = 0.40; // Remover max 40% (Range amplo para sair de ótimo local)

    // Pesos adaptativos
    double sigma1 = 33; // Novo Global
    double sigma2 = 9;  // Novo Melhor que Atual
    double sigma3 = 13; // Pior Aceito
    double reactionFactor = 0.1;
    int segmentSize = 100;
    // ------------------------------------------------------------------------

    // Inicialização
    Solution currentSol = initialSolution(inst);
    Solution bestSol = currentSol;
    double T = currentSol.totalCost * startTempFactor;

    // Inicializa Pesos dos Operadores (Iguais no início)
    // Destroy: 0=Random, 1=Worst, 2=Shaw
    std::vector<double> dWeights = {1.0, 1.0, 1.0};
    std::vector<double> dScores(3, 0.0);
    std::vector<int> dCounts(3, 0);

    // Repair: 0=Greedy, 1=Regret
    std::vector<double> rWeights = {1.0, 1.0};
    std::vector<double> rScores(2, 0.0);
    std::vector<int> rCounts(2, 0);

    std::cout << "Solucao Inicial (NN): " << currentSol.totalCost << std::endl;

    // Loop Principal
    for (int iter = 0; iter < maxIter; ++iter)
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - startTotal).count() > maxTimeSeconds)
        {
            std::cout << "Tempo limite (" << maxTimeSeconds << "s) atingido." << std::endl;
            break;
        }

        Solution tempSol = currentSol;

        // 1. Escolhe Operadores
        int dOp = selectOperator(dWeights);
        int rOp = selectOperator(rWeights);
        dCounts[dOp]++;
        rCounts[rOp]++;

        // 2. Define tamanho da vizinhança (q)
        int minQ = std::max(1, (int)(inst.dimension * minRemPct));
        int maxQ = std::max(2, (int)(inst.dimension * maxRemPct));
        int q = std::uniform_int_distribution<>(minQ, maxQ)(rng);

        // 3. Destroy
        if (dOp == 0)
            destroyRandom(tempSol, q, inst);
        else if (dOp == 1)
            destroyWorst(tempSol, q, inst);
        else
            destroyShaw(tempSol, q, inst);

        // 4. Repair
        if (rOp == 0)
            repairGreedy(tempSol, inst);
        else
            repairRegret(tempSol, inst);

        // 5. Aceitação (SA) e Pontuação
        double score = 0;
        if (tempSol.unassigned.empty())
        { // Verifica validade básica
            if (tempSol.totalCost < bestSol.totalCost)
            {
                bestSol = tempSol;
                currentSol = tempSol;
                score = sigma1;
                std::cout << "Iter " << iter << " | Novo Best: " << bestSol.totalCost << " [" << (dOp == 0 ? "Rnd" : (dOp == 1 ? "Wst" : "Shw")) << "+" << (rOp == 0 ? "Grd" : "Reg") << "]" << std::endl;
            }
            else if (tempSol.totalCost < currentSol.totalCost)
            {
                currentSol = tempSol;
                score = sigma2;
            }
            else
            {
                double delta = (double)(tempSol.totalCost - currentSol.totalCost);
                if (std::uniform_real_distribution<>(0.0, 1.0)(rng) < std::exp(-delta / T))
                {
                    currentSol = tempSol;
                    score = sigma3;
                }
            }
        }

        dScores[dOp] += score;
        rScores[rOp] += score;

        // 6. Atualização Adaptativa de Pesos
        if (iter % segmentSize == 0)
        {
            for (int i = 0; i < 3; ++i)
                if (dCounts[i] > 0)
                {
                    dWeights[i] = (1 - reactionFactor) * dWeights[i] + reactionFactor * (dScores[i] / dCounts[i]);
                    dScores[i] = 0;
                    dCounts[i] = 0;
                }
            for (int i = 0; i < 2; ++i)
                if (rCounts[i] > 0)
                {
                    rWeights[i] = (1 - reactionFactor) * rWeights[i] + reactionFactor * (rScores[i] / rCounts[i]);
                    rScores[i] = 0;
                    rCounts[i] = 0;
                }
        }

        // Resfriamento
        T *= coolingRate;
        if (T < 0.001)
            T = currentSol.totalCost * 0.001; // Reheat suave para evitar congelamento total
    }

    auto endTotal = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = endTotal - startTotal;

    std::cout << "\n===============================" << std::endl;
    std::cout << "Melhor Custo: " << bestSol.totalCost << std::endl;
    std::cout << "Tempo: " << elapsed.count() << "s" << std::endl;
    std::cout << "===============================" << std::endl;

    exportSolution(bestSol, inst);

    return 0;
}