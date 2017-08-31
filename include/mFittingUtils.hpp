#ifndef FITTING_UTILS
#define FITTING_UTILS
#include <glog/logging.h>
#include <ceres/ceres.h>

// I can call outer functions in the below cost functions 
namespace mFitting {
    struct EIKError;
    struct EPROJError;
    struct ESMOOTHError;
    struct EDEPTHError;
    // get the points from the angles and d
    void cal_3djoints(const double * const angles, const double * const d, double * result);
    
    //void fitting(double * angles, double *d);
}
#endif
