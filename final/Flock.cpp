#include "Flock.h"
#include <stdlib.h>
#include <iostream>

using namespace std;

Flock::Flock() {
}

void Flock::step(double sep_weight, double coh_weight) {
   for(int i = 0; i < boids.size(); i++) {
      Boid *b = boids.at(i);
      b->run(boids, sep_weight, coh_weight);
   }
}

//returns boid at index
Boid* Flock::getBoid(int index) {
   return boids.at(index);
}

//returns number of boids in flock
int Flock::flockSize() {
   return boids.size();
}

//add boid to flock
void Flock::addBoid(Boid *b) {
   boids.push_back(b);
}

//prints position of each boid in flock
void Flock::printFlock() {

   if(boids.size() == 0) {
     cout << "Flock is empty" << endl;
   } else {
      for(int i = 0; i < boids.size(); i++) {
         Boid *b = boids.at(i);
         cout << i << ": Position: (" << b->position.x << ","
                                      << b->position.y << ","
                                      << b->position.z << ")" << endl;
      }
   }

}
