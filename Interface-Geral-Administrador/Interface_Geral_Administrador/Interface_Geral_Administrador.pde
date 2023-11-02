import processing.serial.*;
import processing.net.*;
import controlP5.*;
import java.net.*;
import java.io.*;
import processing.core.PApplet;
import g4p_controls.*;
//Bibliotecas de instalação obrigatória: ControlP5 e G4P
ControlP5 cp5;
Button connectButton;
Server server;
PApplet parent; // Variável para acessar a classe principal PApplet


int viewportY = 0;
int rowHeight = 150;
int numRows = 10;
int maxVisibleRows;
long lastStatusChangeTime = 0;
int statusChangeDuration = 5000; // 5000 milissegundos (5 segundos)
boolean isStatusChanging = false;
String randomData = "";

String generateRandomData() {
  int bpm = (int) random(30, 150); // BPM range from 30 to 150 (normal range is typically 60-100)
  float spo2 = random(75.0, 100.0); // SPO2 range from 75% to 100% (normal range is typically 95-100)
  int fall = random(2) < 1 ? 0 : 1; // Randomly generate 0 or 1 for fall
  int panicButton = random(2) < 0.1 ? 1 : 0; // Randomly generate 1 with a 10% chance for panic_button
  
  String data = bpm + " " + spo2 + " " + fall + " " + panicButton;
  return data;
}



int dataGenerationInterval = 5000; // Generate data every 5 seconds (adjust as needed)
int globalLastDataGenerationTime = 0;
int globalCurrentTime;
int globalElapsedTime = globalCurrentTime - globalLastDataGenerationTime;



class User {
  String name;
  float x, y;
  boolean isHovered = false;
  boolean isClicked = false;
  int diametro = 30;  // Adicione o campo 'diametro' como um inteiro com valor inicial
  boolean estado = false;  // Adicione o campo 'estado' como uma variável booleana com valor inicial
  long lastDataUpdateTime; // Store the last time data was updated for this user
  int dataUpdateInterval = 5000; // Update data every 5 seconds
  boolean dataUpdated = false; // Add a flag to track data updates
  int lastHeartRate = 0; // Store the last obtained heart rate
  float lastSpo2 = 0; // Store the last obtained SPO2
  int lastFall = 0; // Store the last obtained fall value
  int lastPanicButton = 0; // Store the last obtained panic button state
  int localElapsedTime;
  
  User(String name, float x, float y) {
    this.name = name;
    this.x = x;
    this.y = y;
    this.lastDataUpdateTime = millis(); // Initialize the last update time
    this.localElapsedTime = globalElapsedTime;
  }

  void display(String externalData) {
  int baseColor = color(240); // Cor de fundo base
  int hoverColor = color(255, 100, 100); // Cor de fundo ao passar o mouse
  int clickColor = color(200); // Cor de fundo ao clicar

  fill(isClicked ? clickColor : (isHovered ? hoverColor : baseColor));
  rect(x, y, 400, rowHeight);

  fill(0);
  textAlign(LEFT, CENTER);
  textSize(24);
  text(name, x + 10, y + 10);

  // Parse the external data string
  String[] dataParts = split(externalData, ' ');
  
  if (dataParts.length == 4) {
    int heartRate = int(dataParts[0]);
    float spo2 = float(dataParts[1]);
    int fall = int(dataParts[2]);
    int panicButton = int(dataParts[3]);
    
    
    
    if (millis() - localElapsedTime >= dataUpdateInterval) {
      // Update the last data values
      lastHeartRate = heartRate;
      lastSpo2 = spo2;
      lastFall = fall;
      lastPanicButton = panicButton;
      
      // Update the last update time
      lastDataUpdateTime = millis();
    }
    
    // Display the last known data
    text("Heart Rate: " + lastHeartRate + " bpm", x + 10, y + 40);
    text("SPO2: " + lastSpo2, x + 10, y + 70);
    text("Fall: " + (lastFall == 1 ? "Fell" : "Normal"), x + 10, y + 100);
    text("Panic Button: " + (lastPanicButton == 1 ? "Pressed" : "Not Pressed"), x + 10, y + 130);
  }

  status(); // Chame o método status() sem passar argumentos.
}

  
void status() {
  // Calcule a posição do círculo com base na posição do texto "Blood Pressure" e "Heart Rate"
  float circleX = x + 370; // Ajuste a posição X como desejar
  float circleY = y + 80;  // Ajuste a posição Y como desejar
  
// Verifique se o mouse está sobre o círculo
  boolean isMouseOverCircle = dist(mouseX, mouseY, circleX, circleY) < diametro / 2;

  // Se o mouse estiver sobre o círculo e ele foi clicado, inverta o estado
  if (isMouseOverCircle && mousePressed) {
    // Inverta o estado da elipse
    estado = !estado;
  }

  // Use a variável "elipseVermelha" para determinar a cor do círculo
  if (estado) {
    fill(200, 0, 0);  // Vermelho
  } else {
    fill(0, 255, 0);  // Verde
  }

  // Desenhe o círculo
  ellipse(circleX, circleY, diametro, diametro);
}



  void checkHover(float mx, float my) {
    isHovered = (mx > x && mx < x + 400 && my > y && my < y + rowHeight);
  }
}

ArrayList<User> users = new ArrayList<User>();
ArrayList<User> clickedUsers = new ArrayList<User>();

int port = 12345;
Client esp32Client = null;
Client nodemcuClient = null;


void setup() {
  server = new Server(this, port);
  String serverIP = server.ip();
  println("Endereço IP do servidor: " + serverIP);
  cp5 = new ControlP5(this);
  size(1400, 900);
  numRows = 10;
  maxVisibleRows = (height / rowHeight) - 2;

  for (int i = 1; i <= numRows; i++) {
    User user = new User("Elderly User " + i, 200, 150 + (i - 1) * rowHeight);
    users.add(user);
  }

  connectButton = cp5.addButton("Connect")
    .setPosition(900, 310)
    .setSize(300, 80)
    .setCaptionLabel("Queda!!!")
    .setColorCaptionLabel(color(255))
    .setColorBackground(color(220, 0, 0))
    .setColorActive(color(200, 0, 0))
    .setFont(createFont("Arial", 32));

  connectButton.addListener(new ControlListener() {
    public void controlEvent(ControlEvent theEvent) {
      if (theEvent.getController() == connectButton) {
        println("IDOSO X NO CHÃO!!!!");
        
      }
    }
  });
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
    if (esp32Client.available() > 0) {
      String data = esp32Client.readString();
      if (data != null) {
        println("Data received from esp32: " + data);
        nodemcuClient.write(data);
        println("Data sent to nodemcu: " + data + "\n");
      }
    }
  }
  
  // Calculate elapsed time since the last data generation
  globalCurrentTime = millis();
  globalElapsedTime = globalCurrentTime - globalLastDataGenerationTime;
  
  if(globalCurrentTime < 5000){
    println("globalElapsedTime = "+globalElapsedTime);
  }
  
  // Check if it's time to generate new data
  if (globalElapsedTime >= dataGenerationInterval) {
      String randomData = generateRandomData();
      println("Dados articiais: " + randomData); // Prints the generated data string //<>//
    
    // Update the last data generation time
    globalLastDataGenerationTime = globalCurrentTime;
  }
  
  background(255);

  fill(0);
  textAlign(CENTER, CENTER);
  textSize(36);
  text("Monitoramento de Idosos - Visão de Administrador", width / 2, 50);

  for (int i = 0; i < maxVisibleRows; i++) {
    int rowIndex = i + viewportY / rowHeight;
    if (rowIndex < numRows) {
      User user = new User("Elderly User " + (rowIndex + 1), 200, 150 + i * rowHeight);
      user.checkHover(mouseX, mouseY);
      user.display(generateRandomData());
      if (user.isClicked && !clickedUsers.contains(user)) {
        clickedUsers.add(user);
      } else if (!user.isClicked && clickedUsers.contains(user)) {
        clickedUsers.remove(user);
      }
    }
  }

  displayArrowButton(75, 75, true);
  displayArrowButton(75, height - 75, false);

  for (User user : clickedUsers) {
    user.display(randomData);
  }
   
  // This should be at the end of your draw() function to control the frame rate
  frameRate(30); // Adjust the frame rate as needed
}

void displayArrowButton(float x, float y, boolean isUp) {
  fill(220);
  ellipse(x, y, 50, 50);
  fill(0);
  triangle(x - 5, y - 10, x, isUp ? y - 15 : y - 5, x + 5, y - 10);
}

void mousePressed() {
  if (mouseX > 50 && mouseX < 100) {
    if (mouseY > 50 && mouseY < 100) {
      scroll(-1);
    } else if (mouseY > height - 100 && mouseY < height) {
      scroll(1);
    } else {
      for (User user : users) {
        if (mouseX > user.x && mouseX < user.x + 400 && mouseY > user.y && mouseY < user.y + rowHeight) {
          if (user.isClicked) {
            user.isClicked = false;
          } else {
            user.isClicked = true;
          }
        } else {
          user.isClicked = false;
        }
      }
    }
  }
}

void scroll(int dir) {
  viewportY = constrain(viewportY + dir * rowHeight, 0, (numRows - maxVisibleRows) * rowHeight);
}
