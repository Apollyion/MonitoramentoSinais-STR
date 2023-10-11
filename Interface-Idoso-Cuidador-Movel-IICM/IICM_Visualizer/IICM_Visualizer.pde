import processing.serial.*;
import processing.net.*;
import controlP5.*;
import java.net.*;
import java.io.*;

ControlP5 cp5;               // Declare a ControlP5 object for creating GUI elements
Textfield ipAddressInput;    // Declare a Textfield for entering an IP address
Textfield portInput;         // Declare a Textfield for entering a port number
Button connectButton;        // Declare a Button for triggering a connection
Socket mainSocket;

void setup() {
  size(1400, 800);            // Set the size of the Processing window
  PFont font1 = createFont("Courier New", 36);  // Create a font object

  cp5 = new ControlP5(this);  // Initialize the ControlP5 library

  background(0);             // Set the background color of the window to black

  // Create a Textfield for entering an IP address
  ipAddressInput = cp5.addTextfield("IP Address")
    .setPosition(500, 400)   // Set the position of the Textfield
    .setSize(400, 50)        // Set the size of the Textfield
    .setFont(font1)          // Set the font for the Textfield
    .setLabelVisible(false); // Hide the label for the Textfield

  // Create a Textfield for entering a port number
  portInput = cp5.addTextfield("Port")
    .setPosition(500, 500)   // Set the position of the Textfield
    .setSize(400, 50)        // Set the size of the Textfield
    .setFont(font1)          // Set the font for the Textfield
    .setLabelVisible(false); // Hide the label for the Textfield

  // Create a Button for connecting to the server
  connectButton = cp5.addButton("Conectar (Listen)")
    .setPosition(500, 600)   // Set the position of the Button
    .setSize(400, 100)       // Set the size of the Button
    .setFont(font1);         // Set the font for the Button

  // Set colors for the Button
  connectButton.setColorBackground(color(100));  // Set the background color of the Button
  connectButton.setColorForeground(color(150));  // Set the color when the Button is hovered
  connectButton.setColorActive(color(150));      // Set the color when the Button is clicked

  textSize(40);              // Set the text size
  text("Interface Idoso-Cuidador-Móvel (IICM)", 400, 50); // Display a text message
  textSize(38);              // Set the text size
  text("Conectar com o servidor", 500, 150);  // Display another text message
}

void draw() {
  fill(255);                 // Set the text color to white
  textSize(24);              // Set the text size
  textAlign(LEFT);           // Set the text alignment to left

  // Display a debug message
  text("(DEBUG) Abaixo será mostrado o que foi digitado: ", 200, 220);

  // Display the entered IP address
  text("IP Address: " + ipAddressInput.getText(), 200, 270);

  // Display the entered port number
  text("Port: " + portInput.getText(), 200, 320);
  
  if (mainSocket != null && mainSocket.isConnected()) { // If connected to the server
    fill(0, 255, 0); // Set the fill color to green (RGB: 0, 255, 0)
    textSize(24);              // Set the text size
    textAlign(RIGHT);           // Set the text alignment to left
    text("CONEXÃO BEM SUCEDIDA!", 1000, 320);
    
    fill(255);                 // Set the text color to white
    textSize(24);              // Set the text size
    textAlign(LEFT);
    try {
      BufferedReader input = new BufferedReader(new InputStreamReader(mainSocket.getInputStream()));
      if (input.ready()) {
        String serverData = input.readLine();
        if (serverData != null) {
          println("Recebido do servidor: " + serverData);
        }
      }
      
      // Enviar dados para o servidor
      int n = 100;
      long sum = 0;
      PrintWriter output = new PrintWriter(mainSocket.getOutputStream(), true);
      for (int i = 0; i < n; i++) {
        long t0 = millis();
        output.println("Ola servidor!");
        long t1 = millis();
        sum += t1 - t0;
      }
      println("Tempo médio de envio: " + sum / float(n));
    } 
    catch (IOException e) {
      println("Erro ao comunicar com o servidor: " + e.getMessage());
    }
  }
}

void controlEvent(ControlEvent event) {
  
  if (event.isController()) {
    if (event.getController() == connectButton) {  // Check if the Connect button is clicked
      String ipAddress = ipAddressInput.getText();  // Get the text from the IP Address Textfield
      String port = portInput.getText();            // Get the text from the Port Textfield

      // Print the text from the text boxes to the Processing terminal
      println("IP Address: " + ipAddress);
      println("Port: " + port);
      
      String ipAddrInput = ipAddressInput.getText();
      String portNumInput = portInput.getText();
      
      if(ipAddrInput==""){
        println("O campo do endereço IP está em branco. Digite um IP.");
      }
      
      if(portNumInput==""){
        println("O campo do número da porta está em branco. Digite um número de porta.");
      }
      
      mainSocket = connectToESP32(ipAddrInput, portNumInput);
      
      if(mainSocket == null){
        println("Digite novamente o IP e o número da porta.");
      }
    }
  }
}

Socket connectToESP32(String ipAddress, String port){
  int serverPort; // Porta do servidor as an integer
  serverPort = Integer.parseInt(port); // Convert the port to an integer
  try {
    Socket socket; // Cliente WiFi
    InetAddress serverAddr = InetAddress.getByName(ipAddress);
    socket = new Socket(serverAddr, serverPort); // Connect to the server
    return socket;
  } 
  catch (IOException e) {
    println("Erro ao conectar ao servidor: " + e.getMessage());
    return null; // Return null in case of an error
  }
}
