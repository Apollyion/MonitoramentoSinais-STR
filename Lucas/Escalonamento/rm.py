import matplotlib.pyplot as plt

def escalonavel(tarefas):
    tarefas.sort(key=lambda x: x['prazo'])  # ordenando as tarefas com base no prazo
    utilizacao_total = sum(tarefa['tempo_execucao'] / tarefa['prazo'] for tarefa in tarefas)
    n = len(tarefas)
    if utilizacao_total <= n * (2 ** (1/n) - 1):
        return True, utilizacao_total, n * (2 ** (1/n) - 1)
    return False, utilizacao_total, n * (2 ** (1/n) - 1)

def desenhar_grafico_escalonabilidade(tarefas, prazo_maximo):
    fig, ax = plt.subplots()
    prazos = [tarefa['prazo'] for tarefa in tarefas]
    utilizacoes = [tarefa['tempo_execucao'] / tarefa['prazo'] for tarefa in tarefas]

    # Adicionando os pontos ao gráfico
    ax.scatter(prazos, utilizacoes, color='b', label='Tarefas', s=100)
    ax.set(xlabel='Prazo', ylabel='Utilização',
           title='Gráfico de Escalonabilidade para Deadline Monotônico')

    # Adicionando linhas para o limite de utilização e o prazo máximo
    ax.axhline(y=1, color='r', linestyle='--', label='Limite de Utilização')
    ax.axvline(x=prazo_maximo, color='g', linestyle='--', label='Prazo Máximo')

    # Adicionando o nome de cada tarefa no gráfico
    for i, tarefa in enumerate(tarefas):
        ax.text(tarefa['prazo'], tarefa['tempo_execucao'] / tarefa['prazo'], tarefa['nome'],
                fontsize=8, ha='right', va='bottom')

    ax.legend()  # Adicionando a legenda
    plt.grid(True)  # Adicionando a grade
    plt.show()  # Mostrando o gráfico


# Tarefas para Core 1
tarefas = [{'tempo_execucao': 5, 'prazo': 20, 'nome': 'Oxi_Car(Max):T2'},
           {'tempo_execucao': 2, 'prazo': 100, 'nome': 'Queda(MCU):T3'},
           {'tempo_execucao': 1, 'prazo': 200, 'nome': 'BPM(Pulse):T4'}]

# # Tarefas para Core 2
# tarefas = [{'tempo_execucao': 4, 'prazo': 500, 'nome': 'EnviarDados'},
#            {'tempo_execucao': 4, 'prazo': 500, 'nome': 'Receber(HandShake)'},]

prazo_maximo = max(tarefa['prazo'] for tarefa in tarefas)
resultado_escalonabilidade, utilizacao_total, limite_utilizacao = escalonavel(tarefas)
print(f"É escalonável: {resultado_escalonabilidade}")
print(f"Utilização Total: {utilizacao_total:.2f}")
print(f"Limite de Utilização: {limite_utilizacao:.2f}")

desenhar_grafico_escalonabilidade(tarefas, prazo_maximo)
