/* -*- c++ -*- ----------------------------------------------------------
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

#ifdef FIX_CLASS

FixStyle(indent/pinball, FixIndentPinball)

#else

#ifndef MPM_FIX_INDENT_PINBALL_H
#define MPM_FIX_INDENT_PINBALL_H

#include "fix.h"
#include "var.h"
#include <vector>

class FixIndentPinball : public Fix {
public:
  FixIndentPinball(class MPM *, vector<string>);
  ~FixIndentPinball();
  void setmask();
  void init();
  void setup();

  void initial_integrate();
  void post_particles_to_grid(){};
  void post_update_grid_state(){};
  void post_grid_to_point(){};
  void post_advance_particles(){};
  void post_velocities_to_grid(){};
  void final_integrate(){};

private:
  string type; // sphere
  string usage =
      "Usage: fix(fix-ID, indent/pinball, group, sphere, K, R, x_center, "
      "y_center, z_center, vx_center, vy_center, vz_center)\n";
  int Nargs = 12;
  int type_pos, Kpos, Rpos, xpos, ypos, zpos, vxpos, vypos,
      vzpos; // Positions of K, the position of the sphere, and
             // its radius in the argument list (args)
};

#endif
#endif

