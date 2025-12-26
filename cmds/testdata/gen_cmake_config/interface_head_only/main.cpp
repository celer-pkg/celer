#include <iostream>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>

using namespace Eigen;
using namespace std;

int main()
{
    cout << "=== Eigen库验证示例 ===" << endl;
    
    cout << "\n1. 基础矩阵运算:" << endl;
    
    Matrix3d A;
    A << 1, 2, 3,
         4, 5, 6,
         7, 8, 10;
    
    Matrix3d B = Matrix3d::Random();
    
    cout << "矩阵 A:\n" << A << endl;
    cout << "随机矩阵 B:\n" << B << endl;
    cout << "A + B:\n" << A + B << endl;
    cout << "A * B:\n" << A * B << endl;
    cout << "A的转置:\n" << A.transpose() << endl;
    cout << "A的行列式: " << A.determinant() << endl;
    cout << "A的逆矩阵:\n" << A.inverse() << endl;

    cout << "\n2. 向量运算:" << endl;
    
    Vector3d v(1, 2, 3);
    Vector3d w(4, 5, 6);
    
    cout << "向量 v: " << v.transpose() << endl;
    cout << "向量 w: " << w.transpose() << endl;
    cout << "点积: " << v.dot(w) << endl;
    cout << "叉积: " << v.cross(w).transpose() << endl;
    cout << "向量范数: " << v.norm() << endl;

    cout << "\n3. 线性方程组求解:" << endl;
    
    Vector3d b(3, 3, 4);
    cout << "方程 Ax = b, 其中 b = " << b.transpose() << endl;
    
    Vector3d x1 = A.colPivHouseholderQr().solve(b);
    Vector3d x2 = A.partialPivLu().solve(b);
    
    cout << "QR分解求解 x: " << x1.transpose() << endl;
    cout << "LU分解求解 x: " << x2.transpose() << endl;
    
    cout << "残差 ||Ax - b||: " << (A * x1 - b).norm() << endl;

    cout << "\n4. 特征值和特征向量:" << endl;
    
    EigenSolver<Matrix3d> solver(A);
    cout << "特征值:\n" << solver.eigenvalues() << endl;
    cout << "特征向量:\n" << solver.eigenvectors() << endl;

    cout << "\n5. 奇异值分解:" << endl;
    
    JacobiSVD<Matrix3d> svd(A, ComputeFullU | ComputeFullV);
    cout << "奇异值: " << svd.singularValues().transpose() << endl;
    cout << "左奇异向量 U:\n" << svd.matrixU() << endl;
    cout << "右奇异向量 V:\n" << svd.matrixV() << endl;

    cout << "\n6. 矩阵分块操作:" << endl;
    
    MatrixXd big(4, 4);
    big << 1, 2, 3, 4,
           5, 6, 7, 8,
           9, 10, 11, 12,
           13, 14, 15, 16;
    
    cout << "大矩阵:\n" << big << endl;
    cout << "左上角2x2子矩阵:\n" << big.block<2,2>(0,0) << endl;
    cout << "右下角2x2子矩阵:\n" << big.block<2,2>(2,2) << endl;

    cout << "\n7. 动态大小矩阵:" << endl;
    
    int rows = 3, cols = 2;
    MatrixXd dynamic(rows, cols);
    dynamic << 1, 4,
               2, 5,
               3, 6;
    
    cout << "动态矩阵 (" << rows << "x" << cols << "):\n" << dynamic << endl;
    cout << "矩阵大小: " << dynamic.rows() << " x " << dynamic.cols() << endl;

    return 0;
}