import processing.net.*;

Server server;
int port = 12345;

void setup() {
  server = new Server(this, port);
  String serverIP = server.ip(); // Obtém o endereço IP do servidor
  println("Endereço IP do servidor: " + serverIP);

  // Envie o endereço IP para o ESP32 e o NodeMCU, se necessário
  // ESP32 e NodeMCU devem estar esperando por isso
}

void draw() {
  Client client = server.available();
  if (client != null) {
    String data = client.readString();
    if (data != null) {
      // Processar os dados recebidos
      println("Dados recebidos do ESP32: " + data);
      
      // Enviar os dados para o NodeMCU (se necessário)
      // client.write("Dados para o NodeMCU");
    }
    client.stop();
  }
}
