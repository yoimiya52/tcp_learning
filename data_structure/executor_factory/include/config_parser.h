#pragma once

#include "absl/status/status.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include "exec_context.h"


namespace script_manager {

// 脚本配置结构体
struct ScriptInfo {
  std::string script_id;       // 脚本ID/名称
  std::string path;            // 脚本路径
  std::vector<std::string> modules;    // 关联模块
  std::vector<std::string> sensor_models;  // 传感器模型
};

// 流程节点配置
struct FlowNode {
  std::string step_type;       // common/branch
  std::string script_name;     // 普通步骤脚本名
  std::string condition;       // 执行条件
  std::string true_script_name;  // 分支true脚本名
  std::string false_script_name; // 分支false脚本名
  bool async;            // 是否异步执行
};

// 设备脚本总配置
struct DeviceScriptConfig {
  std::string device;          // 设备型号
  std::vector<ScriptInfo> scripts;  // 脚本列表
  std::vector<FlowNode> flow;       // 执行流程
};

// JSON配置解析器
class ConfigParser {
 public:
  // 从文件加载配置
  absl::Status LoadConfigFromFile(const std::string& config_path, 
                                  DeviceScriptConfig& config,
                                  ExecContext& exec_ctx);
  

 private:
  // 解析脚本列表
  void ParseScripts(const nlohmann::json& json_scripts, 
                    std::vector<ScriptInfo>& scripts);
  
  // 解析流程节点
  void ParseFlow(const nlohmann::json& json_flow, 
                 std::vector<FlowNode>& flow);
};

}  // namespace script_manager