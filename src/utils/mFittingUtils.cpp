#include <stdlib.h>
#include <string.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../include/mFittingUtils.hpp"
#include "../../include/vnectJointsInfo.hpp"

namespace mFitting {
    // Emmmm...., I am probably using a fk(forward kenimatic) model. It doesn't matter.
    struct EIKError {
        EIKError(double pl_x, double pl_y, double pl_z, int num): pl_x(pl_x), pl_y(pl_y), pl_z(pl_z), num(num){};
        
        template<typename T> bool operator() (const T * const theta, const T * const d, T * residuals) const {
            // theta is array(60) d is array(3) residuals is array(3)
            // For every Pl point it need such a cost function
            T global_3d[joint_num*3];
            // get the 3D location from d and theta with a fixed bone length
            
            cal_3djoints(theta, d, global_3d);
            // the minus the 
            global_3d[num*3] -= d[0];
            global_3d[num*3 + 1] -= d[1];
            global_3d[num*3 + 2] -= d[2];
            // then get the resisual 
            residuals[0] = T(global_3d[num*3]) - T(pl_x);
            residuals[1] = T(global_3d[num*3 + 1]) - T(pl_y);
            residuals[2] = T(global_3d[num*3 + 2]) - T(pl_z);
            return true;
        }
        static ceres::CostFunction * Create(const double pl_x, const double pl_y, const double pl_z, int num) {
            return (new ceres::AutoDiffCostFunction<EIKError, 3, joint_num*3, 3>(new EIKError(pl_x, pl_y, pl_z, num)));
        }
        double pl_x;
        double pl_y;
        double pl_z;
        int num;// the number of the point 
    };
    struct EPROJError {
        EPROJError(double kl_x, double kl_y, int num, glm::mat4 mvp): kl_x(kl_x), kl_y(kl_y), num(num), MVP(mvp){};
        template<typename T> bool operator() (const T * const theta, const T * const d, T * residuals) const {
            T global_3d[joint_num*3];
            cal_3djoints(theta, d, global_3d);
            // Project the point to the image plane
            T tmp[4];
            matrix_multi(MVP, global_3d[3*num], global_3d[3*num + 1], global_3d[3*num + 2], tmp);
            residuals[0] = tmp[0] - T(kl_x);
            residuals[1] = tmp[1] - T(kl_y);
            
            return true;
        }

        static ceres::CostFunction * Create(const double kl_x, const double kl_y, int num, glm::mat4 mvp) {
            return (new ceres::AutoDiffCostFunction<EPROJError, 2, joint_num*3, 3>(new EPROJError(kl_x, kl_y, num, mvp)));
        }
        double kl_x;
        double kl_y;
        int num;
        glm::mat4 MVP;
    };
    //// Punish the accelation of the change
    struct ESMOOTHError {
        ESMOOTHError(std::vector<double> pl_x, std::vector<double> pl_y, std::vector<double> pl_z, int num): pl_x(pl_x), pl_y(pl_y), pl_z(pl_z), num(num) {}
        template<typename T> bool operator() (const T * const theta, const T * const d, T * residuals) const {
            T global_3d[joint_num * 3];
            cal_3djoints(theta, d, global_3d);
            // Cal the acceleration
            residuals[0] = T(global_3d[num * 3]) + T(pl_x[1]) - T(2.0) * T(pl_x[0]);
            residuals[1] = T(global_3d[num * 3 + 1]) + T(pl_y[1]) - T(2.0) * T(pl_y[0]);
            residuals[2] = T(global_3d[num * 3 + 2]) + T(pl_z[1]) - T(2.0) * T(pl_z[0]);
            return true;
        }

        static ceres::CostFunction * Create(const std::vector<double> pl_x, const std::vector<double> pl_y, const std::vector<double> pl_z, int num) {
            return (new ceres::AutoDiffCostFunction<ESMOOTHError, 3, joint_num * 3, 3>(new ESMOOTHError(pl_x, pl_y, pl_z, num)));
        }
        // Store the previous and previous previous data
        std::vector<double> pl_x;
        std::vector<double> pl_y;
        std::vector<double> pl_z;
        int num;
    };
    struct EDEPTHError {
        EDEPTHError(double pl_z, int num): pl_z(pl_z), num(num) {}
        template<typename T> bool operator() (const T * const theta, const T * const d, T * residuals) const {
            T global_3d[joint_num * 3];
            cal_3djoints(theta, d, global_3d);
            
            residuals[0] = T(global_3d[num * 3 + 2]) - T(pl_z);
            return true;
        }

        static ceres::CostFunction * Create(double pl_z, int num) {
            return (new ceres::AutoDiffCostFunction<EDEPTHError, 1, joint_num*3, 3>(new EDEPTHError(pl_z, num)));
        }

        double pl_z;
        int num;
    };
    
    template<typename T> void cal_3djoints(const T * const angles, const T * const d, T * result) {
        // set the root point to d.
        result[3*14] = d[0];
        result[3*14 + 1] = d[1]; 
        result[3*14 + 2] = d[2];
        
        // Then calculate all the points from the root point.
        for (int i=0; i < joint_num - 1; ++i) {
            int from = joint_indics.at(2*i);
            int to = joint_indics.at(2*i+1);
            // Here the "from" point is already known
            result[to*3 + 0] = result[from*3 + 0] + joint_bone_length[i] * angles[3*i + 0];
            result[to*3 + 1] = result[from*3 + 1] + joint_bone_length[i] * angles[3*i + 1];
            result[to*3 + 2] = result[from*3 + 2] + joint_bone_length[i] * angles[3*i + 2];
        }
    }
    template<typename T> void matrix_multi(glm::mat4 mvp, T x, T y, T z, T * tmp) {
        // glm is the col main
        for (int i=0; i < 4; ++i) {
            tmp[i] = T(mvp[0][i]) * x +T(mvp[1][i]) * y + T(mvp[2][i]) * z + T(mvp[3][i]);
        }
    }
    void fitting(double ** joints_2d, double ** joints_3d, glm::mat4 &mvp, double * angles, double *d) {
        ceres::Problem problem;
        for (int i=0; i < joint_num; ++i) {
            ceres::CostFunction * e1_cost_function = EIKError::Create(joints_3d[0][3*i], joints_3d[0][3*i + 1], joints_3d[0][3*i + 2], i);
            ceres::CostFunction * e2_cost_function = EPROJError::Create(joints_2d[0][2*i], joints_2d[0][2*i + 1], i, mvp);
            ceres::CostFunction * e3_cost_function = ESMOOTHError::Create(std::vector<double>({joints_3d[1][3*i], joints_3d[2][3*i]}), std::vector<double>({joints_3d[1][3*i + 1], joints_3d[2][3*i + 1]}), std::vector<double>({joints_3d[1][3*i + 2], joints_3d[2][3*i + 2]}), i);
            ceres::CostFunction * e4_cost_function = EDEPTHError::Create(joints_3d[1][3*i + 2], i);
            problem.AddResidualBlock(e1_cost_function, NULL, angles, d);
            problem.AddResidualBlock(e2_cost_function, NULL, angles, d);
            problem.AddResidualBlock(e3_cost_function, NULL, angles, d);
            problem.AddResidualBlock(e4_cost_function, NULL, angles, d);
        }
        ceres::Solver::Options option;
        option.linear_solver_type = ceres::DENSE_SCHUR;
        option.minimizer_progress_to_stdout = true;
        ceres::Solver::Summary summary;
        ceres::Solve(option, &problem, &summary);
    }
}
