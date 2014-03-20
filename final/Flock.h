#include "Boid.h"
#include <vector>
class Flock {
   std::vector<Boid *> boids;

   public:

      Flock ();

      void step(double, double);
      void addBoid(Boid *b);
      void printFlock();

      Boid* getBoid(int);
      int flockSize();




};
