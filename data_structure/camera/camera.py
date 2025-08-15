# 将给定的 3D bbox（以物体局部坐标给出）用 lf_center_pose（物体在相机坐标系下的位姿）投影并渲染到画布上。
# 说明与假设：
# - 我把 bbox_3d 视为物体坐标系下的边界框（xmin/xmax 等以米为单位，中心在物体局部原点）。
# - lf_center_pose 被视为物体相对于相机的位姿（即把物体点变到相机系的公式 X_cam = R * X_obj + t）。
# - 使用之前会话中提供的相机内参与畸变参数（K 与 dist），图像大小 1920x1536。
# - 如果某条边的两个端点之一在相机后面（Z <= 0），那条边会被跳过（不画）。
# - 绘图使用 matplotlib（不显式指定颜色或样式，遵守绘图规则）。

import numpy as np
import matplotlib.pyplot as plt

# ---------- 输入（来自用户） ----------
bbox = {
    'xmin': -0.08183176275951445,
    'ymin': -0.0899729662970937,
    'zmin': -0.14916972219209595,
    'xmax':  0.08183176275951445,
    'ymax':  0.0899729662970937,
    'zmax':  0.14916972219209595
}

# 物体在相机坐标系下的位姿（object->camera）
pose = {
    'x': -0.18108416169142283,
    'y': -0.012306244017827397,
    'z': 0.7101918256982318,
    'qw': 0.6435301921963487,
    'qx': 0.48145686540413013,
    'qy': -0.11432186181710202,
    'qz': -0.5839509314982139
}

# 使用会话中之前给的相机参数（intrinsics + distortion + size）
K = np.array([
    [516.6035264326025, 0.0, 963.7518583826717],
    [0.0, 517.6320833280236, 763.0190278432547],
    [0.0, 0.0, 1.0]
])
dist = np.array([0.13124165752545514, -0.039745948073131435, 0.004241460834720791, -0.0008143526225479315])
width, height = 1920, 1536

# ---------- 旋转矩阵转换函数 ----------
def quat_to_rotmat(qw, qx, qy, qz):
    qw, qx, qy, qz = np.array([qw, qx, qy, qz], dtype=float)
    n = np.linalg.norm([qw,qx,qy,qz])
    if n == 0:
        raise ValueError("zero-length quaternion")
    qw, qx, qy, qz = qw/n, qx/n, qy/n, qz/n
    R = np.array([
        [1 - 2*(qy**2 + qz**2),     2*(qx*qy - qz*qw),     2*(qx*qz + qy*qw)],
        [    2*(qx*qy + qz*qw), 1 - 2*(qx**2 + qz**2),     2*(qy*qz - qx*qw)],
        [    2*(qx*qz - qy*qw),     2*(qy*qz + qx*qw), 1 - 2*(qx**2 + qy**2)]
    ])
    return R

def project_points_camera(pts_cam, K, dist):
    """
    处理畸变和投影，返回像素坐标和是否有效的布尔掩码。
    """
    X = pts_cam[0,:]; Y = pts_cam[1,:]; Z = pts_cam[2,:]
    # 筛选有效点
    valid = Z > 1e-6
    # 归一化
    x = X[valid] / Z[valid]
    y = Y[valid] / Z[valid]
    k1, k2, p1, p2 = dist[0], dist[1], dist[2], dist[3]
    r2 = x**2 + y**2
    radial = 1 + k1*r2 + k2*(r2**2)  # k3=0
    x_dist = x*radial + 2*p1*x*y + p2*(r2 + 2*x**2)
    y_dist = y*radial + p1*(r2 + 2*y**2) + 2*p2*x*y
    fx = K[0,0]; fy = K[1,1]; cx = K[0,2]; cy = K[1,2]
    u = fx * x_dist + cx
    v = fy * y_dist + cy
    # Build full arrays with NaN for invalid points to keep indices aligned
    u_full = np.full(pts_cam.shape[1], np.nan)
    v_full = np.full(pts_cam.shape[1], np.nan)
    u_full[valid] = u
    v_full[valid] = v
    return u_full, v_full, valid

# ---------- 构造 bbox 的 8 个角点（物体局部坐标） ----------
xmin, ymin, zmin = bbox['xmin'], bbox['ymin'], bbox['zmin']
xmax, ymax, zmax = bbox['xmax'], bbox['ymax'], bbox['zmax']

corners_obj = np.array([
    [xmin, ymin, zmin],
    [xmax, ymin, zmin],
    [xmax, ymax, zmin],
    [xmin, ymax, zmin],
    [xmin, ymin, zmax],
    [xmax, ymin, zmax],
    [xmax, ymax, zmax],
    [xmin, ymax, zmax],
]).T  # shape 3x8

# ---------- 将物体点变换到相机坐标系（假定 pose 表示 object->camera） ----------
R = quat_to_rotmat(pose['qw'], pose['qx'], pose['qy'], pose['qz'])
t = np.array([pose['x'], pose['y'], pose['z']]).reshape(3,1)

corners_cam = (R @ corners_obj) + t  # 3x8

# ---------- 投影到像素平面 ----------
u_full, v_full, valid_mask = project_points_camera(corners_cam, K, dist)

# ---------- 定义立方体 12 条边（以角点索引表示） ----------
edges = [
    (0,1),(1,2),(2,3),(3,0),  # bottom face (zmin)
    (4,5),(5,6),(6,7),(7,4),  # top face (zmax)
    (0,4),(1,5),(2,6),(3,7)   # vertical edges
]

# ---------- 绘制 ----------
fig, ax = plt.subplots(figsize=(10,8))
# 白画布背景
img = np.ones((height, width, 3), dtype=np.uint8) * 255
ax.imshow(img, origin='upper')

# 绘制角点（可见的）
ax.scatter(u_full, v_full, s=20)

# 绘制每条边：仅在两端点均在相机前方（valid）时绘制
for (i,j) in edges:
    if valid_mask[i] and valid_mask[j]:
        ui, vi = u_full[i], v_full[i]
        uj, vj = u_full[j], v_full[j]
        ax.plot([ui, uj], [vi, vj])  # 使用默认样式

# 显示相机坐标系下物体中心深度等信息作为注释
center_cam = (R @ np.array([[0],[0],[0]])) + t
ax.annotate(f"depth={center_cam[2,0]:.3f} m", xy=(20,30), xycoords='axes pixels')

ax.set_xlim(0, width)
ax.set_ylim(height, 0)
ax.set_title("Projected 3D bbox onto image plane")
ax.set_xlabel("u (pixels)")
ax.set_ylabel("v (pixels)")
ax.set_aspect('equal', adjustable='box')
plt.tight_layout()
plt.show()

# ---------- 打印调试信息 ----------
print("Corners in camera coords (X,Y,Z):")
for i in range(corners_cam.shape[1]):
    Xc, Yc, Zc = corners_cam[:,i]
    print(f" {i}: X={Xc:.4f}, Y={Yc:.4f}, Z={Zc:.4f}, projected u={u_full[i]}, v={v_full[i]}, visible={valid_mask[i]}")

