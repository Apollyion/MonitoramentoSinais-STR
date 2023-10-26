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
int rowHeight = 100;
int numRows = 10;
int maxVisibleRows;
long lastStatusChangeTime = 0;
int statusChangeDuration = 5000; // 5000 milissegundos (5 segundos)
boolean isStatusChanging = false;

class User {
  String name;
  float x, y;
  boolean isHovered = false;
  boolean isClicked = false;
  int diametro = 30;  // Adicione o campo 'diametro' como um inteiro com valor inicial
  boolean estado = false;  // Adicione o campo 'estado' como uma variável booleana com valor inicial

  User(String name, float x, float y) {
    this.name = name;
    this.x = x;
    this.y = y;
    
  }

  void display() {
    int baseColor = color(240); // Cor de fundo base
    int hoverColor = color(255, 100, 100); // Cor de fundo ao passar o mouse
    int clickColor = color(200); // Cor de fundo ao clicar

    fill(isClicked ? clickColor : (isHovered ? hoverColor : baseColor));
    rect(x, y, 400, rowHeight);
    
    
    fill(0);
    textAlign(LEFT, CENTER);
    textSize(24);
    text(name, x + 10, y + 10);

    textSize(18);
    int heartRate = int(random(60, 100));
    String bloodPressure = int(random(90, 140)) + "/" + int(random(60, 90)) + " mmHg";
    text("Heart Rate: " + heartRate + " bpm", x + 10, y + 40);
    text("Blood Pressure: " + bloodPressure, x + 10, y + 70);
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
  size(1400, 800);
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
      user.display();
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
    user.display();
  }
   
  
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
