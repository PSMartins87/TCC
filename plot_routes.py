import matplotlib.pyplot as plt
import os

def plot_vrp_solution(filename="solution_data.txt"):
    if not os.path.exists(filename):
        print(f"Erro: O arquivo '{filename}' nao foi encontrado.")
        print("Execute o programa C++ primeiro para gerar os dados.")
        return

    routes = {}
    depot_x = None
    depot_y = None

    # Leitura do arquivo (formato: RouteID X Y)
    with open(filename, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if len(parts) < 3:
                continue
            
            route_id = int(parts[0])
            x = float(parts[1])
            y = float(parts[2])

            if route_id == -1:
                depot_x = x
                depot_y = y
            else:
                if route_id not in routes:
                    routes[route_id] = {'x': [], 'y': []}
                routes[route_id]['x'].append(x)
                routes[route_id]['y'].append(y)

    # Configuração da plotagem
    plt.figure(figsize=(10, 8))
    plt.title("Solução ALNS CVRP (Inteiros)", fontsize=16)
    plt.xlabel("X")
    plt.ylabel("Y")

    # Cores
    colors = plt.cm.get_cmap('tab10', len(routes))

    # Desenha rotas
    for r_id, coords in routes.items():
        plt.plot(coords['x'], coords['y'], 
                 marker='o', linestyle='-', linewidth=2, markersize=5, 
                 label=f'Rota {r_id+1}', color=colors(r_id))

    # Desenha depósito
    if depot_x is not None:
        plt.scatter(depot_x, depot_y, c='black', marker='s', s=100, label='Depósito', zorder=10)

    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    plot_vrp_solution()