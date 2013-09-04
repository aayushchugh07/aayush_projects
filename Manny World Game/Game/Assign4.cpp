#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include<cstdio>
#include<cstdlib>
#include<algorithm>
#include<vector>
#include<cmath>
#include<iostream>
#include<fstream>
#include<string>
#include<cstdlib>
#include<vector>
#include<ctime>
#include<cstring>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include "sound.h"
#include "imageloader.h"
#include "imageloader.cpp"
#if !defined(GLUT_WHEEL_UP)
#  define GLUT_WHEEL_UP   3
#  define GLUT_WHEEL_DOWN 4
#endif
#define PI 3.1415926
#define POINTS_PER_VERTEX 3
#define TOTAL_FLOATS_IN_TRIANGLE 9
#define TOWER_VIEW 0
#define FIRST_PERSON_VIEW 1
#define THIRD_PERSON_VIEW 2
#define TILE_VIEW 3
#define HELICOPTER_CAM 4
#define squaresize 100
#define mannyheight 20
#define VIEWOFFSET (150.0)
#define TILE_SIMPLE 0
#define TILE_LIFT 1
#define TILE_BLOCK 3
#define TILE_TELE 4
#define TILE_JUMP 5
#define TILE_START 6
#define TILE_END 7
#define COLORFRAMES 60
#define LIGHT_ALL 0
#define LIGHT_CONE 1
#define LEVEL_RANDOM 0
#define LEVEL_CUSTOM 1
#define LEVEL_COUNT 5
using namespace std;

float winwidth = 1360, winheight = 768, viewdist = 1000, viewx = -700, viewy = 600,helir=5000,helitheta=0,heliheight=600;vector<int> teletilesx,teletilesy;
int walkframeno = 0, iswalking = 0, isjumping = 0, jumpframeno = 0, view = HELICOPTER_CAM,colori=0,tileviewx=0,tileviewz=0,istracingmouse=0,lightmode;
int teleportstatus=-1; //status of teleport watching, 0=> waiting for jump to finish, 1 => ??, -1=> not waiting
int coinscore=0,maxcoinscore=0,curlevel=1,displayinglevelno=1;
int levelmode=LEVEL_CUSTOM;int starti=0,startj=9,endi=9,endj=0;
GLfloat black[]={0.0f,0.0f,0.0f,1.0f};
GLfloat reflectcolor[]={0.8f,0.8f,0.8f,1.0f};
GLuint texture1;
// <editor-fold defaultstate="collapsed" desc="OBJ Loading Code">
// Start OBJ Loading  source:http://openglsamples.sourceforge.net/

class Model_OBJ {
public:
    Model_OBJ();
    float* calculateNormal(float* coord1, float* coord2, float* coord3);
    int Load(char *filename); // Loads the model
    void Draw(); // Draws the model on the screen
    void Release(); // Release the model

    float* normals; // Stores the normals
    float* Faces_Triangles; // Stores the triangles
    float* vertexBuffer; // Stores the points which make the object
    long TotalConnectedPoints; // Stores the total number of connected verteces
    long TotalConnectedTriangles; // Stores the total number of connected triangles

};

Model_OBJ::Model_OBJ() {
    this->TotalConnectedTriangles = 0;
    this->TotalConnectedPoints = 0;
}

float* Model_OBJ::calculateNormal(float *coord1, float *coord2, float *coord3) {
    /* calculate Vector1 and Vector2 */
    float va[3], vb[3], vr[3], val;
    va[0] = coord1[0] - coord2[0];
    va[1] = coord1[1] - coord2[1];
    va[2] = coord1[2] - coord2[2];

    vb[0] = coord1[0] - coord3[0];
    vb[1] = coord1[1] - coord3[1];
    vb[2] = coord1[2] - coord3[2];

    /* cross product */
    vr[0] = va[1] * vb[2] - vb[1] * va[2];
    vr[1] = vb[0] * va[2] - va[0] * vb[2];
    vr[2] = va[0] * vb[1] - vb[0] * va[1];

    /* normalization factor */
    val = sqrt(vr[0] * vr[0] + vr[1] * vr[1] + vr[2] * vr[2]);

    float* norm = new float[3];
    norm[0] = vr[0] / val;
    norm[1] = vr[1] / val;
    norm[2] = vr[2] / val;


    return norm;
}

int Model_OBJ::Load(char* filename) {
    string line;
    ifstream objFile(filename);
    if (objFile.is_open()) // If obj file is open, continue
    {
        objFile.seekg(0, ios::end); // Go to end of the file, 
        long fileSize = objFile.tellg(); // get file size
        objFile.seekg(0, ios::beg); // we'll use this to register memory for our 3d model

        vertexBuffer = (float*) malloc(fileSize); // Allocate memory for the verteces
        Faces_Triangles = (float*) malloc(fileSize * sizeof (float)); // Allocate memory for the triangles
        normals = (float*) malloc(fileSize * sizeof (float)); // Allocate memory for the normals

        int triangle_index = 0; // Set triangle index to zero
        int normal_index = 0; // Set normal index to zero

        while (!objFile.eof()) // Start reading file data
        {
            getline(objFile, line); // Get line from file

            if (line.c_str()[0] == 'v') // The first character is a v: on this line is a vertex stored.
            {
                line[0] = ' '; // Set first character to 0. This will allow us to use sscanf

                sscanf(line.c_str(), "%f %f %f ", // Read floats from the line: v X Y Z
                        &vertexBuffer[TotalConnectedPoints],
                        &vertexBuffer[TotalConnectedPoints + 1],
                        &vertexBuffer[TotalConnectedPoints + 2]);

                TotalConnectedPoints += POINTS_PER_VERTEX; // Add 3 to the total connected points
            }
            if (line.c_str()[0] == 'f') // The first character is an 'f': on this line is a point stored
            {
                line[0] = ' '; // Set first character to 0. This will allow us to use sscanf

                int vertexNumber[4] = {0, 0, 0};
                sscanf(line.c_str(), "%i%i%i", // Read integers from the line:  f 1 2 3
                        &vertexNumber[0], // First point of our triangle. This is an 
                        &vertexNumber[1], // pointer to our vertexBuffer list
                        &vertexNumber[2]); // each point represents an X,Y,Z.

                vertexNumber[0] -= 1; // OBJ file starts counting from 1
                vertexNumber[1] -= 1; // OBJ file starts counting from 1
                vertexNumber[2] -= 1; // OBJ file starts counting from 1


                /********************************************************************
                 * Create triangles (f 1 2 3) from points: (v X Y Z) (v X Y Z) (v X Y Z). 
                 * The vertexBuffer contains all verteces
                 * The triangles will be created using the verteces we read previously
                 */

                int tCounter = 0;
                for (int i = 0; i < POINTS_PER_VERTEX; i++) {
                    Faces_Triangles[triangle_index + tCounter ] = vertexBuffer[3 * vertexNumber[i] ];
                    Faces_Triangles[triangle_index + tCounter + 1 ] = vertexBuffer[3 * vertexNumber[i] + 1 ];
                    Faces_Triangles[triangle_index + tCounter + 2 ] = vertexBuffer[3 * vertexNumber[i] + 2 ];
                    tCounter += POINTS_PER_VERTEX;
                }

                /*********************************************************************
                 * Calculate all normals, used for lighting
                 */
                float coord1[3] = {Faces_Triangles[triangle_index], Faces_Triangles[triangle_index + 1], Faces_Triangles[triangle_index + 2]};
                float coord2[3] = {Faces_Triangles[triangle_index + 3], Faces_Triangles[triangle_index + 4], Faces_Triangles[triangle_index + 5]};
                float coord3[3] = {Faces_Triangles[triangle_index + 6], Faces_Triangles[triangle_index + 7], Faces_Triangles[triangle_index + 8]};
                float *norm = this->calculateNormal(coord1, coord2, coord3);

                tCounter = 0;
                for (int i = 0; i < POINTS_PER_VERTEX; i++) {
                    normals[normal_index + tCounter ] = norm[0];
                    normals[normal_index + tCounter + 1] = norm[1];
                    normals[normal_index + tCounter + 2] = norm[2];
                    tCounter += POINTS_PER_VERTEX;
                }

                triangle_index += TOTAL_FLOATS_IN_TRIANGLE;
                normal_index += TOTAL_FLOATS_IN_TRIANGLE;
                TotalConnectedTriangles += TOTAL_FLOATS_IN_TRIANGLE;
            }
        }
        objFile.close(); // Close OBJ file
    } else {
        cout << "Unable to open file";
    }
    return 0;
}

void Model_OBJ::Release() {
    free(this->Faces_Triangles);
    free(this->normals);
    free(this->vertexBuffer);
}

void Model_OBJ::Draw() {
    glEnableClientState(GL_VERTEX_ARRAY); // Enable vertex arrays
    //glEnableClientState(GL_TEXTURE_COORD_ARRAY);				// MOD
    glEnableClientState(GL_NORMAL_ARRAY); // Enable normal arrays
    glVertexPointer(3, GL_FLOAT, 0, Faces_Triangles); // Vertex Pointer to triangle array
    glNormalPointer(GL_FLOAT, 0, normals); // Normal pointer to normal array
    glDrawArrays(GL_TRIANGLES, 0, TotalConnectedTriangles); // Draw the triangles
    glDisableClientState(GL_VERTEX_ARRAY); // Disable vertex arrays
    //	glDisableClientState(GL_TEXTURE_COORD_ARRAY);				//MOD
    glDisableClientState(GL_NORMAL_ARRAY); // Disable normal arrays
}
// end OBJ Loading// </editor-fold>
Model_OBJ thecoinmodel;
// start texture loading code
//Makes the image into a texture, and returns the id of the texture
GLuint loadTexture(Image* image) {
	GLuint textureId;
	glGenTextures(1, &textureId); //Make room for our texture
	glBindTexture(GL_TEXTURE_2D, textureId); //Tell OpenGL which texture to edit
	//Map the image to the texture
	glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
				 0,                            //0 for now
				 GL_RGB,                       //Format OpenGL uses for image
				 image->width, image->height,  //Width and height
				 0,                            //The border of the image
				 GL_RGB, //GL_RGB, because pixels are stored in RGB format
				 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
				                   //as unsigned numbers
				 image->pixels);               //The actual pixel data
	return textureId; //Returns the id of the texture
}
// end texture loading code
GLuint simpletexId,lifttexid,blocktexid,boundarytexId,teletexId,jumptexId,cointexId,startendtexId; //The id of the texture

void jump(int);
void walk(int);
void resetMatrix();
void fallcaller(int);
void tilereach();
void endgame();
void playsound();
void zoomviewin(int);
void coinsound(){
    soundb("coin.raw");
}
void winsound(){
    soundb("win.raw");
}
class tile {
public:
    float xs, xe, zs, ze, posx, posy, posz, ys, ye,phase,offy; // xs=xStart, posx, posy=positions
    int goingup,type,hascoin;
    tile() {
        xs = xe = ys = ye = zs = ze = posx = posy = 0, type = TILE_SIMPLE;phase=0;goingup=rand()%2;offy=0;hascoin=0;
    }
	void placeCoin(){hascoin=1;}
    tile(float x, float y, float z) {
        setpos(x, y, z);phase=0;goingup=rand()%2;offy=0;
        type = TILE_SIMPLE;
    }
	void drawCoin(){
		glPushMatrix();
		glBindTexture(GL_TEXTURE_2D,cointexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTranslatef(50,50,50);
		glRotatef(360*((float)colori)/((float)COLORFRAMES),0,1,0);
		thecoinmodel.Draw();
		glPopMatrix();
	}
    void setpos(float x, float y, float z) {
        posx = x;
        posy = y, posz = z;
        xs = posx - squaresize / 2;
        xe = posx + squaresize / 2;
        ys = posy - squaresize / 2;
        ye = posy + squaresize / 2;
        zs = posz - squaresize / 2;
        ze = posz + squaresize / 2;
    }
//    float *posafterjump(){
//        // applies to TILE_LIFT for now
//        int i=32;
//        float offy = this->offy;int goingup=this->goingup;
//        while (i--) {
//            //re-used code from draw()
//            if (goingup) {
//                if (offy > 200) {
//                    goingup = !goingup;
//                    offy -= COLORFRAMES / 15;
//                } else offy += COLORFRAMES / 15;
//            } else {
//                if (offy<-100) {
//                    goingup = !goingup;
//                    offy += COLORFRAMES / 15;
//                }
//
//            }
//        }
//        float *temp=new float[3];temp[0]=posx;temp[1]=offy;temp[2]=posz;
//        return temp;
//    }
    void draw() {
        // <editor-fold defaultstate="collapsed" desc="Draw boundary below">
        glPushAttrib(GL_NORMAL_ARRAY);
            glPushMatrix();
            glTranslatef(-500 + posx * 100, posy -squaresize/25, -500 + posz * 100);
			if(hascoin){drawCoin();}
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glPushAttrib(GL_COLOR_ARRAY);
            glPushAttrib(GL_LINE_WIDTH);
            GLfloat abrown[] = {(fabs((float)colori-COLORFRAMES/2)/COLORFRAMES)*0.25,(fabs((float)colori-COLORFRAMES/2)/COLORFRAMES)*0.20, (fabs((float)colori-COLORFRAMES/2)/COLORFRAMES)*.3,1.f};
			//GLfloat abrown[]={0.8,0.8,0.8,1.};
			glBindTexture(GL_TEXTURE_2D,boundarytexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, abrown);
			
            glLineWidth(10.0f);
            glBegin(GL_QUADS);
            glNormal3d(0, 1, 0);
			glTexCoord2f(0.0,0.0);
            glVertex3f(0, 0, 0);
			glTexCoord2f(1.0,0.0);
            glVertex3f(100, 0, 0);
			glTexCoord2f(1.0,1.0);
            glVertex3f(100, 0, 100);
			glTexCoord2f(0.0,1.0);
            glVertex3f(0, 0, 100);
            glEnd();
            glPopAttrib();
            glTranslatef(0,-posy +squaresize/25,0); // re-adjust to continue drawing tiles
            // </editor-fold>
			if (type == TILE_SIMPLE || type==TILE_START||type==TILE_END) {
            // <editor-fold defaultstate="collapsed" desc="Draw boundary square">
           // texture1 = sampletex.LoadTextureRAW("assets/tile.tif", true);
			if(type==TILE_SIMPLE)glBindTexture(GL_TEXTURE_2D,simpletexId);
			else glBindTexture(GL_TEXTURE_2D,startendtexId);
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if(lightmode==LIGHT_ALL){
                GLfloat tilecolor2[] = {0.5f, 0.5f, .5, 1.0f};
                GLfloat tilecolor1[] = {0.5f, 0.5f, .5, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tilecolor2);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, tilecolor1);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
            }
            else{
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, reflectcolor);
            }
            glBegin(GL_QUADS);
            glNormal3d(0, 1, 0);
			glTexCoord2f(0.0f,0.0f);
            glVertex3f(7, 0, 7);
            glTexCoord2f(1.0f,0.0f);
			glVertex3f(93, 0, 7);
            glTexCoord2f(1.0f,1.0f);
			glVertex3f(93, 0, 93);
            glTexCoord2f(0.0f,1.0f);
			glVertex3f(7, 0, 93);
            glEnd();
            glPopAttrib(); 
            glPopMatrix();
            glPopAttrib();
            
            // </editor-fold>
        }
        else if (type==TILE_BLOCK){
            // <editor-fold defaultstate="collapsed" desc="Draw blocking box">
			glBindTexture(GL_TEXTURE_2D,blocktexid);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            GLfloat tilecolor2[] = {0.8f,.8f,.8f,0.7f};
            // GLfloat tilecolor1[] = {0.5f, 0.1f, .1f, 0.85f};
           // GLfloat tilecolor1[]={0.6+fabs((float)colori-COLORFRAMES/2)/COLORFRAMES*0.3,0.6+fabs((float)colori-COLORFRAMES/2)/COLORFRAMES*0.3,0.6+fabs((float)colori-COLORFRAMES/2)/COLORFRAMES*0.3,1.f};
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tilecolor2);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black/*tilecolor1*/);
            glBegin(GL_QUADS);
            glNormal3d(0, -1, 0);
            glVertex3f(7, 0, 7);
            glVertex3f(93, 0, 7);
            glVertex3f(93, 0, 93);
            glVertex3f(7, 0, 93);
            glEnd();
            glBegin(GL_QUADS);
            glNormal3d(0, 1, 0);
            glVertex3f(7, 150, 7);
            glVertex3f(93, 150, 7);
            glVertex3f(93, 150, 93);
            glVertex3f(7, 150, 93);
            glEnd();
            glBegin(GL_QUADS);
            //glNormal3d(0,0,1);
			glTexCoord2f(0.0f,0.0f);
            glVertex3f(7, 0, 7);
			glTexCoord2f(1.0f,0.0f);
            glVertex3f(93, 0, 7);
			glTexCoord2f(1.0f,1.0f);
            glVertex3f(93, 150, 7);
			glTexCoord2f(0.0f,1.0f);
            glVertex3f(7, 150, 7);
            glEnd();
            glBegin(GL_QUADS);
            //glNormal3d(0, 1, 0);
			glTexCoord2f(0.0f,0.0f);
            glVertex3f(7, 0, 7);
			glTexCoord2f(1.0f,0.0f);
            glVertex3f(7,0,93);
			glTexCoord2f(1.0f,1.0f);
            glVertex3f(7, 150, 93);
			glTexCoord2f(0.0f,1.0f);
            glVertex3f(7, 150, 7);
            glEnd();
            glBegin(GL_QUADS);
            //glNormal3d(0, 1, 0);
			glTexCoord2f(0.0f,0.0f);
            glVertex3f(7, 0, 93);
			glTexCoord2f(1.0f,0.0f);
            glVertex3f(93, 0, 93);
			glTexCoord2f(1.0f,1.0f);
            glVertex3f(93, 150, 93);
			glTexCoord2f(0.0f,1.0f);
            glVertex3f(7, 150, 93);
            glEnd();
            glBegin(GL_QUADS);
            //glNormal3d(0, 1, 0);
			glTexCoord2f(1.0f,0.0f);
            glVertex3f(93, 0, 93);
			glTexCoord2f(0.0f,0.0f);
            glVertex3f(93, 0, 7);
			glTexCoord2f(0.0f,1.0f);
            glVertex3f(93, 150, 7);
			glTexCoord2f(1.0f,1.0f);
            glVertex3f(93, 150, 93);
            glEnd();
            
            // </editor-fold>
        }
        else if (type==TILE_LIFT){
             // <editor-fold defaultstate="collapsed" desc="Draw Up-Down moving lift">
           // texture1 = sampletex.LoadTextureRAW("assets/tile.tif", true);
                 
            // sampletex.FreeTexture(texture1);
//            float offy=(fabs((float)colori-(COLORFRAMES/2)))/(float)COLORFRAMES*200; //y offset from base

            if(goingup){
                if (offy > 200) {
                    goingup=!goingup;
                    offy -= COLORFRAMES / 15;
                }
                else offy += COLORFRAMES / 15;
            }
            else{
                if(offy<-100){
                    goingup=!goingup;
                    offy+=COLORFRAMES/15;
                }
                else offy-=COLORFRAMES/15;
            }
			glBindTexture(GL_TEXTURE_2D,lifttexid);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            //GLfloat tilecolor2[] = {0.1f,0.4f,.1f,1.0f};
            GLfloat tilecolor1[] = {0.8f, 0.8f, .8f, 1.f};
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tilecolor1);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
            glBegin(GL_QUADS);
            glNormal3d(0, 1, 0);
				glTexCoord2f(0.0,0.0);
            glVertex3f(7, offy, 7);
				glTexCoord2f(1.0,0.0);
            glVertex3f(93, offy, 7);
				glTexCoord2f(1.0,1.0);
            glVertex3f(93, offy, 93);
				glTexCoord2f(0.0,1.0);
            glVertex3f(7, offy, 93);
            glEnd();
            glPopAttrib(); 
            glPopMatrix();
            glPopAttrib();
            
                      
        }
		else if (type==TILE_TELE){
			glBindTexture(GL_TEXTURE_2D,teletexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if(lightmode==LIGHT_ALL){
                GLfloat tilecolor2[] = {0.5f, 0.5f, .5, 1.0f};
                GLfloat tilecolor1[] = {0.5f, 0.5f, .5, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tilecolor2);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, tilecolor1);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
            }
            else{
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, reflectcolor);
            }
            glBegin(GL_QUADS);
            glNormal3d(0, 1, 0);
			glTexCoord2f(0.0f,0.0f);
            glVertex3f(7, 0, 7);
            glTexCoord2f(1.0f,0.0f);
			glVertex3f(93, 0, 7);
            glTexCoord2f(1.0f,1.0f);
			glVertex3f(93, 0, 93);
            glTexCoord2f(0.0f,1.0f);
			glVertex3f(7, 0, 93);
            glEnd();
            glPopAttrib(); 
            glPopMatrix();
            glPopAttrib();
            
		}else if(type==TILE_JUMP){
			glBindTexture(GL_TEXTURE_2D,jumptexId);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            if(lightmode==LIGHT_ALL){
                GLfloat tilecolor2[] = {0.5f, 0.5f, .5, 1.0f};
                GLfloat tilecolor1[] = {0.5f, 0.5f, .5, 1.0f};
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tilecolor2);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, tilecolor1);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
            }
            else{
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, reflectcolor);
            }
            glBegin(GL_QUADS);
            glNormal3d(0, 1, 0);
			glTexCoord2f(0.0f,0.0f);
            glVertex3f(7, 0, 7);
            glTexCoord2f(1.0f,0.0f);
			glVertex3f(93, 0, 7);
            glTexCoord2f(1.0f,1.0f);
			glVertex3f(93, 0, 93);
            glTexCoord2f(0.0f,1.0f);
			glVertex3f(7, 0, 93);
            glEnd();
            glPopAttrib(); 
            glPopMatrix();
            glPopAttrib();

		}
		 // </editor-fold>
         //pop matrix, color, line width
        glPopAttrib(); 
        glPopMatrix();
        glPopAttrib();
    }
	void reach(){
		// what to do when readched
		tilereach();
	}
} **tiles;
class man {
public:
    Model_OBJ walker[33], jumper[33];
    int bposx, bposz;float bposy; //block (on which man stands) posx, block posy
    int dirn, dirnold;
    float offx, offy, offz, offangle,jumpheight,destheight,syoff;//syoff=static yoff, not multiplied

    man() {
        bposx = bposy = 0;
        bposz = 9;
        dirn = 0;
        offx = offy = offz = 0;
        offangle = 0;
        dirnold = 0;jumpheight=150;destheight=0;syoff=0;
    }

    void manjump() {
        if (!isjumping) {
            
            isjumping = 1;
            forward();
            jump(0);
        }
    }

    void manwalk() {
        if (!iswalking) {
            iswalking = 1;
            walk(0);
        }
    }
    void fall(int i){
        //iswalking=isjumping=0;
        //bposy=-1000;
        if(i==0)fallcaller(0);
        if(i<32){
            syoff=-100-100*i;
            draw();
        }
        else{
            dirnold=1;dirn=1;
            bposx=starti;bposz=startj;syoff=0;iswalking=0;isjumping=0;offx=offy=offz=0;offangle=0;walkframeno=0;jumpframeno=0;
            draw();
        }
        
    }

    void draw() {
         glPushAttrib(GL_NORMAL_ARRAY);
         glPushMatrix();
        GLfloat abrown[] = {0.3+(fabs((float)colori-COLORFRAMES/2)/COLORFRAMES)*0.3,0.3f+(fabs((float)colori-COLORFRAMES/2)/COLORFRAMES)*0.3, 0.3+(fabs((float)colori-COLORFRAMES/2)/COLORFRAMES)*.3f,1.0f};
		glBindTexture(GL_TEXTURE_2D,boundarytexId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		bposy=tiles[bposx][bposz].offy;
        if(lightmode==LIGHT_ALL){
        //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, abrown);
        //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, abrown);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, abrown);
         //   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
        }else{
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, reflectcolor);
            
        }
        if (isjumping) {
            if (jumpframeno < 9) {
                bposy+=-(jumpframeno)*2;
            } else if (jumpframeno < 21) {
                 bposy+=-16 + (16+jumpheight)* (jumpframeno - 8)/12 ;
            } else if (jumpframeno < 23) {
                bposy+= jumpheight;
            } else {
                bposy+=jumpheight - (jumpheight-destheight)/9 * (jumpframeno - 22);
            }

        }
        glTranslatef(-450 + bposx * 100, 46+bposy+syoff, -450 + bposz * 100);
        glTranslatef((float) walkframeno*offx, (float) walkframeno*offy, (float) walkframeno * offz);
        
        glRotatef(-90.0f * (float) dirnold + (float) walkframeno*offangle, 0.0f, 1.0f, 0.0f);
        if (isjumping) {
            jumper[jumpframeno].Draw();

        } else /*if(iswalking)*/walker[walkframeno].Draw();
        glPopMatrix();
        glPopAttrib();
		if(!iswalking && !isjumping && teleportstatus==0){
			teleportstatus=1;
			tilereach();
			teleportstatus=-1;
		}
    }

    void forward() {
        if (!iswalking) {
            dirnold = dirn;
            /*if(view==FIRST_PERSON_VIEW){*/
            switch (dirn) {
                case 0:offz = -100.0f / 32.0;
                    break;
                case 1:offx = 100.0f / 32.0;
                    break;
                case 2:offz = 100.0f / 32.0;
                    break;
                case 3:offx = -100.0f / 32.0;
                    break;
            }
            /*}*/
            /*else{	dirn=0;offz=-100.0f/32;}*/
			teleportstatus=0; //teleport, forward is called in jump also
            manwalk();
        }

    }

    void backward() {
        if (!iswalking) {
            dirnold = dirn;
            //if(view==FIRST_PERSON_VIEW){
            offangle = 180.0 / 32.0;
            dirn = (dirn + 2) % 4;
            /*else{
                    dirn=2;
                    offz=100.0f/32;}*/
            manwalk();
        }
    }

    void left() {
        if (!iswalking) {
            dirnold = dirn;
            /*if(view==FIRST_PERSON_VIEW){*/offangle = 90.0 / 32.0;
            dirn = (dirn + 3) % 4;
            /*else{dirn=3;offx=-100.0f/32;}*/
            manwalk();
        }
    }

    void right() {
        if (!iswalking) {
            dirnold = dirn;
            /*if(view==FIRST_PERSON_VIEW)
            {*/offangle = -90.0 / 32.0;
            dirn = (dirn + 1) % 4;
            /*else{*/
            /*		dirn=1;
                            offx=100.0f/32;
             */
            manwalk();
        }
    }

}manny;
void callmanjump(int arg){
	manny.manjump();
}
void tilereach(){
	//	cerr<<"Called tilereach with posx posy = "<<posx<<" "<<posy<<endl;
	if(tiles[(int)manny.bposx][(int)manny.bposz].type==TILE_TELE){
		int ni=rand()%(teletilesx.size());
		cerr<<"tele bposx bposz "<<manny.bposx<<" "<<manny.bposz<<endl;
		manny.bposx=teletilesx[ni];manny.bposz=teletilesy[ni]; // go to random tele tile
		cerr<<"tele2 bposx bposz "<<manny.bposx<<" "<<manny.bposz<<endl;
	}
	else if(tiles[(int)manny.bposx][(int)manny.bposz].type==TILE_JUMP){
		glutTimerFunc(5,callmanjump,0);
	}
	if(tiles[(int)manny.bposx][(int)manny.bposz].type==TILE_END){
            winsound();
            endgame();
	}
	if(tiles[(int)manny.bposx][(int)manny.bposz].hascoin==1){
		coinscore++;
                coinsound();
		tiles[(int)manny.bposx][(int)manny.bposz].hascoin=0;
	}

}
void fallcaller(int i){
    if(i==33) return;
    manny.fall(i+1);
    glutTimerFunc(25,fallcaller,i+1);
}

void drawsurface() {
    int i, j;
    
    for(i=0;i<10;i++){
        for(j=0;j<10;j++){
            tiles[i][j].draw();
        }
    }
}

void walk(int arg) {
    if (iswalking) {
        walkframeno++;
        walkframeno %= 32;
    } else {
        walkframeno = 0;
    }
    int newz=manny.bposz,newx=manny.bposx;
    if (manny.dirnold == manny.dirn) {
            switch (manny.dirn) {
                case 0:
                    newz -= 1;
                    break;
                case 1:
                    newx += 1;
                    break;
                case 2:
                    newz += 1;
                    break;
                case 3:
                    newx -= 1;
                    break;
            }
        }
    
   //if(walkframeno==1)manny.destheight=tiles[newx][newz].posafterjump()[1];
    if(newz>9 || newx<0|| newz<0|| newx>9){
        manny.fall(0);
        return;
    }
    if(walkframeno==26){
        if(manny.bposy<tiles[newx][newz].offy){
            cerr<<"fall at "<<newx<<" "<<newz<<endl;
            manny.fall(0);
            return;
        }
    }
    if(walkframeno==16){
        //collision
        if(tiles[newx][newz].type==TILE_BLOCK){
            manny.dirn=(manny.dirn+2)%4;
            manny.dirnold=manny.dirn;
            manny.bposx=newx;manny.bposz=newz;
            switch(manny.dirn){
                case 0:
                case 2:
                    manny.offz=-manny.offz;
                    break;
                case 1:
                case 3:
                    manny.offx=-manny.offx;
                    break;
                    
            }
        }
        
    }
    if (walkframeno) {
        glutTimerFunc(15, walk, 0);
    }
    else { //end walking
        iswalking = 0;
        manny.bposx=newx;manny.bposz=newz;
        
        manny.offx = manny.offy = manny.offz = manny.offangle = 0;
        manny.dirnold = manny.dirn;
		
    }
};

void jump(int arg) {
    if (isjumping) {
        jumpframeno++;
        jumpframeno %= 32;
    } else {
        jumpframeno = 0;
    }
    if (jumpframeno)glutTimerFunc(15, jump, 0);
	else {isjumping = 0;}
}

void resetMatrix() {
    // Reset transformations
    glLoadIdentity();
    // Set the camera
    if(view==TILE_VIEW || view==HELICOPTER_CAM){
        float x=-450+tileviewx*100;
        float z=-450+tileviewz*100;
        float y=tiles[tileviewx][tileviewz].offy+10;
        float cx0=-450 + manny.bposx * 100 +(float) walkframeno * manny.offx;
        float cz0=-450 + manny.bposz * 100 +(float) walkframeno * manny.offz;
        float cy0=(view==TILE_VIEW)?(max(manny.bposy,tiles[tileviewx][tileviewz].offy)):(manny.offy);
        
        if(view==TILE_VIEW)gluLookAt(x,y,z,cx0,cy0,cz0,0,1,0);
        else gluLookAt(helir*cos(helitheta),heliheight,helir*sin(helitheta),cx0,cy0,cz0,0,1,0);
    }
    else if (view == FIRST_PERSON_VIEW || view == THIRD_PERSON_VIEW) {
        float x = -450 + manny.bposx * 100 + (float) walkframeno * manny.offx;
        float y = 150 + (float) walkframeno * manny.offy+manny.bposy+manny.syoff;
        float z = -450 + manny.bposz * 100 + (float) walkframeno * manny.offz;
        if (isjumping) {
            if (jumpframeno < 9) {
                y += -(jumpframeno)*2;
            } else if (jumpframeno < 21) {
                y += -16 + 6 * (jumpframeno - 8);
            } else if (jumpframeno < 23) {
                y += -16 + 6 * (12);
            } else {
                y += -16 + 6 * (12) - 6 * (jumpframeno - 22);
            }
            /*glRotatef(-90.0f*(float)dirn,0.0f,1.0f,0.0f);
             */
        }
        float cx0 = x, cy0 = y/5, cz0 = z;
        switch (manny.dirn) {
            case 0:
                cz0 += -VIEWOFFSET;
                break;
            case 1:
                cx0 += VIEWOFFSET;
                break;
            case 2:
                cz0 += VIEWOFFSET;
                break;
            case 3:
                cx0 += -VIEWOFFSET;
                break;
            default:
                break;
        }
        if (manny.dirn != manny.dirnold) {


            switch (manny.dirnold) {
                case 0:
                    cz0 += -(32.0 - (float) walkframeno) / 32.0 * VIEWOFFSET;
                    cx0 = (x)+(cx0 - x)*(1 - (32.0 - (float) walkframeno) / 32.0);
                    break;
                case 1://	cx1=3000;
                    cx0 += (32.0 - (float) walkframeno) / 32.0 * VIEWOFFSET;
                    cz0 = (z)+(cz0 - z)*(1 - (32.0 - (float) walkframeno) / 32.0);
                    break;
                case 2:
                    cz0 += (32.0 - (float) walkframeno) / 32.0 * VIEWOFFSET;
                    cx0 = (x)+(cx0 - x)*(1 - (32.0 - (float) walkframeno) / 32.0);
                    break;
                case 3:
                    cx0 += -(32.0 - (float) walkframeno) / 32.0 * VIEWOFFSET;
                    cz0 = (z)+(cz0 - z)*(1 - (32.0 - (float) walkframeno) / 32.0);
                    break;
                default:
                    break;
            }
        }

        if (view == THIRD_PERSON_VIEW) {
            // y+=100;
            //            cz0 = z;
            //            cx0 = x;
            //            cy0 = y-mannyheight;
            y += mannyheight * 10;
            
            z -= 1.5 * (cz0 - z);
            x -= 1.5 * (cx0 - x);
        }
        else{ //fix for view inside character when jumping
            x+=(cx0-x)/5;
            z+=(cz0-z)/5;
                       
        }
        gluLookAt(x, y, z,
                cx0, cy0, cz0,
                0, 1, 0
                );
    } else
        gluLookAt(-viewx, viewy, viewdist,
            -450.0 + manny.bposx * 100.0 + (float) walkframeno * manny.offx, 200.0f, -450.0 + manny.bposz * 100.0 + (float) walkframeno * manny.offz,
            0.0f, 1.0f, 0.0f);

}

void gameon() {
    manny.draw();
}
void addtext(){
	//start adding font
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	//gluOrtho2D(0.0, winwidth, 0.0, winheight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(0.0, 1.0, 0.0); // Green
	// below is actual adding
	
	//now restore matrices
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	//done font adding
}
void renderScene(void) {
    int i, j, k;
    float x, y, z;
    // Clear Color and Depth Buffers
	//addtext();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    resetMatrix();
    //GLfloat light_ambient[]={0.0,0.0,1.0, 1.0};
    GLfloat light_specular[] = {0.0+fabs((float)colori-COLORFRAMES/2)/COLORFRAMES*0.9,0.0+fabs((float)colori-COLORFRAMES/2)/COLORFRAMES*0.9,0.0+fabs((float)colori-COLORFRAMES/2)/COLORFRAMES*0.8, 1.0}; //one direction scattered equally in all directions
    //GLfloat light_specular[] = {0.0+fabs((float)colori-COLORFRAMES/2)/COLORFRAMES*0.7,0.0+fabs((float)colori-COLORFRAMES/2)/COLORFRAMES*0.7,0.0+fabs((float)colori-COLORFRAMES/2)/COLORFRAMES*0.7, 1.0}; //comes from a particular direction, and it tends to bounce off the surface in a preferred direction
    GLfloat ldirn[]={(-450.0 + manny.bposx * 100.0 + (float) walkframeno * manny.offx+viewx), (-46.0f-viewy), (-450.0 + manny.bposz * 100.0 + (float) walkframeno * manny.offz-viewdist)};
    //GLfloat ldirn[]={0,-1,0};
    GLfloat light_position[] = {-viewx,viewy,viewdist};
    //GLfloat light_position[] = {-450.0 + manny.bposx * 100.0 + (float) walkframeno * manny.offx,500.0f, -450.0 + manny.bposz * 100.0 + (float) walkframeno * manny.offz};
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION,ldirn);
    //glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION,0.2f);
    //glLightf(GL_LIGHT1,GL_QUADRATIC_ATTENUATION,10.f);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF,20.0f);
    //glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    //glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
     glLightfv(GL_LIGHT1, GL_POSITION, light_position);
//    texture1=sampletex.LoadTextureRAW("assets/tile.tif",true);
//    glBegin( GL_QUADS );
//    glTexCoord2d(0.0,0.0); glVertex2d(0.0,0.0);
//    glTexCoord2d(1.0,0.0); glVertex2d(100.0,0.0);
//    glTexCoord2d(1.0,1.0); glVertex2d(100.0,100.0);
//    glTexCoord2d(0.0,1.0); glVertex2d(0.0,100.0);
//    glEnd();
//    sampletex.FreeTexture(texture1);
    
    //texture1=sampletex.LoadTextureRAW("C:/Users/Aayush/Desktop/gwtex.tiff",true);
    //sampletex.FreeTexture(texture1);

    /*GLfloat red[] = {1.f, .0f, .0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE, red);*/
    glPushMatrix();
    glPopMatrix();
    glPopAttrib();
    drawsurface();

    gameon();
	//start adding text
	float blue[]={.2,0.2,0.3,1.};
	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,blue);
	glRasterPos3f((-4.50+starti)*squaresize,150, (-4.5+startj)*squaresize);;// below left corner is 0,0 , this sets position
	char startst[]="Start";char endst[]="GOAL!";char score[50];sprintf(score,"%d/%d",coinscore,maxcoinscore);
	for(i=0;i<strlen(startst);i++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, (int)startst[i]);
	}
	glRasterPos3f((-4.50+endi)*squaresize,150, (-4.5+endj)*squaresize);
	for(i=0;i<strlen(endst);i++){
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, (int)endst[i]);
	}
	if(!(tiles[manny.bposx][manny.bposz].type==TILE_START)){
		glRasterPos3f(-450 + manny.bposx * 100 +(float) walkframeno * manny.offx,150+manny.bposy, -450 + manny.bposz * 100 +(float) walkframeno * manny.offz);// below left corner is 0,0 , this sets position
		for(i=0;i<strlen(score);i++){
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, (int)score[i]);
		}
	}
	if(displayinglevelno){
		glRasterPos3f(0,300, 0);// below left corner is 0,0 , this sets position
		char startst[50];sprintf(startst,"Level %d",curlevel);
		for(i=0;i<strlen(startst);i++){
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, (int)startst[i]);
		}
	}
	//end adding text
	glutSwapBuffers();
}

void changeSize(int w, int h) {

    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if (h == 0)
        h = 1;
#pragma warning(push)
#pragma warning(disable: 4244)
    float ratio = w * 1.0 / h;
#pragma warning(pop)
    // Use the Projection Matrix
    glMatrixMode(GL_PROJECTION);

    // Reset Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set the correct perspective.
    gluPerspective(45.0f, ratio, 10.0f, 5000.0f + viewdist);

    // Get Back to the Modelview
    glMatrixMode(GL_MODELVIEW);
}

void specialKeyHandler(int key, int mousex, int mousey) {
    if (key == GLUT_KEY_LEFT) {
        viewx -= 10;
        tileviewx=(tileviewx+9)%10;
    }
    if (key == GLUT_KEY_RIGHT) {
        viewx += 10;
        tileviewx=(tileviewx+1)%10;
    }
    if (key == GLUT_KEY_DOWN) {
        viewdist += 10;
        tileviewz=(tileviewz+9)%10;
    }
    if (key == GLUT_KEY_UP) {
        viewdist -= 10;
        tileviewz=(tileviewz+1)%10;
    }
    if (key == GLUT_KEY_PAGE_UP) {
        viewy += 10;
    }
    if (key == GLUT_KEY_PAGE_DOWN) {
        viewy -= 10;
    }

}
void colorupdate(int e){
    colori=(colori+1)%COLORFRAMES;
    glutTimerFunc(17,colorupdate,0);
}
void mytechinitialize() {
    glClearDepth(1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    //glEnable(GL_COLOR_MATERIAL);
    //GLfloat lightpos[] = {100.0f, 100.0f, -100.0f, 1.0f};
    //glLightfv(GL_LIGHT3, GL_POSITION, lightpos);
   /* GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    GLfloat light_position2[] = { 0.0f, 0.0f,-1.0f, 0.0f };
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position2);*/
    GLfloat light_ambient[] = {0.0,0.0,0.0, 1.0}; // all directions
    GLfloat light_diffuse[] = {1.0,1.0,1.0, 1.0}; //one direction scattered equally in all directions
    GLfloat light_specular[] = {1.0,1.0,1.0, 1.0}; //comes from a particular direction, and it tends to bounce off the surface in a preferred direction
    GLfloat light_position[] = {-1000.0, 1000.0, 1.000, 1.0};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    //glColor3f(1.0f,0.0f,0.0f);
    glEnable(GL_LIGHT0);
    
    GLfloat light_ambient2[] = {0.0,0.0,0.0, 1.0}; // all directions
    GLfloat light_diffuse2[] = {1.0,1.0,1.0, 1.0}; //one direction scattered equally in all directions
    GLfloat light_specular2[] = {1.0,1.0,1.0, 1.0}; //comes from a particular direction, and it tends to bounce off the surface in a preferred direction
    GLfloat light_position2[] = {1000.0, 100.0, 1000.0, 1.000};
	glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse2);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
    //glColor3f(1.0f,0.0f,0.0f);
	
	glEnable(GL_LIGHT2);

    glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR/*or GL_NEAREST */);
	//[TODO] continue here, add stuff to initializ textures
	Image *imagesimple=loadBMP("simple.bmp");
	Image *imageblock=loadBMP("wall1.bmp");
	Image *imagelift=loadBMP("lift.bmp");
	Image *imageboundary=loadBMP("wood1.bmp");
	Image *imagetele=loadBMP("tele.bmp");
	Image *imagejump=loadBMP("fire.bmp");
	Image *imagecoin=loadBMP("blocktex.bmp");
	Image *imagestartend=loadBMP("startend.bmp");
	boundarytexId-loadTexture(imageboundary);
	simpletexId=loadTexture(imagesimple);
    lifttexid=loadTexture(imagelift);
	blocktexid=loadTexture(imageblock);
	teletexId=loadTexture(imagetele);
	jumptexId=loadTexture(imagejump);
	cointexId=loadTexture(imagecoin);
	startendtexId=loadTexture(imagestartend);
	delete imagesimple;delete imagelift;delete imageblock;delete imageboundary;delete imagetele;delete imagejump;delete imagecoin;delete imagestartend;
	
	tiles = new tile*[10];
    for(int i=0;i<10;i++){tiles[i]=new tile[10];}
	colorupdate(0);
}
void zoomviewin(int arg){
	helir-=50;
	if(helir>1000){
		glutTimerFunc(10,zoomviewin,0);
	}
}
void mygameinitialize() {
    int i,j,x;
	coinscore=maxcoinscore=0;	
	teletilesx.clear();teletilesy.clear();
	for(i=0;i<10;i++){
		for(j=0;j<10;j++){
			tiles[i][j].phase=tiles[i][j].offy=tiles[i][j].posy=0.0f;
			tiles[i][j].hascoin=0;
		}
	}
	if(levelmode==LEVEL_RANDOM){
		starti=endj=0;startj=endi=9;
		for (i = 0; i < 10; i++) {
			for (j = 0; j < 10; j++) {
				//if(((i==0 && j==9)||(i==9 && j==0)))continue;
				tiles[i][j].setpos(i, 0, j);
				x=(rand() % 30);				
				if (x > 25) {
					tiles[i][j].type = TILE_BLOCK;				
					//todo place coins
				}
				else if (x<8){
					tiles[i][j].type = TILE_LIFT;
					tiles[i][j].phase=x*COLORFRAMES/14;
					tiles[i][j].offy=tiles[i][j].phase;
				}
				else if(x<10){
					tiles[i][j].type=TILE_TELE;
					if(!((i==0 && j==9)||(i==9 && j==0))){teletilesx.push_back(i);teletilesy.push_back(j);}
				}
				else if(x<12){
					tiles[i][j].type=TILE_JUMP;
				}
				else{
					tiles[i][j].type=TILE_SIMPLE;
					if(rand()%2==1){tiles[i][j].placeCoin();maxcoinscore++;}
				}
			}
		}
		tiles[0][9].type=TILE_START;if(tiles[0][9].hascoin){tiles[0][9].hascoin=0;maxcoinscore--;}
		tiles[9][0].type=TILE_END;if(tiles[9][0].hascoin){tiles[9][0].hascoin=0;maxcoinscore--;}
	}
	else{//load from file
		starti=endj=9;startj=endi=0;
		char fname[50],ip[50];sprintf(fname,"levels/level%d",curlevel);
		FILE *f=fopen(fname,"r");
		for(i=0;i<10;i++){
			for(j=0;j<10;j++){
				x=(rand() % 30);	
				tiles[i][j].setpos(i, 0, j);
				fscanf(f,"%s",ip);
				if(ip[0]=='b'){//block
					tiles[i][j].type=TILE_BLOCK;}
				else if(ip[0]=='l'){	//lift				
					tiles[i][j].type = TILE_LIFT;
					tiles[i][j].phase=x*COLORFRAMES/14;
					tiles[i][j].offy=tiles[i][j].phase;
				}
				else if(ip[0]=='t'){//tele
					tiles[i][j].type=TILE_TELE;
					if(!((i==0 && j==9)||(i==9 && j==0))){teletilesx.push_back(i);teletilesy.push_back(j);}
				}
				else if(ip[0]=='j'){//jump
					tiles[i][j].type=TILE_JUMP;}
				else if(ip[0]=='s'){//start
					tiles[i][j].type=TILE_START;
					starti=i;startj=j;
					manny.bposx=i;manny.bposz=j;manny.bposy=0;
				}
				else if(ip[0]=='e'){
					endi=i;endj=j;
					tiles[i][j].type=TILE_END;
				}
				else{
					tiles[i][j].type=TILE_SIMPLE;
					if(ip[0]=='c'){tiles[i][j].placeCoin();maxcoinscore++;}
					//otherwise ip[0]=.
				}
			}
		}
	}
	manny.bposx=starti;manny.bposz=startj;
	helir=5000;helitheta=0;
	zoomviewin(0);
}
void mousefunc(int button, int state, int x, int y){
    if(button==GLUT_LEFT_BUTTON){
        if(state==GLUT_DOWN){
            //x,z::700,1500
            helitheta=(x-680)*PI/680;
            istracingmouse=1;
        }
        else if(state==GLUT_UP){
            istracingmouse=0;
        }
    }
    else if(button==GLUT_WHEEL_UP){
        helir-=100;
    }
    else if(button==GLUT_WHEEL_DOWN){
        helir+=100;
    }
	else if(button==GLUT_RIGHT_BUTTON){
		if(state==GLUT_DOWN){
		if (view == HELICOPTER_CAM)view = THIRD_PERSON_VIEW;
		else view=HELICOPTER_CAM;}
	}
}
void passivemousefunc(int x,int y){
    if(istracingmouse){
        helitheta=(x-680)*PI/680;
        heliheight=(768-y)*1000/768;
    }
}
void motionfunc(int x,int y){
    if(istracingmouse){
        helitheta=(x-680)*PI/680;
        heliheight=(768-y)*1000/768;
    }
}
void playsound(){
    sound("back.raw");
}
void handleKey(unsigned char key, int mousex, int mousey) {

    if (key == ' ') {
        manny.manjump();
    }
    if (key == 'w' || key == 'W') {
        manny.forward();
    }
    if (key == 's' || key == 'S') {
        manny.backward();
    }
    if (key == 'a' || key == 'A') {
        manny.left();
    }
    if (key == 'd' || key == 'D') {
        manny.right();
    }
    if (key == 'v' || key == 'V') {
        if (view == HELICOPTER_CAM)view = FIRST_PERSON_VIEW;
        else if (view == FIRST_PERSON_VIEW)view = THIRD_PERSON_VIEW;
        else if (view == THIRD_PERSON_VIEW)view = TOWER_VIEW;
        else if (view == TOWER_VIEW)view = TILE_VIEW;
        else if (view == TILE_VIEW)view = HELICOPTER_CAM;
    }
    if(key=='e'){
        glDisable(GL_LIGHT1);
        glEnable(GL_LIGHT0); 
        lightmode=LIGHT_ALL;
    }
    if(key=='r'){
        glEnable(GL_LIGHT1); 
        glDisable(GL_LIGHT0);
        lightmode=LIGHT_CONE;
    }
	if(key==13){
		mygameinitialize();
	}
	if(key>='1' && key<='9'){
		curlevel=(int)key-'0';
		curlevel--;
		endgame();
		
	}
	if (key==27) {
		exit(0);
	}
}

void endgame(){
	/* load next level, end current level */

	curlevel++;
	if(curlevel<=LEVEL_COUNT){
		levelmode=LEVEL_CUSTOM;
	}else{
		levelmode=LEVEL_RANDOM;
	}
	helitheta=5000;
	mygameinitialize();
}
int main(int argc, char **argv) {
    playsound();
    int i, j, k, ig;
    float x, y, z, w;
	srand(time(NULL));
    cout << "Welcome to the game!!!" << endl << "loading.....";
    char manfname[] = "assets/man_000001.obj";
    char manjfname[] = "assets/manjump_000001.obj";
	char capfname[]="assets/capsule1.obj";
	thecoinmodel.Load(capfname);
    for (i = 1; i < 10; i++) {
        manfname[16] = (char) i + '0';
        manjfname[20] = (char) i + '0';
        manny.walker[i - 1].Load(manfname);
        manny.jumper[i - 1].Load(manjfname);
        cerr << manfname << endl;
    }
    for (i = 10; i < 33; i++) {
        manfname[15] = (char) (i / 10) + '0';
        manfname[16] = (char) (i % 10) + '0';
        manjfname[19] = (char) (i / 10) + '0';
        manjfname[20] = (char) (i % 10) + '0';
        manny.walker[i - 1].Load(manfname);
        manny.jumper[i - 1].Load(manjfname);
        cerr << manfname << endl;
    }
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(1360, 768);
    glutCreateWindow("Assignment 2");
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(handleKey);
    glutSpecialFunc(specialKeyHandler);
    glutMouseFunc(mousefunc);
    glutPassiveMotionFunc(passivemousefunc);
    glutMotionFunc(motionfunc);
    glutFullScreen(); // Put into full screen
    mytechinitialize();
    mygameinitialize();
    glutIdleFunc(renderScene);
    glutMainLoop();

    return 0;
}