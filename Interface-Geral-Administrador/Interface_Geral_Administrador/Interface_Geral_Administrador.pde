import processing.serial.*;
import processing.net.*;
import controlP5.*;
import java.net.*;
import java.io.*;
import processing.core.PApplet;

ControlP5 cp5;               // Declare a ControlP5 object for creating GUI elements
Textfield ipAddressInput;    // Declare a Textfield for entering an IP address
Textfield portInput;         // Declare a Textfield for entering a port number
Button connectButton;        // Declare a Button for triggering a connection
Socket mainSocket;

Server server;

int viewportY = 0;
int rowHeight = 100;
int numRows = 10; // Replace 'rows' with 'numRows'
int maxVisibleRows;

class User {
  String name;
  float x, y;
  boolean isHovered = false;
  boolean isClicked = false;
  //int realTimeHeartRate;
  //String realTimeBloodPressure;

  User(String name, float x, float y) {
    this.name = name;
    this.x = x;
    this.y = y;
  }

  void display() {
  fill(isHovered ? color(255, 100, 100) : (isClicked ? color(255, 100, 100) : color(200)));
  rect(x, y, 400, rowHeight);

  fill(0);
  textAlign(LEFT, CENTER);
  textSize(28);
  text(name, x + 10, y + 10);

  //if (true/*isHovered || isClicked*/) {
    textSize(18);
    text("Heart Rate: " + int(random(60, 100)) + " bpm", x + 10, y + 40);
    text("Blood Pressure: " + int(random(90, 140)) + "/" + int(random(60, 90)) + " mmHg", x + 10, y + 70);
  //}
}

  void checkHover(float mx, float my) {
    isHovered = (mx > x && mx < x + 400 && my > y && my < y + rowHeight);
  }
}

ArrayList<User> users = new ArrayList<User>();
ArrayList<User> clickedUsers = new ArrayList<User>(); // Store clicked users

int port = 12345;
Client esp32Client = null;
Client nodemcuClient = null;

void setup() {
  server = new Server(this, port);
  String serverIP = server.ip();
  println("Endereço IP do servidor: " + serverIP);
  
  size(1400, 800);
  numRows = 10;
  maxVisibleRows = (height / rowHeight) - 2;
  //maxVisibleRows = height / rowHeight;
  //println(maxVisibleRows);
  // Initialize users and add User objects with dummy data
  for (int i = 1; i <= numRows; i++) {
    User user = new User("Elderly User " + i, 200, 150 + (i - 1) * rowHeight);
    users.add(user);
  }
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
  
  background(220);
  
  // Display a title above the column of rectangles
  fill(0);
  textAlign(CENTER, CENTER);
  textSize(36);
  text("Monitoramento de Idosos - Visão de administrador", width / 2, 50);
  
  // Display patients in the visible area
  for (int i = 0; i < maxVisibleRows; i++) {
    int rowIndex = i + viewportY / rowHeight;
    if (rowIndex < numRows) {
      User user = new User("Elderly User " + (rowIndex + 1), 200, 150 + i * rowHeight);
      user.checkHover(mouseX, mouseY); // Check for hover
      user.display();

      // If user is clicked, add to the list of clicked users
      if (user.isClicked && !clickedUsers.contains(user)) {
        clickedUsers.add(user);
      } else if (!user.isClicked && clickedUsers.contains(user)) {
        // If user is unclicked, remove from the list of clicked users
        clickedUsers.remove(user);
      }
    }
  }

  // Scroll Up button
  displayArrowButton(75, 75, true);

  // Scroll Down button
  displayArrowButton(75, height - 75, false);
  
  // Display data for clicked users
  for (User user : clickedUsers) {
    user.display();
  }
}

void displayUser(String name, float x, float y) {
  fill(200);
  rect(x, y, 400, rowHeight);
  fill(0);
  textAlign(LEFT, CENTER);
  textSize(28);
  text(name, x + 10, y + 10);
}

void displayArrowButton(float x, float y, boolean isUp) {
  fill(200);
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
      // Check for user click
      for (User user : users) {
        if (mouseX > user.x && mouseX < user.x + 400 && mouseY > user.y && mouseY < user.y + rowHeight) {
          if (user.isClicked) {
            user.isClicked = false;  // Toggle the click state
          } else {
            user.isClicked = true;
          }
        } else {
          user.isClicked = false;  // Deselect other users
        }
      }
    }
  }
}

void scroll(int dir) {
  // Update the viewport based on the scroll direction
  viewportY = constrain(viewportY + dir * rowHeight, 0, (numRows - maxVisibleRows) * rowHeight);
}
