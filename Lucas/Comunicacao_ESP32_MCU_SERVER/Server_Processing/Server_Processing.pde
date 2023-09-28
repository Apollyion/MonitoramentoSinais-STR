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
  Client client1 = server.available();
  Client client2 = null;
  if (client1 != null) {
    String data = client1.readString();
    if (data != null) {
      // Process the received data
      println("Data received from client 1: " + data);
      
      // Find another client to send the data to
      for (Client c : server.clients()) {
        if (c != client1) {
          client2 = c;
          break;
        }
      }
      
      // Send the data to the other client
      if (client2 != null) {
        client2.write(data);
        println("Data sent to client 2: " + data);
      }
    }
    client1.stop();
  }
}
