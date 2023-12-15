# MonitoramentoSinais-STR
## Plataforma de Monitoramento de Sinais para Pessoas em Lares de Idosos

Interface final:
![Interface_Geral_Administrador](https://github.com/Apollyion/MonitoramentoSinais-STR/assets/58216614/82db0780-fbe8-4b2f-a4bb-8f131078d1cd)

Arte conceitual:
![arteconceitual](https://github.com/Apollyion/MonitoramentoSinais-STR/assets/58216614/1f6bb871-0e06-4113-992a-46624bea63ea)

Este repositório contém o código-fonte e documentação da "Plataforma de Monitoramento de Sinais para Pessoas em Lares de Idosos". Este projeto representa um avanço notável na integração de tecnologia para melhorar o cuidado e a segurança dos idosos.
Destaques do Projeto

- Monitoramento Contínuo e em Tempo Real: Através de uma combinação cuidadosa de hardware e software, a plataforma oferece monitoramento contínuo e em tempo real dos sinais vitais dos idosos.

- Algoritmo de Escalonamento Deadline Monotonic (DM): Utiliza um algoritmo de escalonamento de prioridade fixa, proporcionando previsibilidade e eficiência na execução de tarefas cruciais.

- Luva Inteligente: Equipada com sensores precisos e recursos de detecção de quedas, enfatizando não apenas a precisão técnica, mas também o conforto e a usabilidade para os usuários.

- Otimização Eficiente dos Recursos: Implementação estratégica de dois núcleos no ESP-32, dedicando um para o processamento de dados vitais e outro para a gestão de conexões Wi-Fi, garantindo desempenho estável e confiabilidade em situações críticas.
  
- Ferramentas Utilizadas: Desenvolvido em C++ e utilizando a plataforma Processing para representação visual dos dados, cores dinâmicas e simulação de dados realistas para uma experiência de usuário envolvente e informativa.

## Lógica de Software

A seção de lógica de software neste repositório descreve a implementação crítica na linguagem Processing, fundamentada em classes, funções e métodos essenciais para o funcionamento e a exibição da plataforma de monitoramento. Destacamos as seguintes características-chave:
Classes e Funcionalidades
Classe Server

    Comunicação Fluida com Dispositivos Externos: A classe Server assegura uma comunicação eficiente com dispositivos externos, como a ESP-32, integrando dados vitais para garantir uma coleta contínua de informações para o monitoramento dos idosos.

Classe User

    Método Status: Eleva a comunicação visual ao traduzir dados complexos em representações gráficas envolventes. Utiliza códigos de cores e elementos visuais para oferecer uma visão instantânea e compreensível do estado de saúde do idoso, incluindo alertas visuais para bradicardia e taquicardia.

    Método checkHover: Aprimora a interatividade ao detectar a posição do cursor, proporcionando respostas visuais dinâmicas. Destaca áreas específicas quando o mouse está sobre um usuário, contribuindo para uma experiência do usuário mais intuitiva.

    Método Display: Destacando-se no aspecto visual, o método Display oferece uma representação gráfica imersiva dos sinais vitais. Traduz batimentos cardíacos, níveis de SPO2, detecção de quedas e o estado do botão de pânico em elementos visuais, proporcionando uma visão compreensível e cativante do estado de saúde de cada idoso.
