#!/bin/bash
set -e

echo "[1/5] 安装 Docker..."

# 等待5秒，模拟安装过程
sleep 5

# if command -v docker &>/dev/null; then
#     echo "Docker 已安装，跳过。"
#     exit 0
# fi

# sudo apt-get remove -y docker docker-engine docker.io containerd runc || true

# sudo apt-get update
# sudo apt-get install -y ca-certificates curl gnupg lsb-release

# sudo install -m 0755 -d /etc/apt/keyrings
# curl -fsSL https://download.docker.com/linux/ubuntu/gpg | \
#     sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg

# echo \
# "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] \
# https://download.docker.com/linux/ubuntu \
# $(lsb_release -cs) stable" | \
# sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# sudo apt-get update
# sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin

# echo "Docker 安装完成。"
