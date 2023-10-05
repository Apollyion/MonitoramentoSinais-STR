import processing.net.*;

Server server;
ArrayList<Client> clients = new ArrayList<Client>();
int port = 12345;

void setup() {
  // Inicializa o servidor na porta especificada
  server = new Server(this, port);
  String serverIP = server.ip(); // Obtém o endereço IP do servidor
  println("Endereço IP do servidor: " + serverIP);
}

void draw() {
  // Verifica se há um novo cliente tentando se conectar
  Client client1 = server.available();
  if (client1 != null) {
    // Adiciona o novo cliente à lista de clientes conectados
    clients.add(client1);
    println("Novo cliente conectado");

    // Envia uma mensagem para o cliente 1 assim que ele se conecta
    client1.write("Bem-vindo, NodeMCU!"); // Substitua esta mensagem pela que deseja enviar

    String data = client1.readString();
    if (data != null) {
      // Processa os dados recebidos do cliente 1
      println("Dados recebidos do cliente 1: " + data);
    }
  }
  
  // Envia dados do cliente 1 para os outros clientes
  for (int i = clients.size() - 1; i >= 0; i--) {
    Client sender = clients.get(i);
    //print(sender.ip());
    String senderData = sender.readString();
    if (senderData != null) {
      // Envia os dados do remetente para todos os outros clientes
      for (Client receiver : clients) {
        if (receiver != sender) {
          println("Receptor Encontrado");
          receiver.write(senderData);
          println("Dados enviados para outro cliente: " + senderData);
        }
      }
    }
  }
  
  // Verifica o status de conexão de cada cliente
  for (int i = clients.size() - 1; i >= 0; i--) {
    Client client = clients.get(i);
    if (!client.active()) {
      println("Cliente desconectado");
      clients.remove(i);
    }
  }
}
