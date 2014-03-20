#include "Boid.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

using namespace std;


//constructor for boid
Boid::Boid(int index) {

   this->index = index;
   srand(time(NULL));
   float angle = 0 + (static_cast <float> (rand())) /
                     (static_cast <float> (RAND_MAX/(6.28)));

   position     = PVector(0, 0, 0);
   velocity     = PVector (cos(angle), sin(angle), 0);
   acceleration = PVector (0, 0, 0);

   maxspeed = 2.0;
   maxforce = 0.03;
}

Boid::Boid(int index, double x, double y, double z, float angle) {
   this->index = index;
   /*
   srand((unsigned)time(NULL));
   float angle = 0 + (static_cast <float> (rand())) /
                     (static_cast <float> (RAND_MAX/(6.28)));
*/
   position     = PVector(x, y, z);
   velocity     = PVector (cos(angle), sin(angle), 0);
   acceleration = PVector (0, 0, 0);

   maxspeed = 2.0;
   maxforce = 0.03;
}

//update location of each boid based on other boids in flock
void Boid::run(std::vector<Boid *> boids, double  sep_weight, double coh_weight) {
   flock(boids, sep_weight, coh_weight);
   update();
   borders();
}

//Accumulate a new acceleration each time based on three rules
void Boid::flock(vector<Boid*> boids, double sep_weight, double coh_weight) {
   PVector sep = separate(boids); //separate
   PVector ali = align(boids);    //alignment
   PVector coh = cohesion(boids); //cohesion

   sep = sep.mult(sep_weight);
   ali = ali.mult(1.0);
   coh = coh.mult(coh_weight);

   applyForce(sep);
   applyForce(ali);
   applyForce(coh);;
}

//apply a force current velociy
void Boid::applyForce(PVector force) {
   acceleration = acceleration.add(force);
}

//Method to update location
void Boid::update() {
   velocity = velocity.add(acceleration);
   velocity.limit(maxspeed);
   this->position = position.add(velocity);
   acceleration = acceleration.mult(0);
}

void Boid::borders() {
   if(position.x < -50.0) position.x = 50.0;
   if(position.y < -50.0) position.y = 50.0;
   if(position.x > 50.0) position.x = -50.0;
   if(position.y > 50.0) position.y = -50.0;
}

//keep a small distance from neighboring boids
PVector Boid::separate(vector<Boid *> boids) {
   float separation = 7;
   PVector v = PVector(0, 0, 0);
   int count = 0;

   for(int i = 0; i < boids.size(); i++) {
      Boid *b = boids.at(i);
      float d = PVector::dist(position, b->position);

      if((d>0) && (d < separation)) {
         PVector diff = PVector::sub(position, b->position);
         diff.normalize();
         diff = diff.div(d);
         v = v.add(diff);
         count++;
      }
   }

   if(count > 0) {
      v = v.div((float)count);
   }

   if(v.mag() > 0) {
      v.normalize();
      v = v.mult(maxspeed);
      v = v.sub(velocity);
      v.limit(maxforce);
   }


   return v;
}

//calculate velocity of near by boids
PVector Boid::align(vector<Boid *> boids) {
   float neighbordist = 10;
   PVector sum = PVector(0,0,0);
   int count = 0;
   for(int i = 0; i < boids.size(); i++) {
      Boid *b = boids.at(i);
      float d = PVector::dist(position, b->position);
      if((d > 0) && (d < neighbordist)) {
         sum = sum.add(b->velocity);
         count++;
      }
   }
   if(count > 0) {
      sum = sum.div((float)count);
      sum.normalize();
      PVector steer = PVector::sub(sum, velocity);
      steer.limit(maxforce);
      return steer;
   } else {
      return PVector(0, 0);
   }
}

//boids fly towards center of mass
PVector Boid::cohesion(vector<Boid *> boids) {
   float neighbordist = 25;
   PVector sum = PVector(0,0,0);
   int count = 0;
   for(int i = 0; i < boids.size(); i++) {
      Boid *b = boids.at(i);
      float d = PVector::dist(position, b->position);
      if((d> 0) && (d < neighbordist)) {
         sum = sum.add(b->position);
         count++;
      }
   }
   if (count > 0) {
      sum = sum.div(count);
      PVector steer = seek(sum);
      return steer;
   } else {
      return PVector(0, 0, 0);
   }
}

//calculate a force towards a target
PVector Boid::seek(PVector target) {
   PVector desired = PVector::sub(target, position);
   desired.normalize();
   desired = desired.mult(maxspeed);

   PVector steer = PVector::sub(desired, velocity);
   steer.limit(maxforce);
   return steer;
}




//print poid data
void Boid::print() {
   cout << "Position:     ";
   cout << "(" << position.x << ","
               << position.y << ","
               << position.z << ")" << endl;
   cout << "Velocity:     ";
   cout << "(" << velocity.x << ","
               << velocity.y << ","
               << velocity.z << ")" << endl;
   cout << "Acceleration: ";
   cout << "(" << acceleration.x << ","
               << acceleration.y << ","
               << acceleration.z << ")" << endl;
}
