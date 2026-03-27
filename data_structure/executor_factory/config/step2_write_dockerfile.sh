#!/bin/bash
set -e

echo "[2/5] 创建 Dockerfile 和 Python 文件..."

# WORK_DIR="$(pwd)/app"
# mkdir -p "$WORK_DIR"

# cat <<EOF > "$WORK_DIR/hello.py"
# print("Hello World from Docker + Python!")
# EOF

# cat <<EOF > Dockerfile
# FROM python:3.11-slim

# WORKDIR /app
# COPY app/hello.py .

# CMD ["python3", "hello.py"]
# EOF

# echo "Dockerfile 与 hello.py 已生成。"
