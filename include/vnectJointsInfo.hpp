#ifndef VNECT_JOINT_INFO
#define VNECT_JOINT_INFO
#include <vector>
#include <string>

const int original_joint_idx[] = {9, 12, 15, 18, 21, 24, 27, 28, 30, 35, 39, 41, 42, 44, 49, 53, 55, 56,  62, 63, 68, 69, 70, 76, 77, 82, 83, 84};
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
// Demo used the joint_parents_o1
const int joint_idx[] = {7, 5, 14, 15, 16, 9, 10, 11, 23, 24, 25, 18, 19, 20, 4, 3, 6, 17, 12, 27, 22};
const int joint_parents_o1[] = {1, 15, 1, 2, 3, 1, 5, 6, 14, 8, 9, 14, 11, 12, 14, 14, 1, 4, 7, 10, 13};
const int joint_parents_o2[] = { 15, 16, 17, 1, 2, 15, 1, 5, 15, 14, 8, 15, 14, 11, 14, 14, 15, 3, 6, 9, 12};
// The bone length, you need to look at the picture to know the position of each element.
// All the length is normalized.
const double joint_bone_length[] = {0.0759187, 0.243122, 0.191611, 0.128607, 0.0759187, 0.243122, 0.191611, 0.128607, 0.12643, 0.137397, 0.0745305, 0.138271, 0.104729, 0.0486109, 0.0745305, 0.138271, 0.126508, 0.0486109, 0.05089, 0.0737413};
const std::vector<unsigned int> joint_indics({14,8,  8,9,  9,10,  10,19,  14,11,  11,12,  12,13,  13,20,  14,15,  15,1,  1,2,  2,3,  3,4,  4,17,  1,5,  5,6,  6,7,  7,18,   1,16,  16,0});
const int joint_num = 21;

#endif
