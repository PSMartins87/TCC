import subprocess
import os
import re
import math
import statistics

# ==============================================================================
# CONFIGURAÇÕES DO EXPERIMENTO 
# ==============================================================================

# Defina aqui o executável e a instância
EXECUTAVEL = "./alns_cvrp" if os.name != 'nt' else "alns_cvrp.exe"
ARQUIVO_INSTANCIA = "F-n135-k7.vrp" # Exemplo: substitua pela instância que está testando

# Valor da Melhor Solução Conhecida (BKS) da literatura para esta instância
# (Necessário para calcular o MDO/Gap). Ex: A-n32-k5 o BKS é 784.
BKS_LITERATURA = 1162

# Número de execuções calculado pela sua fórmula de população infinita
# Ex: Se seu cálculo pediu 383 testes para garantir a confiança, coloque 383.
NUM_EXECUCOES = 30  # <--- ALTERE ESTE VALOR CONFORME SEU CALCULO

# Nível de Confiança para o cálculo do intervalo (usando Z-score aproximado)
# 1.96 para 95%, 2.576 para 99%
Z_SCORE = 1.96 

# ==============================================================================

def rodar_algoritmo(idx):
    """Executa o código C++ uma vez e retorna (Custo, Tempo)."""
    try:
        resultado = subprocess.run(
            [EXECUTAVEL, ARQUIVO_INSTANCIA], 
            capture_output=True, 
            text=True
        )
        saida = resultado.stdout
        
        # Extrai Custo
        match_custo = re.search(r"Melhor Custo.*:\s*(\d+)", saida)
        custo = int(match_custo.group(1)) if match_custo else None
        
        # Extrai Tempo (para calcular MT - Mean Time)
        match_tempo = re.search(r"Tempo.*:\s*([\d\.]+)", saida)
        tempo = float(match_tempo.group(1)) if match_tempo else None
        
        return custo, tempo
    except Exception as e:
        print(f"Erro na execução {idx}: {e}")
        return None, None

def main():
    if not os.path.exists(ARQUIVO_INSTANCIA):
        print(f"ERRO: Instância '{ARQUIVO_INSTANCIA}' não encontrada.")
        return

    # Compilação rápida para garantir (opcional)
    if not os.path.exists(EXECUTAVEL):
        print("Compilando...")
        subprocess.run(["g++", "-O3", "-std=c++17", "main.cpp", "-o", "alns_cvrp"])

    print(f"--- INICIANDO VALIDAÇÃO ESTATÍSTICA ---")
    print(f"Instância: {ARQUIVO_INSTANCIA}")
    print(f"BKS (Literatura): {BKS_LITERATURA}")
    print(f"Tamanho da Amostra (n): {NUM_EXECUCOES}")
    print("-" * 50)

    custos = []
    tempos = []

    for i in range(NUM_EXECUCOES):
        c, t = rodar_algoritmo(i)
        if c is not None:
            custos.append(c)
            tempos.append(t)
            # Feedback visual simples (progresso)
            gap_inst = ((c - BKS_LITERATURA) / BKS_LITERATURA) * 100
            print(f"Teste {i+1}/{NUM_EXECUCOES} | Custo: {c} | Gap: {gap_inst:.2f}% | Tempo: {t:.4f}s")

    if not custos:
        print("Nenhum resultado coletado.")
        return

    # ==========================================
    # CÁLCULOS ESTATÍSTICOS (Tabela 6 do TCC)
    # ==========================================
    
    n = len(custos)
    media_custo = statistics.mean(custos)
    desvio_padrao = statistics.stdev(custos) if n > 1 else 0
    media_tempo = statistics.mean(tempos) # MT(s)

    # Cálculo do MDO (Mean Deviation from Optimum / Gap Médio)
    # Fórmula: ((Média Obtida - Melhor Conhecido) / Melhor Conhecido) * 100
    mdo_percentual = ((media_custo - BKS_LITERATURA) / BKS_LITERATURA) * 100

    # Cálculo do Erro Amostral / Intervalo de Confiança (Conf %)
    # Fórmula IC: Z * (DesvioPadrão / sqrt(n))
    erro_padrao = desvio_padrao / math.sqrt(n)
    margem_erro = Z_SCORE * erro_padrao
    
    # Intervalo de Confiança Absoluto
    ic_inferior = media_custo - margem_erro
    ic_superior = media_custo + margem_erro

    # Intervalo de Confiança em Porcentagem (relativo à média) - Opcional, se sua tabela usar %
    ic_percentual = (margem_erro / media_custo) * 100

    print("\n" + "="*50)
    print("RESULTADOS FINAIS")
    print("="*50)
    print(f"Instância:             {ARQUIVO_INSTANCIA}")
    print(f"Amostras (n):          {n}")
    print(f"Melhor Custo Obtido:   {min(custos)}")
    print(f"Pior Custo Obtido:     {max(custos)}")
    print("-" * 30)
    print(f"Média dos Custos:      {media_custo:.2f}")
    print(f"Desvio Padrão:         {desvio_padrao:.2f}")
    print("-" * 30)
    print(f"MT(s) (Tempo Médio):   {media_tempo:.4f} s")
    print(f"MDO (%) (Gap Médio):   {mdo_percentual:.2f} %")
    print(f"IC (95%):              {media_custo:.2f} +/- {margem_erro:.2f}")
    print(f"IC Limites:            [{ic_inferior:.2f}, {ic_superior:.2f}]")
    print("="*50)

    # Salvar em arquivo CSV para facilitar copiar pro Excel/LaTeX
    with open("resultados_estatisticos.csv", "a") as f:
        # Formato: Instancia, n, BKS, Melhor, Média, MT(s), MDO(%), MargemErro
        f.write(f"{ARQUIVO_INSTANCIA},{n},{BKS_LITERATURA},{min(custos)},{media_custo:.2f},{media_tempo:.4f},{mdo_percentual:.2f},{margem_erro:.2f}\n")
    print("Resumo adicionado ao arquivo 'resultados_estatisticos.csv'")

if __name__ == "__main__":
    main()