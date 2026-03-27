#include "config_parser.h"
#include <fstream>
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"

namespace script_manager {

absl::Status ConfigParser::LoadConfigFromFile(const std::string& config_path, 
                                              DeviceScriptConfig& config,
                                              ExecContext& exec_ctx) {
  std::ifstream file(config_path);
  if (!file.is_open()) {
    return absl::NotFoundError(absl::StrCat("Config file not found: ", config_path));
  }

  nlohmann::json config_json;
  try {
    file >> config_json;
  } catch (const nlohmann::json::parse_error& e) {
    return absl::InvalidArgumentError(absl::StrCat("JSON parse error: ", e.what()));
  }

  // 解析设备名
  if (!config_json.contains("device")) {
    return absl::InvalidArgumentError("Missing 'device' in config");
  }
  config.device = config_json["device"];

  // 解析脚本列表
  if (config_json.contains("scripts")) {
    ParseScripts(config_json["scripts"], config.scripts);
  }

  // 解析执行流程
  if (config_json.contains("flow")) {
    ParseFlow(config_json["flow"], config.flow);
  }
  // 初始化执行上下文
  if (config_json.contains("output_file_path")){
    exec_ctx.SetOutputFilePath(config_json["output_file_path"]);
  }
  return absl::OkStatus();
}



void ConfigParser::ParseScripts(const nlohmann::json& json_scripts, 
                                std::vector<ScriptInfo>& scripts) {
  for (const auto& json_script : json_scripts) {
    ScriptInfo script;
    // 兼容script_name/script_id字段
    if (json_script.contains("script_name")) {
      script.script_id = json_script["script_name"];
    } else if (json_script.contains("script_id")) {
      script.script_id = json_script["script_id"];
    }

    if (json_script.contains("path")) {
      script.path = json_script["path"];
    }

    if (json_script.contains("modules")) {
      for (const auto& mod : json_script["modules"]) {
        script.modules.push_back(mod);
      }
    }

    if (json_script.contains("sensor_models")) {
      for (const auto& sensor : json_script["sensor_models"]) {
        script.sensor_models.push_back(sensor);
      }
    }

    scripts.push_back(script);
  }
}

void ConfigParser::ParseFlow(const nlohmann::json& json_flow, 
                             std::vector<FlowNode>& flow) {
  for (const auto& json_node : json_flow) {
    FlowNode node;
    if (json_node.contains("step_type")) {
      node.step_type = json_node["step_type"];
    }

    if (json_node.contains("script_name")) {
      node.script_name = json_node["script_name"];
    }

    if (json_node.contains("condition")) {
      node.condition = json_node["condition"];
    }

    if (json_node.contains("true_script_name")) {
      node.true_script_name = json_node["true_script_name"];
    }

    if (json_node.contains("false_script_name")) {
      node.false_script_name = json_node["false_script_name"];
    }

    if (json_node.contains("async")) {
      node.async = json_node["async"];
    }else{
      node.async = true;
    }

    flow.push_back(node);
  }
}

}  // namespace script_manager