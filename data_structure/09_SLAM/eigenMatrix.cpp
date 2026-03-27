#include <iostream>

#include <ctime>
// 矩阵运算的核心库
#include <Eigen/Core>
// 稠密矩阵的代数运算
#include <Eigen/Dense>
// 几何变换
#include <Eigen/Geometry> 
#include <pangolin/pangolin.h>
#include <unistd.h>
#define MATRIX_SIZE 50


using namespace std;
using namespace Eigen;    


string trajectory_file = "/home/tars/projects/tcp_learning/data_structure/09_SLAM/trajectory.txt";
void DrawTrajectory(vector<Isometry3d, Eigen::aligned_allocator<Isometry3d>>);



void aboutEigen(){
    Matrix<float,2,3> matrix_23; // 2行3列的矩阵
    Vector3d v_3d; // 3维向量
    Matrix<float,3,1> vd_3d;

    Matrix3d matrix_33 = Matrix3d::Zero(); // 3行3列的矩阵,初始化为0

    // 动态大小的矩阵
    Matrix<double,Dynamic,Dynamic> matrix_dynamic;

    MatrixXd matrix_x; 

    // 对矩阵的操作
    // 1. 矩阵的初始化
    matrix_23 << 1,2,3,4,5,6;
    cout << "matrix 2*3 from 1 to 6: \n" << matrix_23 << endl;

    // 2. 矩阵数值的访问
    cout << "print matrix 2*3 :"<< endl;
    for(int i=0;i<2;i++){
        for(int j=0;j<3;j++){
            cout << matrix_23(i,j)<< "\t";
        }
         cout << endl;
    }
    // 3. 矩阵乘法
    v_3d << 3,2,1;
    vd_3d << 4,5,6;
    Matrix<double,2,1> result = matrix_23.cast<double>() * v_3d;
    cout << "[1,2,3;4,5,6]*[3;2;1] = " << result.transpose() << endl;

    Matrix<float,2,1> result2 = matrix_23 * vd_3d;
    cout << "[1,2,3;4,5,6]*[4,5,6] = " << result2.transpose() << endl;

    // 4. 矩阵常见运算
    matrix_33 = Matrix3d::Random(); // 随机数矩阵
    cout << "random matrix 3*3: \n" << matrix_33 << endl;
    cout << "transpose: \n" << matrix_33.transpose() << endl; // 转置
    cout << "sum: " << matrix_33.sum() << endl; // 元素和
    cout << "trace: " << matrix_33.trace() << endl; // 迹
    cout << "times 10: \n" << 10*matrix_33 << endl; // 数乘
    cout << "inverse: \n" << matrix_33.inverse() << endl; // 逆矩阵
    cout << "determinant: " << matrix_33.determinant() << endl; // 行列式

    // 5. 特征值与特征向量
    // 实对称矩阵可以保证特征值为实数，特征向量正交，并且一定可以进行相似对角化
    SelfAdjointEigenSolver<Matrix3d> eigen_solver(matrix_33.transpose()*matrix_33);
    cout << "Eigen values = \n" << eigen_solver.eigenvalues() << endl;
    cout << "Eigen vectors = \n" << eigen_solver.eigenvectors() << endl;

    // 6. 求解矩阵方程
    // 求解 matrix_NN * x = v_Nd
    Matrix<double,MATRIX_SIZE,MATRIX_SIZE> matrix_NN = MatrixXd::Random(MATRIX_SIZE,MATRIX_SIZE);

    matrix_NN = matrix_NN * matrix_NN.transpose(); // 保证半正定 
    Matrix<double,MATRIX_SIZE,1> v_Nd = MatrixXd::Random(MATRIX_SIZE,1);
    
    clock_t time_stt = clock(); // 计时
    // 直接求逆
    Matrix<double,MATRIX_SIZE,1> x = matrix_NN.inverse() * v_Nd;
    cout << "time of normal inverse is "<< 1000*(clock()-time_stt)/(double)CLOCKS_PER_SEC << "ms" << endl;
    
    cout << "x = " << x.transpose() << endl;

    // 使用矩阵分解求解
    time_stt = clock();
    // 矩阵分解
    x = matrix_NN.colPivHouseholderQr().solve(v_Nd);
    cout << "time of QR decomposition is "<< 1000*(clock()-time_stt)/(double)CLOCKS_PER_SEC << "ms" << endl;
    cout << "x = " << x.transpose() << endl;
    // 正定矩阵 使用cholesky分解速度最快
    time_stt = clock();
    x = matrix_NN.ldlt().solve(v_Nd);
    cout << "time of LDLT decomposition is "<< 1000*(clock()-time_stt)/(double)CLOCKS_PER_SEC << "ms" << endl;
    cout << "x = " << x.transpose() << endl;
}

void aboutGeometry(){
    // 描述几何变换，旋转和平移
    Matrix3d rotation_matrix = Matrix3d::Identity();
    AngleAxisd rotation_vector(M_PI/4,Vector3d(0,0,1)); // 绕z轴旋转45度
    cout.precision(3);
    cout << "rotation matrix: \n" << rotation_matrix.matrix() << endl;
    rotation_matrix = rotation_vector.toRotationMatrix();
    Vector3d v(1,0,0);
    Vector3d v_rotated = rotation_vector * v;
    cout << "(1,0,0) after rotation (by angle axis) = " << v_rotated.transpose() << endl;

    v_rotated = rotation_matrix * v;

    cout << "(1,0,0) after rotation (by matrix) = " << v_rotated.transpose() << endl;

    // 欧拉角 讲旋转矩阵直接转换成欧拉角
    
    Vector3d euler_angles = rotation_matrix.eulerAngles(2,1,0); // ZYX顺序，即roll pitch yaw
    cout << "yaw pitch roll = " << euler_angles.transpose() << endl;

    // 欧氏变换矩阵使用Eigen::Isometry
    Isometry3d T = Isometry3d::Identity(); // 虽然称为3d，实质上是4x4的矩阵
    T.rotate(rotation_vector); // 按照rotation_vector 进行旋转
    T.pretranslate(Vector3d(1,3,4)); // 平移向量为(1,3,4)

    Vector3d v_transformed = T*v; // R * v + t
    cout << "v transformed = " << v_transformed.transpose()<< endl;

    // 关于仿射变换和射影变换 使用 Eigen::Affine3d 和 Eigen::Projective3d

    // 四元数 直接将AngleAxis赋值给四元数
    Quaterniond q = Quaterniond(rotation_vector);
    cout << "quaternion from rotation vector =" << q.coeffs().transpose() << endl; // coeffs的顺序为(x,y,z,w)
    // 将旋转矩阵赋值给四元数
    q = Quaterniond(rotation_matrix);
    cout << "quaternion from rotation matrix =" << q.coeffs().transpose() << endl; // coeffs的顺序为(x,y,z,w)

    // 四元数的旋转运算
    v_rotated = q * v ;

    cout << "(1,0,0) after quaternion rotation = " << v_rotated.transpose() << endl;

}

void example_about_coordinate_transform(){
    // 关于坐标转换的示例
    // 记世界坐标系为w，两个机器人坐标系为r1，r2,一号机器人的位姿是q1=[0.35,0.2,0.3,0.1],t1=[0.3,0.1,0.1];
    // 二号机器人的位姿是q2=[-0.5,0.4,-0.1,0.2],t2=[-0.1,0.5,0.3];此处q，t表达的是世界坐标系到相机坐标系的变换关系。现在，一号机器人观测到
    // 某个点在自身的坐标系下的坐标为Pr1=[0.5,0,0.2],求该点在二号机器人坐标系下的坐标p2。

    Quaterniond q1(0.35,0.2,0.3,0.1),q2(-0.5,0.4,-0.1,0.2);
    q1.normalize();
    q2.normalize();
    Vector3d t1(0.3,0.1,0.1), t2(-0.1,0.5,0.3);
    Vector3d Pr1(0.5,0,0.2);

    Isometry3d T1w(q1),T2w(q2);
    T1w.pretranslate(t1);
    T2w.pretranslate(t2);

    Vector3d p2 = T2w * T1w.inverse() * Pr1;

    cout << "p2 in robot2 coordinate = " << p2.transpose() << endl;

}

void plot_trajectory(){
    vector<Isometry3d, Eigen::aligned_allocator<Isometry3d>> poses;
    ifstream fin(trajectory_file);
    if(!fin){
        cerr<<"cannot find trajectory file at "<< trajectory_file <<endl;
        return;
    }
    while(!fin.eof()){
        double time,tx,ty,tz,qx,qy,qz,qw;
        fin>>time>>tx>>ty>>tz>>qx>>qy>>qz>>qw;
        Isometry3d Twr(Quaterniond(qw,qx,qy,qz));
        Twr.pretranslate(Vector3d(tx,ty,tz));
        poses.push_back(Twr);
    }
    cout << "read total" << poses.size() << "pose entries" <<endl;
    DrawTrajectory(poses);
}

void DrawTrajectory(vector<Isometry3d, Eigen::aligned_allocator<Isometry3d>> poses){
    // create a pangolin window and plot the trajectory
    pangolin::CreateWindowAndBind("Trajectory Viewer",1024,768);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(1024,768,500,500,512,389,0.1,1000),
        pangolin::ModelViewLookAt(0,-0.1,-0.8,0,0,0,0.0,-1.0,0.0)
    );
    pangolin::View &d_cam = pangolin::CreateDisplay()
       .SetBounds(0.0, 1.0,0.0,1.0,-1024.0f/768.0f)
       .SetHandler(new pangolin::Handler3D(s_cam));

    while(pangolin::ShouldQuit()==false){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glLineWidth(2);
        for(size_t i=0;i<poses.size();i++){
            Vector3d Ow = poses[i].translation();
            Vector3d Xw = poses[i]*(0.1 * Vector3d(1,0,0));
            Vector3d Yw = poses[i]*(0.1 * Vector3d(0,1,0));
            Vector3d Zw = poses[i]*(0.1 * Vector3d(0,0,1));
            glBegin(GL_LINES);
            glColor3f(1.0,0.0,0.0);
            glVertex3d(Ow[0],Ow[1],Ow[2]);
            glVertex3d(Xw[0],Xw[1],Xw[2]);
            glColor3f(0.0,1.0,0.0);
            glVertex3d(Ow[0],Ow[1],Ow[2]);
            glVertex3d(Yw[0],Yw[1],Yw[2]);
            glColor3f(0.0,0.0,1.0);
            glVertex3d(Ow[0],Ow[1],Ow[2]);
            glVertex3d(Zw[0],Zw[1],Zw[2]);
            glEnd();
        }

        for(size_t i=0;i<poses.size();i++){
            glColor3f(0.0,0.0,0.0);
            glBegin(GL_LINES);
            auto p1 = poses[i],p2=poses[i+1];
            glVertex3d(p1.translation()[0],p1.translation()[1],p1.translation()[2]);
            glVertex3d(p2.translation()[0],p2.translation()[1],p2.translation()[2]);
            glEnd();
        }
        pangolin::FinishFrame();
        usleep(5000);
    }

}

int main(int argc, char** argv){
    // 1. 关于Eigen
    aboutEigen();
    // 2. 关于geometry
    aboutGeometry();
    // 3. 关于坐标转换
    example_about_coordinate_transform();
    // 4. 绘制轨迹
    plot_trajectory();
    return 0;
}

