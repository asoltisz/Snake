#include <SDL2/SDL.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <vector>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <fstream>
using namespace std;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

bool init();
void close();

class Object{
  public:
    int height = 10;
    int width = 10;
    int x_position, y_position;
    int x_previous, y_previous;

    Object(){
      x_position = 0; y_position = 0;
    }
    Object(int x, int y){
      x_position = x; y_position = y;
    }

    virtual bool checkCollision(Object&){}
    virtual void render(){}
};

class Fruit{
  public:
    Object fruit;
    bool eaten;

    Fruit(){
      place();
    }

    void place(){
      srand(time(NULL));
      eaten = false;
      //place the fruit at a random location on the canvas that is a multiple of
      //the size of the snake's segments
      fruit.x_position = rand() % SCREEN_WIDTH;
      fruit.x_position -= fruit.x_position % fruit.width;
      fruit.y_position = rand() % SCREEN_HEIGHT;
      fruit.y_position -= fruit.y_position % fruit.height;
    }

    void render(){
      SDL_Rect object = {fruit.x_position, fruit.y_position, fruit.width, fruit.height};
      SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0xFF, 0xFF);
      SDL_RenderFillRect(gRenderer, &object);
    }

    bool checkCollision(Object& snakeHead){
      if(snakeHead.x_position == fruit.x_position && snakeHead.y_position == fruit.y_position){
        eaten = true;
        return true;
      }
      else{
        eaten = false;
        return false;
      }
    }
};

class Snake{
  public:
    std::vector<Object> body;
    bool pauseGame;
    int score;

    //constructor for the Snake class
    Snake(){
      Object head;
      body.push_back(head); //add the head to the snake's body vector
      body[0].x_position = SCREEN_WIDTH/2;
      body[0].y_position = SCREEN_HEIGHT/2;
      x_velocity = 0; //zero the velocities to start
      y_velocity = 0;
      pauseGame = false;
      score = 0;
    }

    int velocity(){
      return abs(x_velocity) + abs(y_velocity);
    }

    //move the head of the snake according to keyboard events from the user
    void getUserInput(SDL_Event& event){
      if(event.type == SDL_KEYDOWN && event.key.repeat == 0){
        switch(event.key.keysym.sym){
          case SDLK_LEFT:
            if(x_velocity == 0 && !pauseGame){ //prevent the snake from reversing direction
              x_velocity = -body[0].width; //move the head by one head_width
              y_velocity = 0; //zero the y velocity to prevent diagonal movement
            }
            break;
          case SDLK_RIGHT:
            if(x_velocity == 0 && !pauseGame){
              x_velocity = body[0].width;
              y_velocity = 0;
            }
            break;
          case SDLK_UP:
            if(y_velocity == 0 && !pauseGame){ //prevent the snake from reversing direction
              y_velocity = -body[0].height; //move the head by one head_height
              x_velocity = 0; //zero the x velocity to prevent diagonal movement
            }
            break;
          case SDLK_DOWN:
            if(y_velocity == 0 && !pauseGame){
              y_velocity = body[0].height;
              x_velocity = 0;
            }
            break;
          case SDLK_SPACE:
            pauseGame = !pauseGame;
            printf("PAUSE\n");
            break;
          default:
            break;
        }
      }
    }

    void move(){
      //update the head's previous coordinates
      body[0].x_previous = body[0].x_position;
      body[0].y_previous = body[0].y_position;
      //update the head's current coordinates based on user input (velocity)
      body[0].x_position += x_velocity;
      body[0].y_position += y_velocity;

      //loop through the rest of the snake updating the position of each segment
      if(body.size() > 1){
        for(int segment = 1; segment < body.size(); segment++){ //start with the segment after the head
          //update this segment's previous coordinates
          body[segment].x_previous = body[segment].x_position;
          body[segment].y_previous = body[segment].y_position;
          //update this segment's current coordinates based on the previous
          //segment's previous coordinates
          body[segment].x_position = body[segment-1].x_previous;
          body[segment].y_position = body[segment-1].y_previous;
        }
      }
    }

    bool eat(Fruit& fruit){
      if(body[0].x_position == fruit.fruit.x_position && body[0].y_position == fruit.fruit.y_position){
        fruit.eaten = true;
        //the coordinates of the new segment are set to the previous coordinates of the
        //last segment
        Object* segment = new Object(body[body.size()-1].x_previous, body[body.size()-1].y_previous);
        body.push_back(*segment);
        score++;
        return true;
      }
      else{
        fruit.eaten = false;
        return false;
      }
    }

    bool gameOver(){
      //check if the snake's head has touched the bounderies
      if(body[0].y_position > (SCREEN_HEIGHT - body[0].height)){ //max y bounds
        body[0].y_position = SCREEN_HEIGHT - body[0].height;
        return true;
      }
      if(body[0].y_position < 0){ //min y bounds
        body[0].y_position = 0;
        return true;
      }
      if(body[0].x_position > (SCREEN_WIDTH - body[0].width)){ //max x bounds
        body[0].x_position = SCREEN_WIDTH - body[0].width;
        return true;
      }
      if(body[0].x_position < 0){ //min x bounds
        body[0].x_position = 0;
        return true;
      }

      //check if the snake's head has touched its body
      if(body.size() > 1){
        for(int segment = 1; segment < body.size(); segment++){
          if(body[0].x_position == body[segment].x_position && body[0].y_position == body[segment].y_position)
            return true;
        }
      }
      return false;
    }

    void restart(){
      body.clear();
      Object head;
      body.push_back(head);
      body[0].x_position = SCREEN_WIDTH/2;
      body[0].y_position = SCREEN_HEIGHT/2;
      x_velocity = 0; //zero the velocities to start
      y_velocity = 0;
      pauseGame = false;
      score = 0;
    }

    void render(){
      //render each segment of the snake individually
      for(int segment = 0; segment < body.size(); segment++){
        SDL_Rect segmentRect = {body[segment].x_position, body[segment].y_position, body[segment].width, body[segment].height};
        SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderFillRect(gRenderer, &segmentRect);
      }
    }

  private:
    //velocity variables for the head of the snake
    int x_velocity, y_velocity;
};

class Entry{
  public:
    Entry(){}
    Entry(std::string n, int s){
      score = s; name = n;
    }

    std::string name;
    int score;
};

class Highscores{
  public:
    std::vector<Entry> scores;

    Highscores(){}

    //function for printing all of 'scores' contents
    void printAll(){
      if(!scores.empty()){
        for(int index = 0; index < scores.size(); index++){
          printf("Name: %s | Score: %i\n", scores[index].name.c_str(), scores[index].score);
        }
      }
      else
        printf("NO SAVED SCORES\n");
    }

    void newScore(std::string newName, int newScore){
      readFile();
      Entry newEntry(newName, newScore);
      scores.push_back(newEntry);
      updateFile();
    }

    void readFile(){
      scores.clear();
      std::string line;
      std::ifstream scoreFile("highscores.txt");
      if(scoreFile.is_open()){
        while(getline(scoreFile, line)){
          Entry newEntry(line.substr(0, 3), atoi(line.substr(4).c_str()));
          scores.push_back(newEntry);
        }
        sortScores();
        scoreFile.close();
      }
    }

    void updateFile(){
      sortScores();
      std::ofstream scoreFile("highscores.txt");
      if(scoreFile.is_open() && !scores.empty()){
        for(int index = 0; index < scores.size(); index++)
          scoreFile << scores[index].name << " " << scores[index].score << "\n";
        scoreFile.close();
      }
    }

    void sortScores(){ //bubble sort
      if(!scores.empty()){
        bool done = false;
        while(!done){
          done = true;
          for(int index = 0; index < (scores.size() - 1); index++){
            if(scores[index].score < scores[index+1].score){
              Entry temp(scores[index].name, scores[index].score);
              //FIX ME: think about overloading the = operator
              scores[index].name = scores[index+1].name;
              scores[index].score = scores[index+1].score;
              scores[index+1].name = temp.name;
              scores[index+1].score = temp.score;
              done = false;
            }
          }
        }
      }
    }
};

bool init(){
  bool success = true;
  if(SDL_Init(SDL_INIT_VIDEO) < 0){
    printf("SDL could not initialize. SDL error: %s\n", SDL_GetError());
    success = false;
  }
  else{
    if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
      printf("Warning: Linear texture filtering not enabled.");
    gWindow = SDL_CreateWindow("SNAKE - Andrew Soltisz", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(gWindow == NULL){
      printf("Widnow could not be created. SDL Error: %s\n", SDL_GetError());
      success = false;
    }
    else{
      gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
      if(gRenderer == NULL){
        printf("Renderer could not be created. SDL Error: %s\n", SDL_GetError());
        success = false;
      }
      else
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    }
  }
  return success;
}

void close(){
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  gWindow = NULL;
  gRenderer = NULL;
  SDL_Quit();
}

bool mainMenu(Highscores& myScores){
  bool ready2play = false;
  std::string command;
  while(!ready2play){
    printf("Type 'play' to play the game.\n");
    printf("Type 'scores' to view highscores.\n");
    printf("Type 'exit' to exit the game.\n");
    printf("Command: ");
    std::cin >> command;

    if(command == "play")
      ready2play = true;
    else if(command == "scores"){
      myScores.readFile();
      myScores.printAll();
    }
    else if(command == "exit"){
      ready2play = true;
      printf("\n");
      return true;
    }
    else
      printf("Invalid command.\n");
    printf("\n");
  }
  return false;
}

int main(int argc, char* args[]){
  Highscores myScores;
  bool quit = false;
  printf("\n\n\n***** SNAKE *****\n");
  quit = mainMenu(myScores);
  if(!quit){
  if(!init()) //initialize SDL window
    printf("Failed to initialize.\n");
  else{
    std::string command;
    int updateCount;
    SDL_Event event;
    Snake snake;
    Fruit fruit;
    while(!quit){
      //user input
      while(SDL_PollEvent(&event)){
        if(event.type == SDL_QUIT)
          quit = true;
        else
          snake.getUserInput(event);
        }

      //game behavior
      if(updateCount % 30 == 0 && !snake.pauseGame){
        if(snake.eat(fruit)){
          fruit.place();
          printf("Score: %i\n", snake.score);
        }
        if(snake.gameOver()){
          std::string newName;
          int newScore = snake.score;
          printf("GAME OVER\n");
          snake.restart();
          printf("Enter your name: ");
          std::cin >> newName;
          myScores.newScore(newName, newScore);
          printf("Type 'quit' or 'play' to continue.\n");
          printf("Command: ");
          std::cin >> command;
          printf("\n");
          if(command == "quit")
            quit = mainMenu(myScores);
          fruit.place();
        }
        snake.move();
      }

      if(!quit){
        //graphics rendering
        SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderClear(gRenderer);
        snake.render();
        if(snake.velocity() != 0)
          fruit.render();
        SDL_RenderPresent(gRenderer);

        if(updateCount > 100)
          updateCount = 0;
        updateCount++;
      }
    }
  }
  close();
}
  return 0;
}
