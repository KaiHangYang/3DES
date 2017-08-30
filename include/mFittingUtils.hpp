#ifndef FITTING_UTILS
#define FITTING_UTILS
#include <glog/logging.h>
#include <ceres/ceres.h>

// I can call outer functions in the below cost functions 
namespace mFitting {
    extern struct EIKError;
    extern struct EPROJError;
    extern struct ESMOOTHError;
    extern struct EDEPTHError;
    // get the points from the angles and d
    void cal_3djoints(double * angles, double * d, double * result);

}
#endif
