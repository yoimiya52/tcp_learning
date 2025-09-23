#include <iostream>
#include <vector>
#include <string>
#include <yaml-cpp/yaml.h>
#include <Eigen/Dense>
#include <google/protobuf/text_format.h>
#include "common_utils.hpp"
#include "proto_msgs/HumanoidMotion.pb.h"

struct BoneData {
    int id;
    std::string name;
    Eigen::Vector3d position;      // (x, y, z)
    Eigen::Quaterniond orientation; // 四元数 (w, x, y, z)
    // 默认构造函数
    BoneData() : id(0), name(""), position(Eigen::Vector3d::Zero()), orientation(Eigen::Quaterniond::Identity()) {}
    bool operator==(const BoneData& other) const {
        return id == other.id &&
               name == other.name &&
               position.isApprox(other.position, 1e-8) &&   // Eigen 向量用 isApprox
               orientation.isApprox(other.orientation, 1e-8);
    }
};


class BoneDataManage{
    public:
        BoneDataManage(tars::motive_msgs::BoneDynamicDataList& bone_data_list){
            for(auto& bone_data : bone_data_list.bones()){
                BoneData bone_node;
                bone_node.id = bone_data.id();
                bone_node.name = bone_data.name();
                bone_node.position = Eigen::Vector3d(bone_data.position().x(), bone_data.position().y(), bone_data.position().z());
                bone_node.orientation = Eigen::Quaterniond(bone_data.orientation().w(), bone_data.orientation().x(), bone_data.orientation().y(), bone_data.orientation().z());
                bone_data_list_.push_back(bone_node);
            }
        }
        // 析构函数
        ~BoneDataManage(){
            // 清除bone_data_list_
            bone_data_list_.clear();
        }

        BoneData GetBoneDataByName(std::string bone_name){
            // 确信bone肯定存在
            BoneData bone_data;
            for(auto& bone_node : bone_data_list_){
                if(bone_node.name == bone_name){
                    bone_data=bone_node;
                    bone_data_list_.erase(std::remove(bone_data_list_.begin(), bone_data_list_.end(), bone_node), bone_data_list_.end());
                }
            }
            return bone_data;
        }
        std::vector<BoneData> GetBoneDataList(){
            return bone_data_list_;
        }
        

    private:
        std::vector<BoneData> bone_data_list_;
        
};



// 解析相机函数
CameraCalibration parseCamera(const YAML::Node& root, const std::string& camera_topic) {
    CameraCalibration cam;
    auto cameras = root["sensors"]["camera"]; 
    for (const auto &cameraNode : cameras) {
        cam.topic = cameraNode["topic"].as<std::string>();
        if (cam.topic != camera_topic) {
            continue;
        }

        auto ext = cameraNode["calibration"]["Ext"];
        cam.ext.position.x() = ext["Translation"][0].as<double>();
        cam.ext.position.y() = ext["Translation"][1].as<double>();
        cam.ext.position.z() = ext["Translation"][2].as<double>();

        cam.ext.orientation.x() = ext["Rotation"][0].as<double>();
        cam.ext.orientation.y() = ext["Rotation"][1].as<double>();
        cam.ext.orientation.z() = ext["Rotation"][2].as<double>();
        cam.ext.orientation.w() = ext["Rotation"][3].as<double>();

        // 手动填充 IntMat
        cam.cameraIntMat.clear();
        auto intMatNode = cameraNode["calibration"]["IntMat"];
        for (size_t i = 0; i < intMatNode.size(); ++i) {
            cam.cameraIntMat.push_back(intMatNode[i].as<double>());
        }

        // 填充 DistCoeff
        auto distNode = cameraNode["calibration"]["DistCoeff"];
        std::vector<double> tmpDist;
        for (size_t i = 0; i < distNode.size(); ++i) {
            tmpDist.push_back(distNode[i].as<double>());
        }
        cam.distCoeff = Eigen::Map<Eigen::VectorXd>(tmpDist.data(), tmpDist.size());

        // ImageSize
        auto imgNode = cameraNode["calibration"]["ImageSize"];
        cam.imageSize.clear();
        for (size_t i = 0; i < imgNode.size(); ++i) {
            cam.imageSize.push_back(imgNode[i].as<int>());
        }

        cam.fovHorizontal = cameraNode["calibration"]["FovHorizontal"].as<int>();
        cam.fovVertical   = cameraNode["calibration"]["FovCertical"].as<int>();
    }
    return cam;
};


void HandleGloveAndTrackerData(tars::motive_msgs::BoneDynamicDataList& glove_data, tars::motive_msgs::BoneDynamicDataList& tracker_data, CameraCalibration& camera){
        // 右手
        // 处理glove和tracker的数据，需要完成坐标系的转换
        // 第一步 将glove数据全部转换到LHR-8FFB7245-right坐标系下（通过一个旋转和平移）
        // 第二步 将所有LHR-8FFB7245-right坐标系下的glove数据转到LHR-C8B833F1-chest坐标系下（它们位于steam_VR的世界坐标系下，依靠这个坐标系进行转换）
        // 第三步 将chest的数据转到相机坐标系下（通过相机外参进行转换）
        // 第四步 将相机坐标系的数据转到成像平面坐标系下（通过相机内参进行转换）

        BoneDataManage glove_data_manage(glove_data);
        BoneDataManage tracker_data_manage(tracker_data);

        BoneData right_hand_wrist = glove_data_manage.GetBoneDataByName("RightHand_Hand");
        BoneData right_hand_tracker = tracker_data_manage.GetBoneDataByName("LHR-8FFB7245-right");
        BoneData chest_tracker = tracker_data_manage.GetBoneDataByName("LHR-C8B833F1-chest");

        //1. 计算 手腕 坐标下的转换矩阵，手套骨架节点在手腕坐标系下的坐标
        Eigen::Matrix3d R_world2wrist = right_hand_wrist.orientation.toRotationMatrix();
        // 其他手指节点左乘该矩阵则将坐标转换到手腕坐标下   
        Eigen::Matrix4d T_world2wrist = Utility::makeT(R_world2wrist, right_hand_wrist.position);

        //2. 计算手腕到tracker的转换矩阵(right_hand_wrist->tracker) 标定值待确定
        Eigen::Matrix3d R_wrist2rh_tracker;
        R_wrist2rh_tracker = Eigen::AngleAxisd(M_PI, Eigen::Vector3d::UnitZ()) *   // z 轴 180°
                                     Eigen::AngleAxisd(M_PI, Eigen::Vector3d::UnitY());    // y 轴 180°
        Eigen::Vector3d P_wrist2rh_tracker(-0.035, 0.01, 0.052);
        // wrist 节点坐标左乘该矩阵则将坐标转换到right_tracker坐标下
        Eigen::Matrix4d T_wrist2rh_tracker = Utility::makeT(R_wrist2rh_tracker, P_wrist2rh_tracker);

        //3. 计算right_tracker到世界坐标的转换矩阵(tracker->world)
        Eigen::Matrix3d R_rh_tracker2world = right_hand_tracker.orientation.toRotationMatrix();
        // 世界坐标左乘该矩阵则将坐标转换到right_tracker坐标下,故需要求逆，左乘逆矩阵可以将right_tracker坐标转换到世界坐标
        Eigen::Matrix4d T_rh_tracker2world = Utility::makeT(R_rh_tracker2world, right_hand_tracker.position).inverse();
        // T_rh_tracker2world = T_rh_tracker2world.inverse();

        //4. 计算world到chest坐标系的转换矩阵(world->chest)
        Eigen::Matrix3d R_world2chest = chest_tracker.orientation.toRotationMatrix();
        // 左乘该矩阵可以将世界坐标转换到chest坐标
        Eigen::Matrix4d T_world2chest = Utility::makeT(R_world2chest, chest_tracker.position);
        
        for(const auto& bone_data : glove_data_manage.GetBoneDataList()){
            const Eigen::Vector3d bone_pose_world = bone_data.position;
            Eigen::Vector4d bone_pose(bone_pose_world.x(), bone_pose_world.y(), bone_pose_world.z(), 1.0);
            // 1. 转换到手腕坐标系
            Eigen::Vector4d bone_pose_in_wrist = T_world2wrist * bone_pose;
            // 2. 转换到tracker坐标系
            Eigen::Vector4d bone_pose_in_rh_tracker = T_wrist2rh_tracker * bone_pose_in_wrist;
            // 3. 转换到世界坐标系
            Eigen::Vector4d bone_pose_in_world = T_rh_tracker2world * bone_pose_in_rh_tracker;
            // 4. 转换到chest坐标系
            Eigen::Vector4d bone_pose_in_chest = T_world2chest * bone_pose_in_world;
            // 5. 转换到相机坐标系
            std::vector<double> u_v = Utility::pose3DTo2D(bone_pose_in_chest.head<3>(),camera);
            std::cout << "u_v: " << u_v[0] << " " << u_v[1] << std::endl;
        }
        return;
    }


int main() {
    std::string yaml_file = "/home/tars/projects/platform/DEFAULT_CONFIG/META/calibration_files/calibration_MDC2-UMI.yaml";  // 你保存 YAML 的文件名
    std::string camera_topic = "/sensor_camera_senyun/fpv_lf_fisheye/compressed";

    YAML::Node root = YAML::LoadFile(yaml_file);
    CameraCalibration cam = parseCamera(root, camera_topic);

    std::string text_glove_data = R"pb(
        timestamp: 1756262850535
        bones {
        id: 25
        name: "RightHand_Hand"
        position {
            x: 0.0
            y: 0.0
            z: 0.0
        }
        orientation {
            x: 0.08813947439193726
            y: -0.061528805643320084
            z: 0.9939679503440857
            w: 0.0222302433103323
        }
        }
        bones {
        id: 26
        name: "RightHand_Thumb_Metacarpal"
        position {
            x: -0.011140001937747002
            y: -0.027273092418909073
            z: -0.02151784859597683
        }
        orientation {
            x: 0.0837179571390152
            y: -0.44821158051490784
            z: 0.8430824279785156
            w: -0.2851852774620056
        }
        }
        bones {
        id: 27
        name: "RightHand_Thumb_Proximal"
        position {
            x: -0.04778995364904404
            y: -0.05201830714941025
            z: -0.026613842695951462
        }
        orientation {
            x: 0.08665657043457031
            y: -0.44721460342407227
            z: 0.8427854180335999
            w: -0.2867460548877716
        }
        }
    )pb";

    std::string text_tracker_data = R"pb(
        timestamp: 1756262850533
        bones {
        id: 255
        name: "LHR-8FFB7245-right"
        position {
            x: 0.39305800199508667
            y: -0.8265066742897034
            z: -1.2930546998977661
        }
        orientation {
            x: 0.2307397425174713
            y: 0.6771419048309326
            z: -0.6427468657493591
            w: 0.27406832575798035
        }
        }
        bones {
        id: 255
        name: "LHR-A20BFD8E-horizon"
        position {
            x: 0.6111118793487549
            y: -0.8785986304283142
            z: -1.3961654901504517
        }
        orientation {
            x: 0.7118549942970276
            y: -0.10313613712787628
            z: 0.10837589204311371
            w: 0.6862069964408875
        }
        }
        bones {
        id: 255
        name: "LHR-C8B833F1-chest"
        position {
            x: 0.6241157054901123
            y: -0.5374314188957214
            z: -0.9678712487220764
        }
        orientation {
            x: 0.5643917322158813
            y: 0.14932341873645782
            z: -0.09511077404022217
            w: 0.8062992095947266
        }
        } 
    )pb";

    tars::motive_msgs::BoneDynamicDataList glove_data;
    tars::motive_msgs::BoneDynamicDataList tracker_data;
    
    google::protobuf::TextFormat::ParseFromString(text_glove_data, &glove_data);
    google::protobuf::TextFormat::ParseFromString(text_tracker_data, &tracker_data);

    HandleGloveAndTrackerData(glove_data, tracker_data, cam);




    std::cout << "Camera topic: " << cam.topic << "\n";
    std::cout << "Translation: [" 
              << cam.ext.position.x() << ", "
              << cam.ext.position.y() << ", "
              << cam.ext.position.z() << "]\n";
    std::cout << "Rotation (quaternion wxyz): ["
              << cam.ext.orientation.w() << ", "
              << cam.ext.orientation.x() << ", "
              << cam.ext.orientation.y() << ", "
              << cam.ext.orientation.z() << "]\n";

    std::cout << "IntMat: ";
    for (auto v : cam.cameraIntMat) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "DistCoeff: ";
    for (int i = 0; i < cam.distCoeff.size(); ++i) std::cout << cam.distCoeff[i] << " ";
    std::cout << "\n";

    std::cout << "ImageSize: [" << cam.imageSize[0] << ", " << cam.imageSize[1] << "]\n";
    std::cout << "FOV Horizontal: " << cam.fovHorizontal << ", Vertical: " << cam.fovVertical << "\n";

    return 0;
}
