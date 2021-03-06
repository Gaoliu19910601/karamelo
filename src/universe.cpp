/* ----------------------------------------------------------------------
 *
 *                    ***       Karamelo       ***
 *               Parallel Material Point Method Simulator
 * 
 * Copyright (2019) Alban de Vaucorbeil, alban.devaucorbeil@monash.edu
 * Materials Science and Engineering, Monash University
 * Clayton VIC 3800, Australia

 * This software is distributed under the GNU General Public License.
 *
 * ----------------------------------------------------------------------- */

#include <mpi.h>
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include "universe.h"
#include "version.h"
#include "domain.h"
#include "error.h"

using namespace std;

static vector<int> tile2d(int); ///< Determines the tiling of p procs in 2D
static vector<int> tile3d(int); ///< Determines the tiling of p proces in 3D

struct boundsize { 
    double dl;
    int rank;
};

Universe::Universe(MPM *mpm, MPI_Comm communicator) : Pointers(mpm)
{
  uworld = communicator;
  MPI_Comm_rank(uworld,&me);
  MPI_Comm_size(uworld,&nprocs);
}

Universe::~Universe()
{
  // if (uworld != uorig) MPI_Comm_free(&uworld);
}

void Universe::set_proc_grid() {
  cout << "In Universe::set_proc_grid()\n";
  int dim = domain->dimension;

  if (dim != 1 && dim !=2 && dim !=3) {
    error->all(FLERR, "Error in Universe::set_proc_grid(): invalid dimension: " + to_string(dim) + ".\n");
  }

  double *sublo = domain->sublo;
  double *subhi = domain->subhi;
  double *boxlo = domain->boxlo;
  double *boxhi = domain->boxhi;

  procgrid[0] = 1;
  procgrid[1] = 1;
  procgrid[2] = 1;

  myloc[0] = 0;
  myloc[1] = 0;
  myloc[2] = 0;

  if (nprocs > 1 && dim == 1) {
    // Easy bit, all procs are in line:
    procgrid[0] = nprocs;

    myloc[0] = me;
  }

  if (nprocs > 1 && dim == 2) {
    // Determine the smallest dimension:
    double l[2] = {domain->boxhi[0] - domain->boxlo[0],
		   domain->boxhi[1] - domain->boxlo[1]};

    if (l[0]<1.0e-10 || l[1]<1.0e-10) {
      error->all(FLERR, "Error: the domain has a size in at least one direction that is 0: Lx=" + to_string(l[0]) + ", Ly=" + to_string(l[1]) + ".\n");
    }

    // length >= width
    int length = 1; // Which direction represents the domain's length
    int width = 0;  // Which direction represents the domain's width

    if (l[0] > l[1]) {
      width = 1;
      length = 0;
    }

    vector<int> tile = tile2d(nprocs);
    procgrid[width] = tile[0];
    procgrid[length] = tile[1];

    myloc[0] = me % procgrid[0];
    myloc[1] = me / procgrid[0];
  }

  if (nprocs > 1 && dim == 3) {
    // Determine the smallest dimension:
    vector<boundsize> l = {{domain->boxhi[0] - domain->boxlo[0], 0},
			   {domain->boxhi[1] - domain->boxlo[1], 1},
			   {domain->boxhi[2] - domain->boxlo[2], 2}};

    if (l[0].dl<1.0e-10 || l[1].dl<1.0e-10 || l[2].dl<1.0e-10) {
      error->all(FLERR, "Error: the domain has a size in at least one direction that is 0: Lx=" + to_string(l[0].dl) + ", Ly=" + to_string(l[1].dl) + ", Lz=" + to_string(l[2].dl) + ".\n");
    }

    // Sort values in l according to their dl:
    sort(l.begin(), l.end(),
	 [](boundsize const &a, boundsize const &b) { return a.dl < b.dl; });

    vector<int> tile = tile3d(nprocs);
    procgrid[l[0].rank] = tile[0];
    procgrid[l[1].rank] = tile[1];
    procgrid[l[2].rank] = tile[2];
    
    myloc[2] = me / (procgrid[0]*procgrid[1]);
    myloc[1] = me / procgrid[0] - myloc[2]*procgrid[1];
    myloc[0] = me - (myloc[1] + myloc[2]*procgrid[1])*procgrid[0];
  }


  domain->set_local_box();


  procneigh[0][0] = procneigh[0][1] = 0;
  procneigh[1][0] = procneigh[1][1] = 0;
  procneigh[2][0] = procneigh[2][1] = 0;

  if (dim == 1) {
    if (sublo[0] > boxlo[0] + 1.0e-12) procneigh[0][0] = me - 1;
    else procneigh[0][0] = -1;
    if (subhi[0] < boxhi[0] - 1.0e-12) procneigh[0][1] = me + 1;
    else procneigh[0][1] = -1;
  }

  if (dim == 2) {
    if (sublo[0] > boxlo[0] + 1.0e-12)
      procneigh[0][0] = me - 1;
    else
      procneigh[0][0] = -1;
    if (subhi[0] < boxhi[0] - 1.0e-12)
      procneigh[0][1] = me + 1;
    else
      procneigh[0][1] = -1;
    if (sublo[1] > boxlo[1] + 1.0e-12)
      procneigh[1][0] = me - procgrid[0];
    else
      procneigh[1][0] = -1;
    if (subhi[1] < boxhi[1] - 1.0e-12)
      procneigh[1][1] = me + procgrid[0];
    else
      procneigh[1][1] = -1;
  }

  if (dim == 3) {
    if (sublo[0] > boxlo[0] + 1.0e-12)
      procneigh[0][0] = me - 1;
    else
      procneigh[0][0] = -1;
    if (subhi[0] < boxhi[0] - 1.0e-12)
      procneigh[0][1] = me + 1;
    else
      procneigh[0][1] = -1;
    if (sublo[1] > boxlo[1] + 1.0e-12)
      procneigh[1][0] = me - procgrid[0];
    else
      procneigh[1][0] = -1;
    if (subhi[1] < boxhi[1] - 1.0e-12)
      procneigh[1][1] = me + procgrid[0];
    else
      procneigh[1][1] = -1;
    if (sublo[2] > boxlo[2] + 1.0e-12)
      procneigh[2][0] = me - procgrid[0]*procgrid[1];
    else
      procneigh[2][0] = -1;
    if (subhi[2] < boxhi[2] - 1.0e-12)
      procneigh[2][1] = me + procgrid[0]*procgrid[1];
    else
      procneigh[2][1] = -1;
  }

#ifdef DEBUG
    cout << "proc " << universe->me << "\tprocneigh = [[" << procneigh[0][0] << "," << procneigh[0][1] <<"],[" << procneigh[1][0] << "," << procneigh[1][1] <<"],[" << procneigh[2][0] << "," << procneigh[2][1] << "].\n";
#endif
}

vector<int> tile2d(int p) {
  vector<int> result;
  int n = (int) floor(sqrt(p));

  while ((p/n)*n != p) {
    n -= 1;
  }

  result.push_back(n);
  result.push_back(p/n);

  return result;
}


vector<int> tile3d(int p) {
  vector<int> result;

  int n = round(pow(p,1.0/3.0));

  while ((p/n)*n != p) {
    n -= 1;
  }

  result.push_back(n);
  vector<int> ml = tile2d(p/n);

  result.push_back(ml[0]);
  result.push_back(ml[1]);
  sort(result.begin(), result.end());

  return result;
}
