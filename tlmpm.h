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
  string method_type;
  double FLIP;
  string shape_function;

  TLMPM(class MPM *, vector<string>);
  ~TLMPM();

  void setup(vector<string>);

  void compute_grid_weight_functions_and_gradients();
  double (*basis_function)(double, int);
  double (*derivative_basis_function)(double, int, double);
  void particles_to_grid();
  void update_grid_state();
  void grid_to_points();
  void advance_particles();
  void velocities_to_grid();
  void compute_rate_deformation_gradient();
  void update_deformation_gradient();
  void update_stress();
  void adjust_dt();
  void reset();

  int update_wf;
};

double linear_basis_function(double, int);
double derivative_linear_basis_function(double, int, double);
double cubic_spline_basis_function(double, int);
double derivative_cubic_spline_basis_function(double, int, double);
double bernstein_quadratic_basis_function(double, int);
double derivative_bernstein_quadratic_basis_function(double, int, double);


#endif
#endif
