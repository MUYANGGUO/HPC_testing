#include "verlet.h"
#include "cloud.h"

//TODO: add gpu related modification
void
verlet_step_accelerate (int Np, double dt, const double *restrict X[3], double *restrict U[3])
{
    

  for (int i = 0; i < Np; i++) {
    double u[3] = {0.};
    
#pragma omp target
{
  
#pragma omp parallel for
    for (int j = 0; j < Np; j++) {
      if (j != i) {
        double du[3];
        double dx, dy, dz;
  		double qdx, qdy, qdz;
  		double r2, ir3;

        dx = remainder(X[0][i] - X[0][j], 1.);
        dy = remainder(X[1][i] - X[1][j], 1.);
        dz = remainder(X[2][i] - X[2][j], 1.);

        qdx = qdist(dx);
        qdy = qdist(dy);
        qdz = qdist(dz);

        r2 = qdx*qdx + qdy*qdy + qdz*qdz;
        ir3 = 1. / (sqrt(r2) * r2);

        du[0] = dt * ir3 * qdx * dqdist(dx);
        du[1] = dt * ir3 * qdy * dqdist(dy);
        du[2] = dt * ir3 * qdz * dqdist(dz);
          

        for (int d = 0; d < 3; d++) {
          u[d] += du[d];
        }
      }
    }
    for (int d = 0; d < 3; d++) {
      U[d][i] += u[d];
    }
}
  }
}