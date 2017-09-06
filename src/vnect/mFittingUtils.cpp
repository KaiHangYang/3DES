#include <stdlib.h>
#include <string.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../include/mFittingUtils.hpp"
#include "../../include/vnectJointsInfo.hpp"
#include "../../include/mDefs.h"
#include "../../include/mTimeCount.hpp"


namespace mFitting {
    // This may be implemented by lossfunction
    const double m_fitting_w1 = 1;
    const double m_fitting_w2 = 44;
    const double m_fitting_w3 = 0.07;
    const double m_fitting_w4 = 0.11;
    
    double base_plane_height =  5.0 * glm::tan(glm::radians(base_vof/2));
    double base_plane_width = ratio_w * base_plane_height / ratio_h;

    // To minimize the computing time
    const int point_to_bone_start[] = {8, 8, 8, 8, 8, 8, 8, 8, 0, 0, 0, 4, 4, 4, -1, 8, 8, 8, 8, 0, 4}; // 20
    const int point_to_bone_end[] = {19, 9, 10, 11, 12, 14, 15, 16, 0, 1, 2, 4, 5, 6, -1, 8, 18, 13, 17, 3, 7};
    const int point_to_bone_mid1[] = {9, -1, -1, -1, -1, 9, 9, 9, -1, -1, -1, -1, -1, -1, -1, -1, 9, -1, 9, -1, -1};
    const int point_to_bone_mid2[] = {18, -1, -1, -1, -1, 14, 14, 14, -1, -1, -1, -1, -1, -1, -1, -1, 18, -1, 14, -1, -1};
    // Emmmm...., I am probably using a fk(forward kenimatic) model. It doesn't matter.
    struct EIKError {
        EIKError(double pl_x, double pl_y, double pl_z, int num): pl_x(pl_x), pl_y(pl_y), pl_z(pl_z), num(num){};
        
        template<typename T> bool operator() (const T * const theta, const T * const d, T * residuals) const {
            // theta is array(60) d is array(3) residuals is array(3)
            // For every Pl point it need such a cost function
            T global_3d[joint_num*3];
            // get the 3D location from d and theta with a fixed bone length
            
            cal_3djoints(theta, d, global_3d, num);
            // the minus the 
            global_3d[num*3] -= d[0];
            global_3d[num*3 + 1] -= d[1];
            global_3d[num*3 + 2] -= d[2];
            // then get the resisual 
            residuals[0] = T(m_fitting_w1) * (T(global_3d[num*3]) - T(pl_x));
            residuals[1] = T(m_fitting_w1) * (T(global_3d[num*3 + 1]) - T(pl_y));
            residuals[2] = T(m_fitting_w1) * (T(global_3d[num*3 + 2]) - T(pl_z));
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
        EPROJError(double * joints_2d, glm::mat4 mvp): joints_2d(joints_2d), MVP(mvp){};
        template<typename T> bool operator() (const T * const theta, const T * const d, T * residuals) const {
            T global_3d[joint_num*3];
            cal_3djoints(theta, d, global_3d, -1);
            // Project the point to the image plane
            T tmp[4];
            T tmp2[4];
            for (int i=0; i < joint_num; ++i) {
                matrix_multi(MVP, T(2 * joints_2d[2*i + 1] * base_plane_width), T(2 * joints_2d[2*i] * base_plane_height), T(0), tmp2);
                // because of the pin hole model
                matrix_multi(MVP, global_3d[3*i], -global_3d[3*i + 1], global_3d[3*i + 2], tmp);
                
                residuals[2*i] = T(m_fitting_w2) * (tmp[0] - tmp2[0]);
                residuals[2*i + 1] = T(m_fitting_w2) * (tmp[1] - tmp2[1]);
            }
            //std::cout << "X: " << tmp[0] << ", " << tmp2[0] << "\tY: " << tmp[1] << ", " << tmp2[1] << "\tZ: " << tmp[2] << ", " << tmp2[2] << "\tW: " << tmp[1] << ", " << tmp2[1] << std::endl;
            return true;
        }
        static ceres::CostFunction * Create(double * joints_2d, glm::mat4 mvp) {
            return (new ceres::AutoDiffCostFunction<EPROJError, 2*joint_num, joint_num*3, 3>(new EPROJError(joints_2d, mvp)));
        }
        double * joints_2d;
        glm::mat4 MVP;
    };
    //// Punish the accelation of the change
    struct ESMOOTHError {
        //ESMOOTHError(std::vector<double> pl_x, std::vector<double> pl_y, std::vector<double> pl_z, int num): pl_x(pl_x), pl_y(pl_y), pl_z(pl_z), num(num) {}
        ESMOOTHError(double pl_x, double pl_y, double pl_z, int num): angle_x(pl_x), angle_y(pl_y), angle_z(pl_z), num(num) {}
        template<typename T> bool operator() (const T * const theta, const T * const d, T * residuals) const {
            //T global_3d[joint_num * 3];
            //cal_3djoints(theta, d, global_3d, num);
            // Cal the acceleration
            //residuals[0] = T(m_fitting_w3)*(T(global_3d[num * 3]) - T(pl_x[0]));
            //residuals[1] = T(m_fitting_w3)*(T(global_3d[num * 3 + 1]) - T(pl_y[0]));
            //residuals[2] = T(m_fitting_w3)*(T(global_3d[num * 3 + 2]) - T(pl_z[0]));
            residuals[0] = T(m_fitting_w3) * (theta[3*num] - T(angle_x));
            residuals[1] = T(m_fitting_w3) * (theta[3*num + 1] - T(angle_y));
            residuals[2] = T(m_fitting_w3) * (theta[3*num + 2] - T(angle_z));
            return true;
        }

        static ceres::CostFunction * Create(const double pl_x, const double pl_y, const double pl_z, int num) {
            return (new ceres::AutoDiffCostFunction<ESMOOTHError, 3, joint_num * 3, 3>(new ESMOOTHError(pl_x, pl_y, pl_z, num)));
        }
        // Store the previous and previous previous data
        //std::vector<double> pl_x;
        //std::vector<double> pl_y;
        //std::vector<double> pl_z;
        double angle_x;
        double angle_y;
        double angle_z;
        int num;
    };
    struct EDEPTHError {
        EDEPTHError(double pl_z, int num): pl_z(pl_z), num(num) {}
        template<typename T> bool operator() (const T * const theta, const T * const d, T * residuals) const {
            T global_3d[joint_num * 3];
            cal_3djoints(theta, d, global_3d, num);
            
            residuals[0] = T(m_fitting_w4)*(T(global_3d[num * 3 + 2]) - T(pl_z));
            return true;
        }

        static ceres::CostFunction * Create(double pl_z, int num) {
            return (new ceres::AutoDiffCostFunction<EDEPTHError, 1, joint_num*3, 3>(new EDEPTHError(pl_z, num)));
        }

        double pl_z;
        int num;
    };
    // mnum < 0 mean calculate all the points
    // otherwise I calculate the certain point to decrease the compute time 
    template<typename T> void cal_3djoints(const T * const angles, const T * const d, T * result, int mnum) {
        // set the root point to d.
        result[3*14] = d[0];
        result[3*14 + 1] = d[1]; 
        result[3*14 + 2] = d[2];
        if (mnum < 0) {
            for (int i=0; i < joint_num - 1; ++i) {
                int from = joint_indics.at(2*i);
                int to = joint_indics.at(2*i+1);
                // Here the "from" point is already known
                result[to*3 + 0] = result[from*3 + 0] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 0]*T(0.00555556));
                result[to*3 + 1] = result[from*3 + 1] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 1]*T(0.00555556));
                result[to*3 + 2] = result[from*3 + 2] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 2]*T(0.00555556));
            }
        }
        else if (mnum != 14) {
            // The number is the joint num
            int start = point_to_bone_start[mnum];
            int end = point_to_bone_end[mnum];
            int mid1 = point_to_bone_mid1[mnum];
            int mid2 = point_to_bone_mid2[mnum]; 
            if (mid1 == -1 && mid2 == -1) {
                for (int i=start; i <= end; ++i) {
                    int from = joint_indics.at(2*i);
                    int to = joint_indics.at(2*i+1);
                    // Here the "from" point is already known
                    result[to*3 + 0] = result[from*3 + 0] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 0]*T(0.00555556));
                    result[to*3 + 1] = result[from*3 + 1] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 1]*T(0.00555556));
                    result[to*3 + 2] = result[from*3 + 2] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 2]*T(0.00555556));
                }
            }
            else {
                for (int i=start; i <= mid1; ++i) {
                    int from = joint_indics.at(2*i);
                    int to = joint_indics.at(2*i+1);
                    // Here the "from" point is already known
                    result[to*3 + 0] = result[from*3 + 0] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 0]*T(0.00555556));
                    result[to*3 + 1] = result[from*3 + 1] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 1]*T(0.00555556));
                    result[to*3 + 2] = result[from*3 + 2] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 2]*T(0.00555556));
                }
                for (int i=mid2; i <= end; ++i) {
                    int from = joint_indics.at(2*i);
                    int to = joint_indics.at(2*i+1);
                    // Here the "from" point is already known
                    result[to*3 + 0] = result[from*3 + 0] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 0]*T(0.00555556));
                    result[to*3 + 1] = result[from*3 + 1] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 1]*T(0.00555556));
                    result[to*3 + 2] = result[from*3 + 2] + joint_bone_length[i] * ceres::cos(T(M_PI) * angles[3*i + 2]*T(0.00555556));
                }
            }
        }
        // Then calculate all the points from the root point.
    }
    template<typename T> void matrix_multi(glm::mat4 mvp, T x, T y, T z, T * tmp) {
        // glm is the col main
        for (int i=0; i < 4; ++i) {
            tmp[i] = T(mvp[0][i]) * x +T(mvp[1][i]) * y + T(mvp[2][i]) * z + T(mvp[3][i]);
        }
        for (int i=0; i < 3; ++i) {
            tmp[i] = tmp[i]/tmp[3];
        }
    }
    void fitting(double ** joints_2d, double ** joints_3d, glm::mat4 &mvp, double ** angles, double *d) {
        ceres::Problem problem;
        //for (int i=0; i < joint_num; ++i) {
            //ceres::CostFunction * e1_cost_function = EIKError::Create(joints_3d[0][3*i], joints_3d[0][3*i + 1], joints_3d[0][3*i + 2], i);
            //ceres::CostFunction * e2_cost_function = EPROJError::Create(2*joints_2d[0][2*i + 1], 2*joints_2d[0][2*i + 0], i, mvp); // cause the 2d is y, x and it's normalized to [-0.5, 0.5]
            ceres::CostFunction * e2_cost_function = EPROJError::Create(joints_2d[0], mvp); // cause the 2d is y, x and it's normalized to [-0.5, 0.5]
            //ceres::CostFunction * e4_cost_function = EDEPTHError::Create(joints_3d[1][3*i + 2], i);
            //problem.AddResidualBlock(e1_cost_function, NULL, angles[0], d);
            problem.AddResidualBlock(e2_cost_function, NULL, angles[0], d);
            //problem.AddResidualBlock(e4_cost_function, NULL, angles[0], d);
            //ceres::CostFunction * e3_cost_function = ESMOOTHError::Create(std::vector<double>({joints_3d[1][3*i], joints_3d[2][3*i]}), std::vector<double>({joints_3d[1][3*i + 1], joints_3d[2][3*i + 1]}), std::vector<double>({joints_3d[1][3*i + 2], joints_3d[2][3*i + 2]}), i);
            //problem.AddResidualBlock(e3_cost_function, NULL, angles, d);
            //if (i != joint_num-1) {
                //ceres::CostFunction * e3_cost_function = ESMOOTHError::Create(angles[0][3*i], angles[0][3*i + 1], angles[0][3*i + 2], i);
                //problem.AddResidualBlock(e3_cost_function, NULL, angles[0], d);
            //}
        //}
        ceres::Solver::Options option;
        option.linear_solver_type = ceres::DENSE_SCHUR;
        option.minimizer_progress_to_stdout = false;
        option.max_num_iterations = 10;
        option.trust_region_strategy_type = ceres::LEVENBERG_MARQUARDT;
        ceres::Solver::Summary summary;
        ceres::Solve(option, &problem, &summary);
    }
}
