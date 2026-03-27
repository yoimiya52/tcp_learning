#include "script_factory.h"
#include "absl/strings/match.h"


namespace script_manager {

absl::Status ScriptFactory::Init(const std::vector<ScriptInfo>& scripts) {
  for (const auto& script : scripts) {
    std::string script_type = GetScriptType(script);
    std::shared_ptr<BaseScriptExecutor> executor;

    if (script_type == "shell") {
      executor = std::make_shared<ShellScriptExecutor>(script);
    } else if (script_type == "dag") {
      executor = std::make_shared<DagScriptExecutor>(script);
    } else {
      return absl::InvalidArgumentError(
          absl::StrCat("Unsupported script type for: ", script.script_id));
    }

    script_executors_[script.script_id] = executor;
  }

  return absl::OkStatus();
}

std::shared_ptr<BaseScriptExecutor> ScriptFactory::GetScriptExecutor(const std::string& script_id) {
  auto it = script_executors_.find(script_id);
  return it != script_executors_.end() ? it->second : nullptr;
}

std::string ScriptFactory::GetScriptType(const ScriptInfo& script_info) {
//   // 根据路径后缀判断类型
  if (absl::EndsWith(script_info.path, ".sh")) {
    return "shell";
  } else if (absl::EndsWith(script_info.path, ".dag")) {
    return "dag";
  }
   return "unknown";
}

}  // namespace script_manager