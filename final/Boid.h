#include <iostream>
#include <vector>
#include "pvector.hpp"


class Boid {

   public:

      PVector position;
      PVector velocity;
      PVector acceleration;
      Boid (int index);
      Boid (int index, double, double, double, float);


      void print();
      void run(std::vector<Boid *> boids, double sep_weight, double coh_weight);

   private:
      int index;
      float maxforce;
      float maxspeed;
      void applyForce(PVector force);
      void update();
      void flock(std::vector<Boid *>, double, double);
      void borders();

      PVector seek(PVector target);
      PVector separate(std::vector<Boid *>);
      PVector align(std::vector<Boid *>);
      PVector cohesion(std::vector<Boid *>);


};
