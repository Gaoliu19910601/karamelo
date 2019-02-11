/* -*- c++ -*- ----------------------------------------------------------*/

#ifdef METHOD_CLASS

MethodStyle(tlmpm,TLMPM)

#else

#ifndef LMP_TLMPM_H
#define LMP_TLMPM_H

#include "method.h"
#include <vector>
#include <Eigen/Eigen>

class TLMPM : public Method {
 public:

  TLMPM(class MPM *, vector<string>);
  ~TLMPM();

  void setup();
  void compute_grid_weight_functions_and_gradients();
  double spline(double);
  double derivative_spline(double, double);
  void particles_to_grid();
  void update_grid_state();
  void grid_to_points();
  void advance_particles();
  void velocities_to_grid();
  void compute_rate_deformation_gradient();
  void update_deformation_gradient();
  void update_stress();

  int update_wf;
};

#endif
#endif
