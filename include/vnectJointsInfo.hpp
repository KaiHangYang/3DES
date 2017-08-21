#ifndef VNECT_JOINT_INFO
#define VNECT_JOINT_INFO
#include <vector>
#include <string>

namespace VNect {
    const int original_joint_idx[] = {10, 13, 16, 19, 22, 25, 28, 29, 31, 36, 40, 42, 43, 45, 50, 54, 56, 57,  63, 64, 69, 70, 71, 77, 78, 83, 84, 85};
    const std::vector<std::string> original_joint_names({
            "spine3", "spine4", "spine2", "spine1", "spine",     
            "neck", "head", "head_top", "left_shoulder", "left_arm", "left_forearm",
            "left_hand", "left_hand_ee",  "right_shoulder", "right_arm", "right_forearm", "right_hand",
            "right_hand_ee", "left_leg_up", "left_leg", "left_foot", "left_toe", "left_ee",
            "right_leg_up" , "right_leg", "right_foot", "right_toe", "right_ee"
            });
    const std::vector<std::string> all_joint_names({
            "spine3", "spine4", "spine2", "spine", "pelvis",
            "neck", "head", "head_top", "left_clavicle", "left_shoulder", "left_elbow",
            "left_wrist", "left_hand",  "right_clavicle", "right_shoulder", "right_elbow", "right_wrist",
            "right_hand", "left_hip", "left_knee", "left_ankle", "left_foot", "left_toe",
            "right_hip" , "right_knee", "right_ankle", "right_foot", "right_toe"
            });
    
};

#endif
