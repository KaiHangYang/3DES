#ifndef FITTING_UTILS
#define FITTING_UTILS
#include <glog/logging.h>
#include <ceres/ceres.h>
#include <glm/glm.hpp>
// I can call outer functions in the below cost functions 
namespace mFitting {
    struct EIKError;
    struct EPROJError;
    struct ESMOOTHError;
    struct EDEPTHError;
    // get the points from the angles and d
    template<typename T> void cal_3djoints(const T * const angles, const T * const d, T * result);
    template<typename T> void matrix_multi(glm::mat4 mvp, T x, T y, T z, T * tmp);
    //void fitting(double * angles, double *d);
}
#endif
