/*
 * MAZE Game Framework
 * Written by Dr. Dhanyu Amarasinghe Spring 2018
 */
#ifndef BUSH_H
#define BUSH_H

#include<CommonThings.h>
#include<string.h>
#include<gl/gl.h>
#include<GL/glut.h>
#include<SOIL.h>
#include<iostream>
#include <Timer.h>
#include <math.h>       /* ceil */

class bush
{
    public:
        bush();                            // constructor
        virtual ~bush();                   // De constructor
        bush(int);                         // overload constructor

            void drawBush();               // Draw the Maze bush
            void bushInit(int, char *);    // initialize the bush
            void placeBush(int, int);      // place the bush

            float unitWidth;               // unit width of the grid cell
            int gridSize;                  // grid size

            bool liveBush;                 // bush status (broken or not)
    protected:

    private:
              loc bushShrub;                 // viewport location of one bush/shrub of the hedge
              GLuint bushTex;              // bush texture handler
              GridLoc GetBushLoc;          // Grid Location of the bush
};

#endif // BUSH_H
