#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

//DISPALY PINS
#define TFT_DC 9
#define TFT_CS 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

//Initial positions of snake, food, red food, snake features such as speed
int position[100][2] = {{12, 16}, {12, 17}, {12, 18}};  //Staring postions of the snake. Snake has three segments at start
int food_position[2];
int score = 0;
int highscore = 0;
int speed = 130;
int element = 3;
int richtung = 1; //Initially move up 1-up
int level = 1 ;
int barrier_position[2];
unsigned long food_start_time = 0; // Track when the food appears
unsigned long food_display_duration = 5000;  //The time where the food will dissappear from screen
bool food_visible = true;
int red_food_positions[10][2];// Position of red food which reduce score
bool red_food_active[10];
int red_food_count = 0;
bool red_food_visible = false;

//Below are the function prototypes (the functions used in game)
void startscreen();
void game();
void draw_foodandsnake();
void draw_logo();
void draw_design();
void check_direction();
void step();
void draw_quadrat(int x_pos, int y_pos, int color);
void check_and_delay(int ms);
void check_food();
void append_square();
void check_collision();
void draw_score();
void check_highscore();
void draw_level();
void place_barrier();
void erase_barrier();
void gameover();
void check_food_timer3();
void draw_countdown_timer();
void spawn_food_upto_level2();
void spawn_food_level3();
void spawn_food_level4();
void reset_game();
void spawn_food_level5();

//starting the game; Joystick button input pin_4 & Initialize display
void setup() {
  pinMode(4, INPUT_PULLUP);
  tft.begin();
}
//displaying the start screen feature. An opening of game
void loop() {
  startscreen();
  game();
}

// Start screen display
void startscreen() {
  tft.fillScreen(ILI9341_BLACK);
  draw_foodandsnake();
  draw_logo();
  while (digitalRead(4) == HIGH) {}// Wait for joystick press to start
}

// Game operations, draw design - game UI elements
void game() {
  draw_design();
  if (level < 3) {
    spawn_food_upto_level2();  // For Levels 1 and 2
  } 
  else if (level == 3) {
    spawn_food_level3();       // For Level 3
  } 
  else if( level == 4){
    spawn_food_level4();       // For Level 4
  }
  else if(level >=5){
    spawn_food_level5();
  }

  // Snake at the start position
  for (int i = 0; i < element; i++) {
    draw_quadrat(position[i][0], position[i][1], ILI9341_WHITE);
  }
  check_direction();// move the snake and monitor the snake
}

//Navigation of the snake and check the current direction
void check_direction() {
  while (true) {
    if (richtung == 1) {//Up direction
      step();
      position[0][1] -= 1;
    } else if (richtung == 2) {//Right direction
      step();
      position[0][0] += 1;
    } else if (richtung == 3) {  //Down direction
      step();
      position[0][1] += 1;
    } else if (richtung == 4) {  //Left direction
      step();
      position[0][0] -= 1;
    }

    //If the snake goes through an edge it will come back through opposite edge
    if (position[0][0] > 22) position[0][0] = 1;//if goes through right comes through left
    if (position[0][0] < 1) position[0][0] = 22;//if goes through left comes through right
    if (position[0][1] > 30) position[0][1] = 3;//if goes through down comes through top
    if (position[0][1] < 3) position[0][1] = 30;//if goes through top comes through down

    draw_quadrat(position[0][0], position[0][1], ILI9341_WHITE);// Draw head of the snake
    check_and_delay(speed);               //Delay and check for new joystick inputs
    if (level >= 3) {
      check_food_timer3();      //Check wether the food needs to be dissapear or not
      draw_countdown_timer();// show a countdown timer to dissapear the food
    }
    
    
  }
}

//The entire snake is being moved
void step() {
  int last = element - 1;
  draw_quadrat(position[last][0], position[last][1], ILI9341_BLACK);

  //Move each elements of the snake body
  for (int i = last; i > 0; i--) {
    position[i][0] = position[i - 1][0];
    position[i][1] = position[i - 1][1];
  }

  check_food();
  check_collision();
  
}

//check whether the snake eats the food
void check_food() {
  bool respawnNeeded = false;

  //If the snake head position is same as food position then snake eats the food
  if (position[0][0] == food_position[0] && position[0][1] == food_position[1]) {
    score += 1;//for each food score increases by 1
    draw_score();//show the current score in the display
    append_square();//each time snake eats the food, snake length is increased by one square
    respawnNeeded = true;// Set flag to respawning the food

    //Each level is incremented when score reaches two at that level
    if (score % 2 == 0) {
      level++;
      draw_level();//displaying the current level on display
    }

    //A barrier will appear on display
    if (level >= 2) {
      place_barrier();
    }

    if (level >= 5){
        speed = speed*0.8;
        red_food_count++;
        erase_barrier();
    }

  }
  //Introducing red food which will reduce the score by 1
  for (int i = 0; i < red_food_count; i++) {
    if (red_food_active[i] && position[0][0] == red_food_positions[i][0] && position[0][1] == red_food_positions[i][1]) {
      score -= 1;// when snake eats the food, score is reduced by 1 
      draw_score();//new score after reduction
      red_food_active[i] = false;// Disable red food, once snake eats it.
    }
  }

  if (respawnNeeded) {
    if (level < 3) {
      spawn_food_upto_level2();//food respawns without any timers for level 1, level 2
    }
    if (level >= 3) {
      spawn_food_level3();//A timer is set to dissapear food after predefined time
    }
    if (level == 4) {
      spawn_food_level4();//Red and normal food will appear from level 4 and more
    }
    if(level >= 5){
      spawn_food_level5();
    }
  }   
}

//Identifying whether the snake collide with itslef.(To terminate the game), Identifying the whether the snake collide with the barrier that appears
void check_collision() {
  //Check collision with barrier that appears
  if (position[0][0] == barrier_position[0] && position[0][1] == barrier_position[1]) {
    gameover();
  }

  // Check whether the snake collide with itslef.
  for (int i = 2; i < element; i++) {
    if (position[0][0] == position[i][0] && position[0][1] == position[i][1]) {
      gameover();
    }
  }
}

//Increasing the length of the snake
void append_square() {
  position[element][0] = position[element - 1][0];
  position[element][1] = position[element - 1][1];
  element++;
}

//Checks the Joystick input
void check_and_delay(int ms) {
  for (int i = 0; i < ms; i++) {
    delay(1);
    //UP
    if (analogRead(A0) == 1023 && richtung != 3) {
      richtung = 1;
    }
    //DOWN
    if (analogRead(A0) == 0 && richtung != 1) {
      richtung = 3;
    }
    //LEFT
    if (analogRead(A1) == 0 && richtung != 4) {
      richtung = 2;
    }
    //RIGHT
    if (analogRead(A1) == 1023 && richtung != 2) {
      richtung = 4;
    }
  }
}

void draw_quadrat(int x_pos, int y_pos, int color) {
  for (int i = 0; i < 9; i++) {
    for (int j = 0; j < 9; j++) {
      tft.drawPixel(x_pos * 10 + i, y_pos * 10 + j, color);
    }
  }
}

//draw design of game display features such as color, size, score
void draw_design() {
  tft.fillScreen(ILI9341_BLACK);
  tft.drawRect(1, 49, 224, 350, ILI9341_BLACK);

  tft.setCursor(50, 5);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(4);
  tft.println("SCORE: ");
  tft.setCursor(195, 5);
  tft.println(score);
  
}

void draw_score () {
  tft.setCursor(195, 5);
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_BLACK);
  tft.println(score - 1);

  tft.setCursor(195, 5);
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_WHITE);
  tft.println(score);
}

void draw_foodandsnake() {

  tft.fillRect(50, 290, 180, 10, ILI9341_BLUE);
  tft.fillRect(220, 250, 10, 40, ILI9341_BLUE);
  tft.fillRect(20, 250, 200, 10, ILI9341_BLUE);
  tft.fillRect(20, 210, 10, 40, ILI9341_BLUE);
  tft.fillRect(20, 210, 150, 10, ILI9341_BLUE);

  tft.fillRect(162, 217, 3, 3, ILI9341_BLACK);
  tft.fillRect(162, 212, 3, 3, ILI9341_BLACK);

  tft.fillRect(190, 210, 10, 10, ILI9341_BLUE);

}

//DISPLAY GROUP NUMBER
void draw_logo (){
  tft.setCursor(30, 72);
  tft.setTextColor(ILI9341_ORANGE);
  tft.setTextSize(6);
  tft.println("GROUP");
  tft.setCursor(105, 125);
  tft.println("02");
}

//Identify whether high score or not & print high score
void check_highscore() {
  if (score > highscore) {
    highscore = score;

    //If the present score is the high score, print the new highscore
    tft.setCursor(40, 230);
    tft.setTextColor(ILI9341_DARKGREEN);
    tft.setTextSize(2);
    tft.println("NEW HIGHSCORE!");
  }

  tft.setCursor(40, 5);
  tft.setTextColor(ILI9341_DARKGREEN);
  tft.setTextSize(2);
  tft.println("HIGHSCORE:");
  tft.setCursor(180, 5);
  tft.println(highscore);
}

//Displaying game over
void gameover() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(15, 140);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(4);
  tft.println("GAME OVER");

  tft.setCursor(70, 200);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("Score:");
  tft.setCursor(155, 200);
  tft.println(score);

  check_highscore();

  tft.setCursor(7, 300);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.println("<press to continue>");

  while (digitalRead(2) == HIGH) {};

  reset_game();

}
//Initializing snake features when game over -reset condition
void reset_game() {
  
  richtung = 1;
  score = 0;
  level = 1;
  speed = 130;
  element = 3;

  for (int i = 0; i < 3; i++) {
    position[i][0] = 12;
    position[i][1] = 16 + i;
  }

  game();
}

//Placing a barrier at random place,(for the collision of snake to maek the game over)
void place_barrier() {
  bool validPosition = false;
  
  //Erasing the old barrier when a new barrier is appeared
  erase_barrier();

  //Random position for barrier
  while (!validPosition) {
    barrier_position[0] = random(1, 23);
    barrier_position[1] = random(15, 31);
    validPosition = true;

    //Check whether the barrier appears in snake position
    for (int i = 0; i < element; i++) {
      if (position[i][0] == barrier_position[0] && position[i][1] == barrier_position[1]) {
        validPosition = false;
        break;
      }
    }

    //Check whether the barrier is very close to the food
    if (abs(barrier_position[0] - food_position[0]) < 3 && abs(barrier_position[1] - food_position[1]) < 3) {
      validPosition = false;
    }
  }

  //New Barrier
  tft.setCursor(barrier_position[0] * 10, barrier_position[1] * 10);
  tft.setTextColor(ILI9341_RED);
  tft.setTextSize(4);
  tft.print("2");
}

//Level indicator in display
void draw_level() {
  tft.setCursor(5, 45);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Level: ");
  tft.print(level);
}



void spawn_food_upto_level2() {
  bool validPosition = false;
  
  while (!validPosition) {
    int random_x = random(1, 23);
    int random_y = random(10, 31);
    validPosition = true;

    //Identify whether the food appears in the snake
    for (int i = 0; i < element; i++) {
      if (position[i][0] == random_x && position[i][1] == random_y) {
        validPosition = false;
        break;
      }
    }

    // Check whether the food spawns too close to the barrier
    if (level == 2 && abs(random_x - barrier_position[0]) < 3 && abs(random_y - barrier_position[1]) < 3) {
      validPosition = false;
      erase_barrier();
    }

    if (validPosition) {
      food_position[0] = random_x;
      food_position[1] = random_y;
      draw_quadrat(random_x, random_y, ILI9341_BLUE);
    }
  }
}

void spawn_food_level3() {
  bool validPosition = false;

  while (!validPosition) {
    int random_x = random(1, 23);
    int random_y = random(15, 31);
    validPosition = true;

    //Identify whether the food appears in the snake
    for (int i = 0; i < element; i++) {
      if (position[i][0] == random_x && position[i][1] == random_y) {
        validPosition = false;
        break;
      }
    }

    //Check whether the food spawns too close to the barrier
    if (abs(random_x - barrier_position[0]) < 3 && abs(random_y - barrier_position[1]) < 3) {
      validPosition = false;  // Ensure a margin around the barrier
    }

    if (validPosition) {
      food_position[0] = random_x;
      food_position[1] = random_y;
      draw_quadrat(random_x, random_y, ILI9341_BLUE);

      //Introducing timer display in level 3 
      food_start_time = millis();
      food_visible = true;
    }
  }
}

void check_food_timer3() {
  unsigned long current_time = millis();

  if (food_visible && (current_time - food_start_time >= food_display_duration)) {
    //Dissapear the food, 5 seconds after it appears
    draw_quadrat(food_position[0], food_position[1], ILI9341_BLACK);
    food_visible = false;
    spawn_food_level3();

    
  }
}
//Show a countdown timer when food appears
void draw_countdown_timer() {
  if (food_visible) {
    int time_left = (food_display_duration - (millis() - food_start_time)) / 1000;
    tft.setCursor(10, 2);
    tft.setTextSize(4);
    tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
    tft.println(time_left);
  }
}

void erase_barrier() {// Erase the area of the previous barrier completely by filling it with the background color
  int textWidth = 22;
  int textHeight = 34;

  tft.fillRect(barrier_position[0] * 10, barrier_position[1] * 10, textWidth, textHeight, ILI9341_BLACK);
}


void spawn_food_level4() {
    bool redFoodValid = false;

    

    //Appear red food
    if (red_food_count < 1) {
        while (!redFoodValid) {
            int random_x = random(1, 23);
            int random_y = random(15, 31);
            redFoodValid = true;

            // Check whether red food spawns on the snake, normal blue food, or too close to the barrier
            for (int i = 0; i < element; i++) {
                if (position[i][0] == random_x && position[i][1] == random_y) {
                    redFoodValid = false;
                    break;
                }
            }
            if ((random_x == food_position[0] && random_y == food_position[1]) ||
                (abs(random_x - barrier_position[0]) < 3 && abs(random_y - barrier_position[1]) < 3)) {
                redFoodValid = false;
            }

            if (redFoodValid) {
                red_food_positions[red_food_count][0] = random_x;
                red_food_positions[red_food_count][1] = random_y;
                red_food_active[red_food_count] = true;
                draw_quadrat(random_x, random_y, ILI9341_RED);
                red_food_count++;//Increase the appearance of the red food
            }
        }
    }

    food_start_time = millis();
}

void spawn_food_level5() {
    bool redFoodValid = false;

    //Appear some more additional red food for level 5
    if (red_food_count > (2) && red_food_count < 10) {
        while (!redFoodValid) {
            int random_x = random(1, 23);
            int random_y = random(15, 31);
            redFoodValid = true;

            //Check whether red food spawns on the snake, normal blue food, or too close to the barrier
            for (int i = 0; i < element; i++) {
                if (position[i][0] == random_x && position[i][1] == random_y) {
                    redFoodValid = false;
                    break;
                }
            }
            if ((random_x == food_position[0] && random_y == food_position[1]) ||
                (abs(random_x - barrier_position[0]) < 3 && abs(random_y - barrier_position[1]) < 3)) {
                redFoodValid = false;
            }

            if (redFoodValid) {
                red_food_positions[red_food_count][0] = random_x;
                red_food_positions[red_food_count][1] = random_y;
                red_food_active[red_food_count] = true;
                draw_quadrat(random_x, random_y, ILI9341_RED);
                red_food_count++;
            }
        }
    }
    food_start_time = millis();
}




