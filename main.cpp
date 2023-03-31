//we're in the endgame now
//Steven Nguyen(ID#109933505), Anurag Uppuluri(ID#110352456), Israel Perez(ID#109666108)
/*
legend (has it that):
0: grassland (place to move)
1: wall
2: bush (place to hide)
3: enemy
4: player
5: chest (reach to win)
6: arrow set (get extra arrow)
*/
/*
 * MAZE Game Framework
 * Written by Dr. Dhanyu Amarasinghe Spring 2018
 */

#include <string.h>
#include <CommonThings.h>
#include <Maze.h>
#include <iostream>
#include <Timer.h>
#include <player.h>
#include <fstream>
#include <queue>
#include <stack>
#include <ctime>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <Enemies.h>

#include <wall.h>
#include <bush.h>
#include <math.h>


/* GLUT callback Handlers */

using namespace std;

//a couple of global variables
Maze *M = new Maze(10);
Player *P = new Player();
Maze *menu = new Maze(10), *won = new Maze(10), *lost = new Maze(10);

int numWalls = 0, numEnemies = 0, numBushes = 0, numArrows = 100;
int chestPos, quiverPos;                    // used for the 2 major collisions we check for

bool gameOn = false, hasWon = false, hasLost = false;

wall W[100];                                    // wall with number of bricks
Enemies E[100];                                  // create number of enemies
Timer *T0 = new Timer();
bush B[100];                                    // hedge with number of bushes

float wWidth, wHeight;                          // display window width and Height
float xPos,yPos;                                // Viewpoar mapping

int *enemy_layout = new int[100], *player_layout = new int[100];

string lastKeyPressed = "nothing";

void reset_game(){
    M = new Maze(10);                         // Set Maze grid size
    P = new Player();                       // create player
    menu = new Maze(10);                // set menu screen grid size
    won = new Maze(10);                 // set winning screen grid size
    lost = new Maze(10);                // set losing screen grid size

    numWalls = 0, numEnemies = 0, numBushes = 0, numArrows = 5; //initial number of objects in the maze and arrows with the player

    gameOn = false;                              // shifting from menu screen to game play
    hasWon = false;
    hasLost = false;

    T0 = new Timer();                        // animation timer

    enemy_layout = new int[100]; // enemy can move to a 1 but not to a 0
    player_layout = new int[100]; // player can move to a 1 but not to a 0

    lastKeyPressed = "nothing"; //helpful when the player shoots the arrow (the lightning bolt)
}

void display(void);                             // Main Display : this runs in a loop

float FPS = 10.0;
int time_now, time_prev;

void dynamic_delete(){ //free up all the dynamic memory allocations
    delete M;
    delete P;
    delete menu;
    delete won;
    delete lost;
    delete enemy_layout;
    delete player_layout;
    delete T0;
}

void resize(int width, int height)              // resizing case on the window
{
    wWidth = width;
    wHeight = height;

    if(width<=height)
        glViewport(0,(GLsizei) (height-width)/2,(GLsizei) width,(GLsizei) width);
    else
        glViewport((GLsizei) (width-height)/2 ,0 ,(GLsizei) height,(GLsizei) height);
}

//just for reading the input text file that has a 2D array of numbers representing maze objects, into a 1D integer array
//the key thing to note here is that in the text file the coordinates start at (0,0) at the top-left corner and end at (9, 9) at the bottom-right going across each row sequentially
//but in the actual graphical display of the maze that Dr. Dhanyu implemented, they start at (0,0) at the bottom-left corner and end at (9,9) at the top-right, again going across each row sequentially
//therefore we have had to adopt different kinds of transformations of the coordinates suitable to each usage when going from the final display to the internal 1D representation
//many-a-times the x-coordinates and the y-coordinates had to be swapped while going from one representation to the other
int* load_level(char* fileName)
{
    int* layout_int = new int[100];

    char* layout_char = new char[100];

    ifstream myFile(fileName);
    if(myFile.is_open())
    {
        int i = 0;
        while(!myFile.eof())
        {
            myFile >> layout_char[i];
            i++;
        }
        myFile.close();
        for(int i = 0; i < 100; i++){
            layout_int[i] = layout_char[i] - '0';
        }/*
        cout<<"\nThis is how the level has been loaded:\n";
        for(int i = 0; i<100; i++){
            cout<<"("<<i<<"):\t"<<"("<<i/10<<","<<i%10<<"):\t"<<layout_int[i]<<endl;
        }*/
        return layout_int;
    }
    else
    {
        return nullptr;
    }
}

//6. Main functions
//1. Display a maze based on the values from a matrix
void init()
{
    glEnable(GL_COLOR_MATERIAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0,0.0,0.0,0.0);
    gluOrtho2D(0, wWidth, 0, wHeight);

    T0->Start();                                        // set timer to 0

    glEnable(GL_BLEND);                                 //display images with transparent
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //M->loadBackgroundImage("images/bak.jpg");           // Load maze background image
    M->loadBackgroundImage("images/seamless-grass-texture-game-photoshop-5c74bece58dba.jpg");

    M->loadChestImage("images/Treasure_Chest_ID_810_sprite_002.png");              // load chest image

    M->loadSetOfArrowsImage("images/http___www.pngall.com_wp-content_uploads_2017_01_Lightning-Download-PNG.png");      // load set of arrows image

    //P->initPlayer(M->getGridSize(),6,"images/p.png");   // initialize player pass grid size,image and number of frames
    P->initPlayer(M->getGridSize(),6,"images/trump_run.png");

    P->loadArrowImage("images/lightning bolt.png");                // Load arrow image

    //somehow displays flipped image
    menu->loadBackgroundImage("images/menu_screen_flipped.png");
    won->loadBackgroundImage("images/win_screen_flipped.png");
    lost->loadBackgroundImage("images/lost_screen_flipped.png");


    //in loading the level either the escape-sequenced backslash works or the forward slash alone without the escape sequence indicator backslash
    //int* layout_array = load_level("layouts/level1_no_enemy_50_98.txt");
    //int* layout_array = load_level("layouts\\level1_no_enemy_50_98.txt");
}

void load_maze(int* layout_array){
    for (int i = 0; i < 100; i++){
        switch(layout_array[i]){
    case 0: //both enemies and players can move in grasslands
        enemy_layout[i] = 1;
        player_layout[i] = 1;
        break;
    case 1:
        W[numWalls].wallInit(M->getGridSize(),"images/emboss-lighted-texture.png");// Load walls
        W[numWalls].placeWall(i%10,9-i/10);                              // place each brick

        numWalls++;
        enemy_layout[i] = 0; // neither enemies nor players can get to walls
        player_layout[i] = 0;
        break;
    case 2:
        B[numBushes].bushInit(M->getGridSize(),"images/bush_PNG7220.png");
        B[numBushes].placeBush(i%10, 9-i/10);
        numBushes++;
        enemy_layout[i] = 0; // enemies can't get to bushes but players can and hide
        player_layout[i] = 1;
        break;
    case 3:
        //E[numEnemies].initEnm(M->getGridSize(),4,"images/e.png"); //Load enemy image
        E[numEnemies].initEnm(M->getGridSize(),9,"images/C3ZwL_trump_enemies.png");

        E[numEnemies].placeEnemy(i%10,9-i/10);

        numEnemies++;
        enemy_layout[i] = 0; //2 enemies cannot occupy the same cell
        player_layout[i] = 1; //player can walk into the enemy and sacrifice themselves like some great avengers do in endgame
        break;
    case 4:
        P->placePlayer(i%10,9-i/10);                                // Place player

        enemy_layout[i] = 1; // the enemy can walk into you and kill you
        player_layout[i] = 1; //the player can choose to remain in the same spot
        break;
    case 5:
        M->placeChest(i%10,9-i/10);                                 // place chest in a grid

        chestPos = i; //just keeping track of the location in the 1D array for collision check with player
        enemy_layout[i] = 1; // enemies can walk over the chest
        player_layout[i] = 1; // if the player walks into the chest he's supposed to win
        break;
    case 6:
        M->placeStArrws(i%10,9-i/10);
        quiverPos = i; //keeping track of the location just like for the chest
        enemy_layout[i] = 1; // enemies can walk over the bag of arrows
        player_layout[i] = 1; //player can go grab an extra arrow
        break;
    default:
        enemy_layout[i] = 1; // if it's not one of these 7 cases then it's a mistake
        player_layout[i] = 1;
        break;
        }
    }
/*
    cout<<"Num Walls: "<<numWalls<<endl;
    cout<<"Num Enemies: "<<numEnemies<<endl;

    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++)
            cout<<layout_array[10*i+j]<<" ";
        cout<<endl;
    }
    cout<<endl;

    for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++)
            cout<<enemy_layout[10*i+j]<<" ";
        cout<<endl;
    }
    cout<<endl;

        for(int i = 0; i < 10; i++){
        for(int j = 0; j < 10; j++)
            cout<<player_layout[10*i+j]<<" ";
        cout<<endl;
    }
    for(int i = 0; i < numEnemies; i++){
        cout<<"enemy "<<i<<" loc: "<<9-E[i].getEnemyLoc().y<<", "<<E[i].getEnemyLoc().x<<endl;
    }
*/
}

void display(void)
{
    //this is for slowing down any animation like object rotations or how fast the arrow moves etc.
time_now = glutGet(GLUT_ELAPSED_TIME); //Get the current time
if((time_now - time_prev) > 1000/FPS) //Wait for render
{

  glClear (GL_COLOR_BUFFER_BIT);        // clear display screekn

        if(gameOn == false){         //then don't display the maze
            //glClear (GL_COLOR_BUFFER_BIT); //first clear, then display
        if(hasWon == true){
            //glClear (GL_COLOR_BUFFER_BIT); //first clear, then display
            glPushMatrix();
            won->drawBackground();          //display win screen
            glPopMatrix();
    //glutSwapBuffers();

        }

        else if(hasLost == true){
            //glClear (GL_COLOR_BUFFER_BIT); //first clear, then display
            glPushMatrix();
            lost->drawBackground();
            glPopMatrix();
    //glutSwapBuffers();

        }
        else {
          //glClear (GL_COLOR_BUFFER_BIT);
          glPushMatrix();

         menu->drawBackground();    //display the menu first
        glPopMatrix();

        }

        }

        else{
            //glClear (GL_COLOR_BUFFER_BIT); //first clear, then display

            glPushMatrix();
            M->drawBackground();
            glPopMatrix();

            for(int i=0; i<numWalls;i++)
        {
           W[i].drawWall();
        }


        for(int i=0; i<numBushes;i++)
        {
            B[i].drawBush();
        }

        glPushMatrix();
            M->drawGrid();
        glPopMatrix();

        glPushMatrix();
            P->drawplayer();
        glPopMatrix();

        for(int i=0; i<numEnemies;i++)
        {
        E[i].drawEnemy();
        }

        glPushMatrix();
            P->drawArrow();
        glPopMatrix();

         glPushMatrix();
           M->drawChest();
        glPopMatrix();

        glPushMatrix();
           M->drawArrows();
        glPopMatrix();
    //glutSwapBuffers();

        }

    glutSwapBuffers();
    //again for slowing down the animation
    time_prev = time_now; //End if timer
    }

}



void key(unsigned char key, int x, int y)
{
    switch (key)
    {
    case '1':{
        //dynamic_delete();
        //reset_game();
        gameOn = true;
        int* layout_array = load_level("layouts\\sample.txt"); //loads the easy level
        load_maze(layout_array);
        break;
    }
    case '2':{
        //dynamic_delete();
        //reset_game();
        gameOn = true;
        int* layout_array = load_level("layouts/level1.txt"); //loads the hard level
        load_maze(layout_array);
        break;
    }

    case '3':{
        //dynamic_delete();
        //reset_game();
        gameOn = true;
        srand(time(NULL));
        char* levels[4] = {"layouts/level1_no_enemy_50_98.txt", "layouts/Lvl_Layout_2.txt", "layouts/Lvl_Layout.txt", "layouts/Lvl_Layout_3.txt"};
        int randIndex = rand() % 4; //generates a random number between 0 and 3
        int* layout_array = load_level(levels[randIndex]);
        load_maze(layout_array);
        break;
    }

        case 'm':{
            //gameOn = false;
            //dynamic_delete();
            reset_game();
            //init();
            break;
        }

        //the thing to notice is that the player kills enemies only after the game registers that atleast one move has been taken by the player
        //to keep it simple the arrow does not affect anything except the enemies
        //it just passes through other objects and exits the maze if it doesn't meet an enemy
        case ' ':{
          // if(!M->liveSetOfArrws)      // if set of arrows was picked by player
            if(numArrows != 0){
             P->shootArrow();
             numArrows--;
        // x and y coordinates have to be swapped as the location (0,0) in Dr. Dhanyu's version is actually 90 in the corresponding 1D matrix
        // another strange thing to notice is that no matter what checks we do, the enemies start getting killed in order starting with the last one
        // yet another strange thing is that if there are more than 10 enemies on the board, they stop getting killed as soon as there are just 10 of them left in the game
             int playerX = 9-P->getPlayerLoc().y;
             int playerY = P->getPlayerLoc().x;
             int enemyX, enemyY;
             for(int i = 0; i < numEnemies; i++){
                enemyX = 9-E[i].getEnemyLoc().y;
                enemyY = E[i].getEnemyLoc().x;
                if(playerX == enemyX){
                    if(lastKeyPressed == "left" && enemyY < playerY){
                        E[i].live = false;
                        //P->arrowStatus = false;
                        numEnemies--;
                        enemy_layout[enemyX*10+enemyY] = 1; //spot open for another enemy
                    }
                    else if(lastKeyPressed == "right" && enemyY > playerY){
                        E[i].live = false;
                        //P->arrowStatus = false;
                        numEnemies--;
                        enemy_layout[enemyX*10+enemyY] = 1; //this instruction fixed the problem of how to kill the right enemy
                    }
                }
                else if(playerY == enemyY){
                    if(lastKeyPressed == "up" && enemyX < playerX){
                        E[i].live = false;
                        //P->arrowStatus = false;
                        numEnemies--;
                        enemy_layout[enemyX*10+enemyY] = 1;
                    }
                    else if(lastKeyPressed == "down" && enemyX > playerX){
                        E[i].live = false;
                        //P->arrowStatus = false;
                        numEnemies--;
                        enemy_layout[enemyX*10+enemyY] = 1;
                    }
                }
             }/*
    for(int i = 0; i < numEnemies; i++){
        cout<<"enemy "<<i<<" loc: "<<9-E[i].getEnemyLoc().y<<", "<<E[i].getEnemyLoc().x<<endl;
    }*/
            } //end biggest if

        break;
    } //end space case
        case 27 :                       // esc key to exit
        case 'q':
            exit(0);
            break;
    }

    glutPostRedisplay();
}


 void GetOGLPos(int x, int y)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

    xPos =posX ;
    yPos =posY ;
}

 void idle(void)
{

    //Your Code here

    glutPostRedisplay();
}


void mouse(int btn, int state, int x, int y){

    switch(btn){
        case GLUT_LEFT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;


      case GLUT_RIGHT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;
    }
     glutPostRedisplay();
};

// get_successor_squares gives us the next eligible square to explore in bfs
queue<int> get_successor_squares(int startState, int* matrix) {

	queue<int> eligibleSquares;
	//i = x*n + y ; //the usual formula for going to the 1D rep from the 2D rep

	int up = startState - 10;

	int down = startState + 10;

	int left = startState - 1;

	int right = startState + 1;


	//eligibleSquares is the list of all the successors of a vertex in bfs
	//action is the list of all the actions that were taken from the parent to reach corresponding successors

	//just making sure the successors respect the walls

    if (up/10 >= 0 && up%10 == startState%10) {
		if (matrix[up]==1) { eligibleSquares.push(up); /*cout << up << " ";*/ }
	}

	if (down/10 <= 9 && down%10 == startState%10) {
		if (matrix[down]==1) { eligibleSquares.push(down); /*cout << down << " ";*/ }
	}

	if (left%10 >= 0 && left/10 == startState/10) {
		if (matrix[left]==1) { eligibleSquares.push(left); /*cout << left << " ";*/ }
	}
    //error found here: just the check for right%10 <= 9 will take successor to be the first one of the next row eg: 89+1 = 90 which should not be the case
    //and hence we use a secondary condition that considers a successor only if it in in the same row
	if (right%10 <= 9 && right/10 == startState/10) {
		if (matrix[right]==1) { eligibleSquares.push(right); /*cout << right;*/}
	}

	cout << endl;

	return eligibleSquares;
}

void BreadthFirstSearch(int startState, int goalState, int* matrix, int*& color, int*& distance, int*& pi) {
	queue<int> frontier;
	color = new int[100];
	distance = new int[100];
	pi = new int[100];

	// initialize input/output arrays
	for (int i = 0; i < 100; i++) {
		color[i] = 0;
		distance[i] = 0;
		pi[i] = 0;
	}
	// but the startState index is initialized differently
	color[startState] = 1; // it's grayed out which means it's visited but not explored
	distance[startState] = 0; // the distance to itself is 0
	pi[startState] = -1; // it's got no predecessor

	frontier.push(startState);
	while (!frontier.empty()) {
		int toExplore = frontier.front(); // already gray
		frontier.pop();
		if (toExplore == goalState)
			return;
		queue<int> successorSquares = get_successor_squares(toExplore, matrix);
		while (!successorSquares.empty()) {
			int successor = successorSquares.front();
			successorSquares.pop();
			if (color[successor] == 0) {
				color[successor] = 1;
				distance[successor] = distance[toExplore] + 1;
				pi[successor] = toExplore;
				frontier.push(successor);
			}
		}
		color[toExplore] = 2;
	}
}

string BFS(int startState, int goalState) {
	int* color, * distance, * pi;

	BreadthFirstSearch(startState, goalState, enemy_layout, color, distance, pi);
	// now we just gotta retrace the parents to the greatest grand parent i.e., the startState to get the first right action
	stack<int> parentChain;
	string rightAction;

	int temp = goalState;
	//we stop retracing the parents if either the state under consideration is the startState or it is an orphan
    while ((temp != startState) && (pi[temp] != 0)) {
		parentChain.push(temp);
		temp = pi[temp];
	}
/*
	cout << "\nThis be the parent chain: ";
	for (stack<int> dump = parentChain; !dump.empty(); dump.pop())
		cout << dump.top() << " ";

	cout << "(" << parentChain.size() << " elements)\n";
*/
    if(!parentChain.empty()){
      	int firstChild = parentChain.top();
        if (firstChild == startState - 10)
            rightAction = "up";
        else if (firstChild == startState + 10)
            rightAction = "down";
        else if (firstChild == startState - 1)
            rightAction = "left";
        else if (firstChild == startState + 1)
            rightAction = "right";
        else rightAction = "nothing";
    }
    else rightAction = "nothing";

	cout << endl;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++)
			cout << color[10 * i + j] << "\t";
		cout << endl;
	}
	cout << endl;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++)
			cout << distance[10 * i + j] << "\t";
		cout << endl;
	}
	cout << endl;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++)
			cout << pi[10 * i + j] << "\t";
		cout << endl;
	}
	cout << endl;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++)
			cout << 10 * i + j << "\t";
		cout << endl;
	}

	delete[] color;
	delete[] distance;
	delete[] pi;

	return rightAction;
}

//just the player collision with the chest(treasure) or the quiver(lightning source)
void check_quiver_chest(int playerPos){
    if(playerPos == quiverPos){
        M->liveSetOfArrws = false;
        numArrows += 10;
    }
    else if(playerPos == chestPos)
    {
        gameOn = false;
        hasWon = true;
    }
}

int oldPlayerLoc = (9-P->getPlayerLoc().y) * 10 + P->getPlayerLoc().x; //useful to keep tract of the current/old player location


int get_new_enemy_loc(int enemyLoc, string nextAction){
        if (nextAction == "up")
            enemyLoc -= 10;
        else if (nextAction == "down")
            enemyLoc += 10;
        else if (nextAction == "left")
            enemyLoc -= 1;
        else if (nextAction == "right")
            enemyLoc += 1;
        return enemyLoc;
}

void Specialkeys(int key, int x, int y)
{

    // Your Code here
    switch(key)
    {
    case GLUT_KEY_UP:{
         //cout<<"Player location before keyup: "<<9-P->getPlayerLoc().y<< "    "<<P->getPlayerLoc().x<<endl;
         oldPlayerLoc = (9-P->getPlayerLoc().y) * 10 + P->getPlayerLoc().x;
         int up = oldPlayerLoc - 10;

         if(up/10 >= 0 && up%10 == oldPlayerLoc%10){
         if(player_layout[up]==1)
            P->movePlayer("up");
            lastKeyPressed = "up";
            check_quiver_chest(up);
         }
         //cout<<"Player location after keyup: "<<9-P->getPlayerLoc().y<< "    "<<P->getPlayerLoc().x<<endl;
         int newPlayerLoc = (9-P->getPlayerLoc().y) * 10 + P->getPlayerLoc().x;
         for(int i =0; i<numEnemies; i++){

        if(enemy_layout[newPlayerLoc]==1){ //if it's a square the enemy can move on then compute bfs to newPlayerLoc
            //cout<<"Enemy "<<i<<" location before keyup: "<<9-E[i].getEnemyLoc().y<< "    "<<E[i].getEnemyLoc().x<<endl;
            int enemyLoc = (9-E[i].getEnemyLoc().y) * 10 + E[i].getEnemyLoc().x;
            string nextAction = BFS(enemyLoc, newPlayerLoc);
            E[i].moveEnemy(nextAction);
            //int newEnemyLoc = (9-E[i].getEnemyLoc().y) * 10 + E[i].getEnemyLoc().x; //for some strange reason this is still showing the old enemy location
            //cout<<"Enemy "<<i<<" location after keyup: "<<9-E[i].getEnemyLoc().y<< "    "<<E[i].getEnemyLoc().x<<endl;
            enemy_layout[enemyLoc] = 1;
            //manually setting newEnemyLoc
            enemyLoc = get_new_enemy_loc(enemyLoc, nextAction);
            enemy_layout[enemyLoc] = 0; //the square that the enemy moved to is not available for grabs by the other enemies
            oldPlayerLoc = newPlayerLoc; //player location can be updated

            if(oldPlayerLoc == enemyLoc){
                P->livePlayer = false;
                gameOn = false;
                hasLost = true;
            }

        }
        else{//if the enemy can't go where the player just went e.g. a bush then compute bfs to oldPlayerLoc
            int enemyLoc = (9-E[i].getEnemyLoc().y) * 10 + E[i].getEnemyLoc().x;
            string nextAction = BFS(enemyLoc, oldPlayerLoc);
            E[i].moveEnemy(nextAction);
            enemy_layout[enemyLoc] = 1;
            enemyLoc = get_new_enemy_loc(enemyLoc, nextAction);
            enemy_layout[enemyLoc] = 0;

            if(oldPlayerLoc == enemyLoc){
                P->livePlayer = false;
                gameOn = false;
                hasLost = true;
            }


         }
         }
    break;
    }

    case GLUT_KEY_DOWN:{
        //cout<<"Player location before keydown: "<<9-P->getPlayerLoc().y<< "    "<<P->getPlayerLoc().x<<endl;
         oldPlayerLoc = (9-P->getPlayerLoc().y) * 10 + P->getPlayerLoc().x;
         int down = oldPlayerLoc + 10;
         //check for the striking the bounds of the maze and make sure if the x coordinate of the new (after taking down) and the old player locations match
         if(down/10 <= 9 && down%10 == oldPlayerLoc%10){
         if(player_layout[down]==1)
            P->movePlayer("down");
            lastKeyPressed = "down";
            check_quiver_chest(down);
         }
         //cout<<"Player location after keydown: "<<9-P->getPlayerLoc().y<< "    "<<P->getPlayerLoc().x<<endl;
         int newPlayerLoc = (9-P->getPlayerLoc().y) * 10 + P->getPlayerLoc().x;
        for(int i =0; i<numEnemies; i++){

        if(enemy_layout[newPlayerLoc]==1){
            //cout<<"Enemy "<<i<<" location before keyup: "<<9-E[i].getEnemyLoc().y<< "    "<<E[i].getEnemyLoc().x<<endl;
            int enemyLoc = (9-E[i].getEnemyLoc().y) * 10 + E[i].getEnemyLoc().x;
            string nextAction = BFS(enemyLoc, newPlayerLoc);
            E[i].moveEnemy(nextAction);
            oldPlayerLoc = newPlayerLoc;

            enemy_layout[enemyLoc] = 1;
            enemyLoc = get_new_enemy_loc(enemyLoc, nextAction);
            enemy_layout[enemyLoc] = 0;

                        if(oldPlayerLoc == enemyLoc){
                        P->livePlayer = false;
                        gameOn = false;
                        hasLost = true;
                        }
        }
        else{
            int enemyLoc = (9-E[i].getEnemyLoc().y) * 10 + E[i].getEnemyLoc().x;
            string nextAction = BFS(enemyLoc, oldPlayerLoc);
            E[i].moveEnemy(nextAction);
            enemy_layout[enemyLoc] = 1;
            enemyLoc = get_new_enemy_loc(enemyLoc, nextAction);
            enemy_layout[enemyLoc] = 0;

                        if(oldPlayerLoc == enemyLoc){
                        P->livePlayer = false;
                        gameOn = false;
                        hasLost = true;
                        }
         }
        }
    break;
    }

    case GLUT_KEY_LEFT:{
         //cout<<"Player location before keyleft: "<<9-P->getPlayerLoc().y<< "    "<<P->getPlayerLoc().x<<endl;
         oldPlayerLoc = (9-P->getPlayerLoc().y) * 10 + P->getPlayerLoc().x;
         int left = oldPlayerLoc - 1;

         if(left%10 >= 0 && left/10 == oldPlayerLoc/10){
         if(player_layout[left]==1)
            P->movePlayer("left");
            lastKeyPressed = "left";
            check_quiver_chest(left);
         }
         //cout<<"Player location after keyleft: "<<9-P->getPlayerLoc().y<< "    "<<P->getPlayerLoc().x<<endl;
         int newPlayerLoc = (9-P->getPlayerLoc().y) * 10 + P->getPlayerLoc().x;
        for(int i =0; i<numEnemies; i++){

        if(enemy_layout[newPlayerLoc]==1){
            //cout<<"Enemy "<<i<<" location before keyleft: "<<9-E[i].getEnemyLoc().y<< "    "<<E[i].getEnemyLoc().x<<endl;
            int enemyLoc = (9-E[i].getEnemyLoc().y) * 10 + E[i].getEnemyLoc().x;
            string nextAction = BFS(enemyLoc, newPlayerLoc);
            E[i].moveEnemy(nextAction);
            oldPlayerLoc = newPlayerLoc;

            enemy_layout[enemyLoc] = 1;
            enemyLoc = get_new_enemy_loc(enemyLoc, nextAction);
            enemy_layout[enemyLoc] = 0;

                        if(oldPlayerLoc == enemyLoc){
                            P->livePlayer = false;
                            gameOn = false;
                            hasLost = true;
                        }


        }
        else{
            int enemyLoc = (9-E[i].getEnemyLoc().y) * 10 + E[i].getEnemyLoc().x;
            string nextAction = BFS(enemyLoc, oldPlayerLoc);
            E[i].moveEnemy(nextAction);
            enemy_layout[enemyLoc] = 1;
            enemyLoc = get_new_enemy_loc(enemyLoc, nextAction);
            enemy_layout[enemyLoc] = 0;

                        if(oldPlayerLoc == enemyLoc){
                        P->livePlayer = false;
                        gameOn = false;
                        hasLost = true;
                        }


         }
    }
    break;

    }

    case GLUT_KEY_RIGHT:{
         //cout<<"Player location before keyright: "<<9-P->getPlayerLoc().y<< "    "<<P->getPlayerLoc().x<<endl;
         oldPlayerLoc = (9-P->getPlayerLoc().y) * 10 + P->getPlayerLoc().x;
         int right = oldPlayerLoc + 1;
        //similarly here too we gotta check for the bounds of the maze and make sure if the y coordinate of the new (after taking right) and the old player locations match
        if(right%10 <= 9 && right/10 == oldPlayerLoc/10){
         if(player_layout[right]==1)
            P->movePlayer("right");
            lastKeyPressed =  "right";
            check_quiver_chest(right);
         }
         //cout<<"Player location after keyright: "<<9-P->getPlayerLoc().y<< "    "<<P->getPlayerLoc().x<<endl;
         int newPlayerLoc = (9-P->getPlayerLoc().y) * 10 + P->getPlayerLoc().x;
        for(int i =0; i<numEnemies; i++){
            //E[i].moveEnemy("down");
        if(enemy_layout[newPlayerLoc]==1){
            //cout<<"Enemy "<<i<<" location before keyright: "<<9-E[i].getEnemyLoc().y<< "    "<<E[i].getEnemyLoc().x<<endl;
            int enemyLoc = (9-E[i].getEnemyLoc().y) * 10 + E[i].getEnemyLoc().x;
            string nextAction = BFS(enemyLoc, newPlayerLoc);
            E[i].moveEnemy(nextAction);
            oldPlayerLoc = newPlayerLoc;

            enemy_layout[enemyLoc] = 1;
            enemyLoc = get_new_enemy_loc(enemyLoc, nextAction);
            enemy_layout[enemyLoc] = 0;

                        if(oldPlayerLoc == enemyLoc){
                        P->livePlayer = false;
                        gameOn = false;
                        hasLost = true;
                        }


        }
        else{
            int enemyLoc = (9-E[i].getEnemyLoc().y) * 10 + E[i].getEnemyLoc().x;
            string nextAction = BFS(enemyLoc, oldPlayerLoc);
            E[i].moveEnemy(nextAction);
            enemy_layout[enemyLoc] = 1;
            enemyLoc = get_new_enemy_loc(enemyLoc, nextAction);
            enemy_layout[enemyLoc] = 0;

                        if(oldPlayerLoc == enemyLoc){
                                P->livePlayer = false;
                                gameOn = false;
                                hasLost = true;
                        }


         }
        }
    break;

    }

   }
  glutPostRedisplay();
}


/* Program entry point */

int main(int argc, char *argv[])
{
   //reset_game();

   glutInit(&argc, argv);

   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize (800, 800);                //window screen
   glutInitWindowPosition (100, 100);            //window position
   glutCreateWindow ("Maze");                    //program title
   init();

   glutDisplayFunc(display);                     //callback function for display
   glutReshapeFunc(resize);                      //callback for reshape
   glutKeyboardFunc(key);                        //callback function for keyboard
   glutSpecialFunc(Specialkeys);
   glutMouseFunc(mouse);
   glutIdleFunc(idle);
   glutMainLoop();

   //dynamic_delete();
   return EXIT_SUCCESS;
}
