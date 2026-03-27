#pragma once

#include <memory>
#include <unordered_map>
#include "script_executor.h"
#include "config_parser.h"

namespace script_manager {

class ScriptFactory {
 public:
  // 初始化工厂：加载所有脚本并创建执行器
  absl::Status Init(const std::vector<ScriptInfo>& scripts);

  // 获取脚本执行器
  std::shared_ptr<BaseScriptExecutor> GetScriptExecutor(const std::string& script_id);

 private:
  // 根据脚本路径/后缀判断类型
  std::string GetScriptType(const ScriptInfo& script_info);

  std::unordered_map<std::string, std::shared_ptr<BaseScriptExecutor>> script_executors_;
};

}  // namespace script_manager