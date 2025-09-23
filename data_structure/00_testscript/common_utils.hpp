#ifndef COMMON_UTILS_HPP
#define COMMON_UTILS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <Eigen/Dense>
#include <thread>
#include <yaml-cpp/yaml.h>
#include <opencv2/core/eigen.hpp>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <chrono>


struct Pose3D {
    Eigen::Vector3d position;      // (x, y, z)
    Eigen::Quaterniond orientation; // 四元数 (w, x, y, z)
};



struct CameraCalibration {
    std::string topic;
    // 外参 旋转+平移
    Pose3D ext;
    // 内参 矩阵+畸变
    std::vector<double> cameraIntMat;
    Eigen::VectorXd distCoeff;
    // 像素尺寸
    std::vector<int> imageSize;

    double fovHorizontal;
    double fovVertical;
    bool isFisheye;
};

class Utility {
public:

    static void executeShell(const std::string& command_str) {
        // 执行shell命令
        std::cout << "Executing shell command: " << command_str << std::endl;
        std::string command = "/bin/bash -c \""+ command_str + "\"";
        std::thread([command]() {
            int result = system(command.c_str());
            int exit_code = WEXITSTATUS(result);
            if (exit_code != 0) {
                std::cerr << "Command failed with exit code: " << exit_code << std::endl;
            }
        }).detach();
    }

    static CameraCalibration parseCamera(const YAML::Node& root, const std::string& camera_topic) {
        // 解析标定文件的相机参数
        CameraCalibration cam;
        auto cameras = root["sensors"]["camera"]; 
        for (const auto &cameraNode : cameras) {
            cam.topic = cameraNode["topic"].as<std::string>();
            if (cam.topic!= camera_topic) {
                std::cout << "Camera topic: " << cam.topic << " not match with " << camera_topic << std::endl;
                continue;
            }
            std::cout << "Camera topic: " << cam.topic << " found" << std::endl;
            auto ext = cameraNode["calibration"]["Ext"];
            cam.ext.position.x() = ext["Translation"][0].as<double>();
            cam.ext.position.y() = ext["Translation"][1].as<double>();
            cam.ext.position.z() = ext["Translation"][2].as<double>();

            // todo 明确四元数的顺序
            cam.ext.orientation.x() = ext["Rotation"][0].as<double>();
            cam.ext.orientation.y() = ext["Rotation"][1].as<double>();
            cam.ext.orientation.z() = ext["Rotation"][2].as<double>();
            cam.ext.orientation.w() = ext["Rotation"][3].as<double>();

            cam.cameraIntMat.clear();
            auto intMatNode = cameraNode["calibration"]["IntMat"];
            for (size_t i = 0; i < intMatNode.size(); ++i) {
                cam.cameraIntMat.push_back(intMatNode[i].as<double>());
            }
            
            cam.distCoeff   = Eigen::Map<Eigen::VectorXd>(cameraNode["calibration"]["DistCoeff"].as<std::vector<double>>().data(), cameraNode["calibration"]["DistCoeff"].as<std::vector<double>>().size());
            cam.imageSize   = cameraNode["calibration"]["ImageSize"].as<std::vector<int>>();

            cam.fovHorizontal = cameraNode["calibration"]["FovHorizontal"].as<int>();
            cam.fovVertical   = cameraNode["calibration"]["FovCertical"].as<int>();
        }
        return cam;
    }

    
    
    inline double coef(const Eigen::VectorXd& v, int i) {
        // 安全取系数
        return (i < v.size()) ? v[i] : 0.0;
    }

    
    inline Eigen::Vector2d distortNormalizedPoint(const Eigen::Vector2d& xy, const Eigen::VectorXd& d) {
        // 对归一化坐标应用畸变
        const double x = xy.x();
        const double y = xy.y();
        const double r2 = x*x + y*y;
        const double r4 = r2*r2;
        const double r6 = r4*r2;

        const double k1 = coef(d,0), k2 = coef(d,1), k3 = coef(d,4);
        const double k4 = coef(d,5), k5 = coef(d,6), k6 = coef(d,7);
        const double p1 = coef(d,2), p2 = coef(d,3);
        const double s1 = coef(d,8), s2 = coef(d,9), s3 = coef(d,10), s4 = coef(d,11);

        double radial = (1 + k1*r2 + k2*r4 + k3*r6) / (1 + k4*r2 + k5*r4 + k6*r6);

        Eigen::Vector2d tanDist;
        tanDist.x() = 2*p1*x*y + p2*(r2 + 2*x*x);
        tanDist.y() = p1*(r2 + 2*y*y) + 2*p2*x*y;

        Eigen::Vector2d prismDist;
        prismDist.x() = s1*r2 + s2*r4;
        prismDist.y() = s3*r2 + s4*r4;

        return xy * radial + tanDist + prismDist;
    }

    
    static inline std::vector<double> pose3DTo2D(const Eigen::Vector3d& Pw, const CameraCalibration& cam) {
    //Pw chest坐标系下的点
    std::cout<<"cam.cameraIntMat.size() = " << cam.cameraIntMat.size()<<std::endl;
    for(auto i:cam.cameraIntMat){
        std::cout<<i<<" ";
    }
    if (cam.cameraIntMat.size() != 9) throw std::runtime_error("IntMat must have 9 elements");

    // 1. 四元数 -> 旋转矩阵
    Eigen::Quaterniond q(cam.ext.orientation.w(), cam.ext.orientation.x(),
                         cam.ext.orientation.y(), cam.ext.orientation.z());
    Eigen::Matrix3d R = q.toRotationMatrix();

    // 2. chest坐标系 -> 相机坐标系
    Eigen::Vector3d Pc = R.transpose() * (Pw - cam.ext.position);
    // todo 需要检查z轴的值
    // if (Pc.z() <= 0.0) throw std::runtime_error("Point is behind the camera (Zc <= 0).");

    // 3. 转为 OpenCV 类型
   std::vector<cv::Point3f> objectPoints;
    objectPoints.push_back(cv::Point3f(Pc.x(), Pc.y(), Pc.z()));

    // 内参矩阵
    cv::Mat K = (cv::Mat_<double>(3,3) <<
                 cam.cameraIntMat[0], cam.cameraIntMat[1], cam.cameraIntMat[2],
                 cam.cameraIntMat[3], cam.cameraIntMat[4], cam.cameraIntMat[5],
                 cam.cameraIntMat[6], cam.cameraIntMat[7], cam.cameraIntMat[8]);

    // 畸变系数
    cv::Mat dist;
    if (cam.isFisheye) {
        dist = cv::Mat(cam.distCoeff.size(), 1, CV_64F);
        for (int i=0; i<cam.distCoeff.size(); i++) dist.at<double>(i) = static_cast<double>(cam.distCoeff[i]);
    } else {
        dist = cv::Mat(cam.distCoeff.size(), 1, CV_64F);
        for (int i=0; i<cam.distCoeff.size(); i++) dist.at<double>(i) = static_cast<double>(cam.distCoeff[i]);
    }

    // 外参旋转和平移向量
    cv::Mat rvec, tvec;
    cv::Mat cvR;
    cv::eigen2cv(R, cvR);
    cv::Rodrigues(cvR, rvec);          // 3x3 -> rvec
    tvec = (cv::Mat_<double>(3,1) << cam.ext.position.x(), cam.ext.position.y(), cam.ext.position.z());

    // 输出像素点
    std::vector<cv::Point2f> imagePoints;

    // 在调用前添加类型检查
    std::cout << "objectPoints type: " << objectPoints.size() << " (CV_64FC3=" << CV_64FC3 << ")" << std::endl;
    std::cout << "rvec type: " << rvec.type() << " (CV_64FC1=" << CV_64FC1 << ")" << std::endl;
    std::cout << "tvec type: " << tvec.type() << " (CV_64FC1=" << CV_64FC1 << ")" << std::endl;
    std::cout << "K type: " << K.type() << " (CV_64FC1=" << CV_64FC1 << ")" << std::endl;
    std::cout << "dist type: " << dist.type() << " (CV_64FC1=" << CV_64FC1 << ")" << std::endl;

    if (cam.isFisheye) {
        cv::fisheye::projectPoints(objectPoints, imagePoints, rvec, tvec, K, dist);
    } else {
        cv::projectPoints(objectPoints, rvec, tvec, K, dist, imagePoints);
    }

    return {imagePoints[0].x, imagePoints[0].y};
}

    // 转换两个坐标系的坐标
    static Pose3D convertCoordinate(const Pose3D& p1,const Pose3D& p2) {
        // 参数中的p1和p2的坐标都是位于p2坐标系下的,该函数将p2坐标转换到p1坐标系下
        // 构造 p1 的齐次变换矩阵
        Eigen::Matrix4d T_p1_in_p2 = Eigen::Matrix4d::Identity();
        T_p1_in_p2.block<3,3>(0,0) = p1.orientation.toRotationMatrix();
        T_p1_in_p2.block<3,1>(0,3) = p1.position;

        // 构造 p2 的齐次矩阵（在自己坐标系下就是单位矩阵加平移）
        Eigen::Matrix4d T_p2_in_p2 = Eigen::Matrix4d::Identity();
        T_p2_in_p2.block<3,3>(0,0) = p2.orientation.toRotationMatrix();
        T_p2_in_p2.block<3,1>(0,3) = p2.position;

        // 求 p2 在 p1 坐标系下
        Eigen::Matrix4d T_p2_in_p1 = T_p1_in_p2.inverse() * T_p2_in_p2;

        Eigen::Vector3d p2_in_p1_pos = T_p2_in_p1.block<3,1>(0,3);
        Eigen::Matrix3d R_p2_in_p1 = T_p2_in_p1.block<3,3>(0,0);
        Eigen::Quaterniond p2_in_p1_quat(R_p2_in_p1);

        Pose3D p2_in_p1;
        p2_in_p1.position = p2_in_p1_pos;
        p2_in_p1.orientation = p2_in_p1_quat;

        return p2_in_p1;
    }
    
    static inline Eigen::Matrix4d makeT(const Eigen::Matrix3d& R, const Eigen::Vector3d& t) {
    // 构造齐次变换矩阵，R,t为某个定位点(tracker)在世界坐标系下的位姿；返回值T.inverse()即为世界坐标系到tracker坐标系的变换矩阵
    // 比如 a（1，2，1，1）为世界坐标系下的点（齐次坐标），T.inverse() * a即为tracker坐标系下的点
        Eigen::Matrix4d T = Eigen::Matrix4d::Identity();
        T.block<3,3>(0,0) = R;
        T.block<3,1>(0,3) = t;
        return T.inverse();
    }

};
#endif // COMMON_UTILS_HPP