/*
 *  CPE 471 lab 4 - draws a box without sides
 *  glut/OpenGL application
 *  uses glm for matrices
 *  Has support for simple transforms - check the keyboard function
 *
 *  Created by zwood on 1/6/12
 *  Copyright 2010 Cal Poly. All rights reserved.
 *
 *****************************************************************************/
#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#include <OpenAl/al.h>
#include <OpenAl/alc.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif
#include <chrono>
#include <sndfile.h>
#include <ALUT/alut.h>
#include <fftw3.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr

#include "Flock.h"

#define SWAP(a,b)tempr=(a);(a)=(b);(b)=tempr

#define NUM_BUFFERS 3
#define NUM_SOURCES 3
#define BUFFER_SIZE 4096
#define STEPS_PER_SEC 30

using namespace std;

//position and color data handles
GLuint triBuffObj, colBuffObj;

//flag and ID to toggle on and off the shader
int shade = 1;
int ShadeProg;

//Handles to the shader data
GLint h_aPosition;
GLint h_uColor;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLuint CubeBuffObj, CIndxBuffObj, GrndBuffObj, GIndxBuffObj;
int g_CiboLen, g_GiboLen;

static float g_width, g_height;
float g_trans = -1.5;
float g_Mtrans = 0;
float g_angle = 0;
static float g_scale = 1;

static const float g_groundY = -2.0;      // y coordinate of the ground
static const float g_groundSize = 100.0;   // half the ground length

int run_count=1;

Flock flock1, flock2, flock3;
double coh_weight=0, sep_weight=0;

//OpenAl vars
ALCcontext *context;
ALuint source;
ALuint buffer;

//fourier functions
double index2freq(int i);
int fundamental();
void   fourier(double *data, int num_samples, int sample_rate, int isign);
double magnitude(double re, double im);
int fund_window(double start, double end);
int freq2index(double freq);

//fourier data
SNDFILE *infile;
int channels=0;
double *in_c1, *in_c2;
fftw_complex *out;
fftw_plan p;

int bpm;
double interval;


/* projection matrix  - do not change */
void SetProjectionMatrix() {
   glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);
   safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
   glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(0.0f, 0, -55 + g_trans));
   glm::mat4 RotateX = glm::rotate( Trans, g_angle, glm::vec3(0.0f, 1, 0));
   safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(RotateX));
}

/* model transforms - change these to create a shape with boxes*/
void SetModel(float trans_x, float trans_y, float trans_z) {
   glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(g_Mtrans + trans_x,
                                                                trans_y, trans_z));
   safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(Trans));
}

/* code to create a large ground plane,
 * represented by a list of vertices and a list of indices  */
static void initGround() {

   // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
   float GrndPos[] = {
       g_groundSize, g_groundSize, 10,
       g_groundSize, -g_groundSize, 10,
       -g_groundSize, -g_groundSize, 10,
       -g_groundSize, g_groundSize,  10
   };
   unsigned short idx[] = {0, 1, 2, 0, 3, 2};

   g_GiboLen = 6;
   glGenBuffers(1, &GrndBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

   glGenBuffers(1, &GIndxBuffObj);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
}


/* code to create a cube, represented by a list of vertices and a list of indices */
/* note this a weird cube, because it dreams of being a house someday...(see nxt lab)*/
static void initCube() {
   float CubePos[] = {
      -0.1, -0.1, -0.1,
      -0.1,  0.1, -0.1,
       0.1,  0.1, -0.1,
       0.1, -0.1, -0.1,
      -0.1, -0.1,  0.1,
      -0.1,  0.1,  0.1,
       0.1,  0.1,  0.1,
       0.1, -0.1,  0.1
   };

   unsigned short idx[] = {0, 1, 2, 0, 2, 3, 7, 6, 4, 4, 6, 5, 1, 5, 6, 1, 6, 2, 0, 3, 7, 0, 7, 4, 0, 1, 5, 0, 5, 4, 2, 3, 6, 2, 5, 6};

    g_CiboLen = 36;
    glGenBuffers(1, &CubeBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);

    glGenBuffers(1, &CIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
}

/* Initialize the geometry */
void InitGeom() {
   initGround();
   initCube();
   float angle;

   srand(time(0));

   flock1 = Flock();
   flock2 = Flock();
   flock3 = Flock();
   for(int i = 0; i < 200; i++) {
      angle = 0 + (static_cast <float> (rand())) /
               (static_cast <float> (RAND_MAX/(6.28)));
      Boid *b = new Boid(i, 0.0, 0.0 , 0.0, angle);
      flock1.addBoid(b);
   }

   for(int i = 0; i < 200; i++) {
      angle = 0 + (static_cast <float> (rand())) /
               (static_cast <float> (RAND_MAX/(6.28)));
      Boid *b = new Boid(i, 0.8, 0.8 , 0.0, angle);
      flock2.addBoid(b);
   }

   for(int i = 0; i < 200; i++) {
      angle = 0 + (static_cast <float> (rand())) /
               (static_cast <float> (RAND_MAX/(6.28)));
      Boid *b = new Boid(i, -0.8, -0.8 , 0.0, angle);
      flock3.addBoid(b);
   }

}

/*function to help load the shaders (both vertex and fragment */
/* for this assignment we are doing anything interesting with the fragment sahder */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
   GLuint VS; //handles to shader object
   GLuint FS; //handles to frag shader object
   GLint vCompiled, fCompiled, linked; //status of shader

   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);

   //load the source
   glShaderSource(VS, 1, &vShaderName, NULL);
   glShaderSource(FS, 1, &fShaderName, NULL);

   //compile shader and print log
   glCompileShader(VS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
   printShaderInfoLog(VS);

   //compile shader and print log
   glCompileShader(FS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
   printShaderInfoLog(FS);

   if (!vCompiled || !fCompiled) {
      printf("Error compiling either shader %s or %s", vShaderName, fShaderName);
      return 0;
   }

   //create a program object and attach the compiled shader
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);

   glLinkProgram(ShadeProg);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
   printProgramInfoLog(ShadeProg);

   glUseProgram(ShadeProg);

   /* get handles to attribute and uniform data in shader */
   h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
   h_uColor = safe_glGetUniformLocation(ShadeProg,  "uColor");
   h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");

   printf("sucessfully installed shader %d\n", ShadeProg);
   return 1;
}


/* Some OpenGL initialization */
void Initialize ()					// Any GL Init Code
{
   // Start Of User Initialization
   glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
   // Black Background
   glClearDepth (1.0f);	// Depth Buffer Setup
   glDepthFunc (GL_LEQUAL);	// The Type Of Depth Testing
   glEnable (GL_DEPTH_TEST);// Enable Depth Testing
}

//draw a single flock of boids
void drawFlock(Flock flock, GLfloat red, GLfloat green, GLfloat blue) {
   for(int i=0; i < flock.flockSize(); i++) {

      Boid *b = flock.getBoid(i);
      float x = b->position.x;
      float y = b->position.y;
      float z = b->position.z;

      //cout << "i: " << x <<","<< y <<","<< z << endl;
      SetModel(x, y, z);
      safe_glEnableVertexAttribArray(h_aPosition);

      glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
      safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
      glUniform3f(h_uColor, red, green, blue);

      glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
      safe_glDisableVertexAttribArray(h_aPosition);
   }

}

/* Main display function */
void Draw (void)
{


   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glUseProgram(ShadeProg);

   /* set up the projection and camera - do not change */
   SetProjectionMatrix();
   SetView();

   //Draw Ground

   SetModel(0.0, 0.0, -50.0);
   safe_glEnableVertexAttribArray(h_aPosition);

   glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
   glUniform3f(h_uColor, 0.0, 0.0, 0.0);

   glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);
   safe_glDisableVertexAttribArray(h_aPosition);


   //draw flock of cubes
   drawFlock(flock1, 1.0, 0.0, 0.0);
   drawFlock(flock2, 0.0, 1.0, 0.0);
   drawFlock(flock3, 0.0, 0.0, 1.0);



   //Diisable the shader
   glUseProgram(0);
   glutSwapBuffers();

}

/* Reshape - note no scaling as perspective viewing*/
void ReshapeGL (int width, int height)
{
	g_width = (float)width;
	g_height = (float)height;
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));

}

void cleanUpFFT() {
   free(in_c1);
   if(channels == 2) {
      free(in_c2);
   }
   fftw_destroy_plan(p);
   fftw_free(out);
}

void cleanUpMusic() {
   sf_close(infile);
   ALCdevice *device;
   alDeleteSources(1, &source);
   alDeleteBuffers(1, &buffer);
   device = alcGetContextsDevice(context);
   alcMakeContextCurrent(NULL);
   alcDestroyContext(context);
   alcCloseDevice(device);
}

//the keyboard callback
void keyboard(unsigned char key, int x, int y ){
   switch( key ) {
      case 'w':
         g_trans += 0.1;
         break;
      case 's':
         g_trans -= 1;
         break;
      case 'a':
         g_angle += 1;
         break;
      case 'd':
         g_angle -= 1;
         break;
      case 'm':
         g_Mtrans -= .1;
         break;
      case 'n':
         g_Mtrans += .1;
         break;
      case 'q': case 'Q' :
         cleanUpMusic();
         cleanUpFFT();
         exit( EXIT_SUCCESS );
         break;
   }
   glutPostRedisplay();
}




//perform FFT, update position of boids in every flock
void flock(int t) {

   int16_t data[BUFFER_SIZE];




   //read data from WAV file
   if(run_count == 1) {
   if(BUFFER_SIZE != sf_read_short(infile, data, BUFFER_SIZE)) {
      cout << "Error loading data" << endl;
   }
   else {
      //cout << "song end" << endl;
   }

   //fill audio buffers
   if(channels == 1) {
      //mono
      for(int i=0; i < BUFFER_SIZE; i++) {
         in_c1[i] = (double)data[i];
      }
   } else {
      //stereo
      for(int i=0, j=0; i < BUFFER_SIZE; i+=2, j++) {
         in_c1[j] = (double)data[i];
         in_c2[j] = (double)data[i+1];
      }
   }

   fftw_execute(p);
   }
   int index = fundamental();

  // cout << "fundamental = " << index2freq(index) << endl;

/*
   for(int i=0; i < BUFFER_SIZE; i++) {
      cout << "" << i << "-" << index2freq(i)
      << " real: " << out[i][0]
      << " imag: " << out[i][1]
      << " magn: " << magnitude(out[i][0], out[i][1]) << endl;
   }
*/



   //calculate highest amplitude in each signal band
   float low=0;
   double low_mag=0;
   if((index = fund_window(0.0, 300.0)) > 0) {
      low   = index2freq(index);
      low_mag = magnitude(out[index][0], out[index][1]);
   } else {
      cout << "low window size out of bounds" << endl;
   }


   float mid=0;
   double mid_mag=0;
   if((index = fund_window(300.0, 600.0)) > 0) {
      mid   = index2freq(index);
      mid_mag = magnitude(out[index][0], out[index][1]);
   } else {
      cout << "mid window size out of bounds" << endl;
   }

   float high=0;
   double high_mag=0;
   if((index = fund_window(600.0, 1023.0)) > 0) {
      high  = index2freq(index);
      high_mag = magnitude(out[index][0], out[index][1]);
   } else {
      cout << "high window size out of bounds" << endl;
   }


   //normalize magnitudes
   double max_mag = max(max(low_mag, mid_mag), high_mag);
   high_mag = ceil((high_mag/max_mag) * 3) + 1;
   mid_mag  = ceil((mid_mag/max_mag) * 3)  + 1;
   low_mag  = ceil((low_mag/max_mag) * 3)  + 1;
   /*

   cout << "low= " << low << " mag="<< low_mag << endl;
   cout << "mid= " << mid << " mag="<< mid_mag << endl;
   cout << "high= " << high << " mag="<< high_mag << endl;
*/

   //set weights depndingon amplitude
   if(run_count == interval) {
      run_count = 0;
   }

   if(run_count > interval/2) {
      flock1.step(0, high_mag);
      flock2.step(0, high_mag);
      flock3.step(0, high_mag);
      //coh_weight = high_mag;
      //sep_weight = 0.0;
   } else {
      flock1.step(high_mag - 1, 1);
      flock2.step(mid_mag - 1, 1);
      flock3.step(low_mag - 1, 1);
      //coh_weight = 1.0;
      //sep_weight = high_mag - 1;
   }

   run_count++;
   /*
   flock1.step(sep_weight, coh_weight);
   flock2.step(sep_weight, coh_weight);
   flock3.step(sep_weight, coh_weight);
   */


   glutPostRedisplay();
   glutTimerFunc(0, flock ,0);

}

//init buffers and data for mono or stero WAV files for use with FFTW
void initFFT() {
   //allocate buffers for FFT
   if(channels == 1) {
      in_c1 = (double*) malloc(sizeof(double) * BUFFER_SIZE);
      out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (BUFFER_SIZE/2) + 1);
      p = fftw_plan_dft_r2c_1d(BUFFER_SIZE, in_c1, out, FFTW_ESTIMATE);
   }
   if(channels == 2) {
      in_c2 = (double*) malloc(sizeof(double) * BUFFER_SIZE/2);
      in_c1 = (double*) malloc(sizeof(double) * BUFFER_SIZE/2);
      out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (BUFFER_SIZE/4) + 1);
      p = fftw_plan_dft_r2c_1d(BUFFER_SIZE/2, in_c1, out, FFTW_ESTIMATE);
   }
}


//handle openal initializaiton, begin playing file
void initMusic(char *filename) {
   //start music
   ALCdevice *dev;
   ALsizei size, freq;
   ALenum format;
   ALvoid *data;
   ALboolean loop = AL_FALSE;
   ALCenum error;

   dev = alcOpenDevice(NULL);
   if(!dev) {
      cout << "FUCK" << endl;
   }

   //ALCcontext *context;
   context = alcCreateContext(dev, NULL);
   if(!alcMakeContextCurrent(context)) {
      cout << "Context error" << endl;
   }

   //ALuint source;
   alGenSources((ALuint)1, &source);
   error = alGetError();
   if(error != AL_NO_ERROR) {
      cout << "error" << endl;
   }
   alSourcef(source, AL_PITCH, 1);
   error = alGetError();
   if(error != AL_NO_ERROR) {
      cout << "error" << endl;
   }
   alSourcef(source, AL_GAIN, 1);
   error = alGetError();
   if(error != AL_NO_ERROR) {
      cout << "error" << endl;
   }
   alSource3f(source, AL_POSITION, 0, 0, 0);
   error = alGetError();
   if(error != AL_NO_ERROR) {
      cout << "error" << endl;
   }
   alSource3f(source, AL_VELOCITY, 0, 0, 0);
   error = alGetError();
   if(error != AL_NO_ERROR) {
      cout << "error" << endl;
   }
   alSourcei(source, AL_LOOPING, AL_FALSE);
   error = alGetError();
   if(error != AL_NO_ERROR) {
      cout << "error" << endl;
   }

   //ALuint buffer;
   alGenBuffers((ALuint)1, &buffer);

   alutLoadWAVFile(filename, &format, &data, &size, &freq);

   alBufferData(buffer, format, data, size, freq);

   alSourcei(source, AL_BUFFER, buffer);

   alSourcePlay(source);



}

//compute frequency for given index of output data
double index2freq(int i) {

   int buff_size;
   if(channels == 1) {
      buff_size = BUFFER_SIZE;
   } else {
      buff_size = BUFFER_SIZE/2;
   }

   return (double) i * 44100 / buff_size;
}

//compute the index for the given frequency
int freq2index(double freq) {
   int buff_size;
   if(channels == 1) {
      buff_size = BUFFER_SIZE;
   } else {
      buff_size = BUFFER_SIZE/2;
   }
   return ((freq * buff_size) / 44100);

}

//find the index of the fundamental frequency in the FFT output
int fundamental() {
   int index = 0;
   double max = 0;
   int buff_size;
   if(channels == 1) {
      buff_size = BUFFER_SIZE/2;
   } else {
      buff_size = BUFFER_SIZE/4;
   }
   for(int i=0; i < buff_size; i++) {
         double tmp = magnitude(out[i][0], out[i][1]);
         if(tmp > max) {
            index = i;
            max = tmp;
         }
   }
   return index;
}

//compute magnitude of a + bi signal
double magnitude(double re, double im) {
   double re2 = re * re;
   double im2 = im * im;
   double density = sqrt(re2 + im2);
   return density;
}

//find the fundamental of a window within
//range of given freqencies in Hz
int fund_window(double start, double end) {
   int index = 0;
   double max = 0;
   int buff_size;
   int start_index = freq2index(start);
   int end_index   = freq2index(end);
   if(channels == 1) {
      buff_size = BUFFER_SIZE/2;
   } else {
      buff_size = BUFFER_SIZE/4;
   }

   if(start_index < 0 || end_index > buff_size) {
      return -1;
   }

   for(int i=start_index; i < end_index; i++) {
      double tmp = magnitude(out[i][0], out[i][1]);
      if(tmp > max) {
         index = i;
         max = tmp;
      }
   }
   return index;
}

//calculate the number of flock() calls executed per beat
double calculate_interval(int bpm) {
   double bps = ((double)bpm)/60.0;
   double steps_per_beat = STEPS_PER_SEC/bps;

   return floor(steps_per_beat);
}

//Non optimized FFT algorithm. I chose to use the FFTW library to get a faster FFTW to better align
//with the music. I left the dead code in to show that I did research the algorithm
/*
void fourier(double *data, int num_samples, int sample_rate, int isign) {
   unsigned long n, mmax, m, j, istep, i;
   double wtemp, wr, wpr, wpi, wi, theta;
   double tempr, tempi;



   for(int i=0; i<num_samples; i++) {
      complex_data[i*2] = data[i];
      complex_data[i*2+1] = 0.0;
   }
   n = num_samples << 1;
   // reverse-binary reindexing
   j=0;
   for (i=0; i<n/2; i+=2) {
      if (j>i) {
         swap(complex_data[j], complex_data[i]);
         swap(complex_data[j+1], complex_data[i+1]);
         if((j/2)<(n/4)) {
            SWAP(complex_data[(n-(i+2))], complex_data[(n-(j+2))]);
            SWAP(complex_data[(n-(i+2))+1], complex_data[(n-(j+2))+1]);
         }
      }
      m = n >> 1;
      while (m>=2 && j>m) {
         j -= m;
         m >>= 1;
      }
      j += m;
   }


    // here begins the Danielson-Lanczos section
   mmax=2;
   while (n>mmax) {
      istep = mmax<<1;
      theta = isign*(2*M_PI/mmax);
      wtemp = sin(0.5*theta);
      wpr = -2.0*wtemp*wtemp;
      wpi = sin(theta);
      wr = 1.0;
      wi = 0.0;
      for (m=1; m < mmax; m += 2) {
         for (i=m; i <= n; i += istep) {
            j=i+mmax;
            tempr = wr*complex_data[j-1] - wi*complex_data[j];
            tempi = wr * complex_data[j] + wi*complex_data[j-1];

            complex_data[j-1] = complex_data[i-1] - tempr;
            complex_data[j] = complex_data[1] - tempi;
            complex_data[i-1] += tempr;
            complex_data[i] += tempi;
         }
         wr=(wtemp=wr)*wpr-wi*wpi+wr;
         wi=wi*wpr+wtemp*wpi+wi;
      }
      mmax=istep;
   }
}
*/

void usage() {
   cout << "usage: ./a.out filename.wav bpm" << endl;
}

int main( int argc, char *argv[] )
{
   glutInit( &argc, argv );
   glutInitWindowPosition( 20, 20 );
   glutInitWindowSize( 800, 800 );
   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   glutCreateWindow("My Fragment shader");
   glutReshapeFunc( ReshapeGL );
   glutDisplayFunc( Draw );
   glutKeyboardFunc( keyboard );
   glutTimerFunc(20, flock,0);
   Initialize();



   if(argc != 3) {
      usage();
      return EXIT_FAILURE;
   }

   char* filename = argv[1];
   initMusic(filename);

   bpm = atoi(argv[2]);
   interval = calculate_interval(bpm);
   cout << "interval = " << interval << endl;

   cout << "bpm = " << bpm << endl;

   SF_INFO sfinfo;
   //sfinfo.format = 0;

   if(! (infile = sf_open(filename, SFM_READ, &sfinfo))) {
      cout << "unable to open input file" << endl;
   } else {
      cout << "frames: " << sfinfo.frames << endl;
      cout << "samplerate: " << sfinfo.samplerate << endl;
      cout << "channels: " << sfinfo.channels << endl;
      channels = sfinfo.channels;
   }
   initFFT();

   if(channels > 2) {
      cout << "Audio file contains more than 2 channels. Only support 1 and 2 channel .wav" << endl;
      return EXIT_FAILURE;
   }

   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"Lab4_vert.glsl"), textFileRead((char *)"Lab4_frag.glsl"))) {
      printf("Error installing shader!\n");
      return 0;
   }

   InitGeom();
   glutMainLoop();
   return 0;
}
