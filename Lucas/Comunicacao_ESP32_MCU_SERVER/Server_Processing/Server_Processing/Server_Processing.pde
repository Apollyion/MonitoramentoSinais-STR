import processing.net.*;

Server server;
int port = 12345;
Client esp32Client = null;
Client nodemcuClient = null;

void setup() {
  server = new Server(this, port);
  String serverIP = server.ip(); // Obtém o endereço IP do servidor
  println("Endereço IP do servidor: " + serverIP);
}

void draw() {
  if (esp32Client == null || !esp32Client.active()) {
    esp32Client = null;
  }
  if (nodemcuClient == null || !nodemcuClient.active()) {
    nodemcuClient = null;
  }

  if (esp32Client == null || nodemcuClient == null) {
    Client client = server.available();
    if (client != null) {
      String data = client.readString();
      if (data != null) {
        println("Data received from client: " + data);

        if (data.equals("esp32") && esp32Client == null) {
          esp32Client = client;
          println("Client set as esp32");
        } else if (data.equals("nodemcu") && nodemcuClient == null) {
          nodemcuClient = client;
          println("Client set as nodemcu");
        }
      }
    }
  } else {
    // Check for data from esp32 and send to nodemcu
    if (esp32Client.available() > 0) {
      String data = esp32Client.readString();
      if (data != null) {
        println("Data received from esp32: " + data);
        nodemcuClient.write(data);
        println("Data sent to nodemcu: " + data + "\n");
      }
    }
  }
}
