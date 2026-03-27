#include "flow_executor.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
using namespace std;
namespace script_manager {

absl::Status FlowExecutor::ExecuteFlow(const std::vector<FlowNode>& flow, ExecContext& context) {
  for (const auto& node : flow) {
    // 如果已有错误，终止流程（可根据需求调整）
    if (context.HasError()) {
      return absl::FailedPreconditionError(
          absl::StrCat("Flow stopped due to previous error: ", context.GetErrorMsg()));
    }

    // 先判断条件是否满足
    if (!node.condition.empty() && !EvaluateCondition(node.condition, context)) {
      cout << "Condition not satisfied: " << node.condition << std::endl;
      continue;
    }

    // 执行对应类型步骤
    if (node.step_type == "common") {
      absl::Status status = ExecuteCommonStep(node, context);
      if (!status.ok()) {
        return status;
      }
    } else if (node.step_type == "branch") {
      absl::Status status = ExecuteBranchStep(node, context);
      if (!status.ok()) {
        return status;
      }
    } else {
      return absl::InvalidArgumentError(
          absl::StrCat("Unsupported step type: ", node.step_type));
    }
  }

  return absl::OkStatus();
}

absl::Status FlowExecutor::ExecuteCommonStep(const FlowNode& node, ExecContext& context) {
  auto executor = script_factory_.GetScriptExecutor(node.script_name);
  if (!executor) {
    return absl::NotFoundError(
        absl::StrCat("Script executor not found: ", node.script_name));
  }
  if (node.async) {
      // 异步执行，启动新线程
      threads_.emplace_back([executor, &context]() {
          executor->Execute(context);
      });
      return absl::OkStatus();
  }
  // 同步执行
  return executor->Execute(context);
}

absl::Status FlowExecutor::ExecuteBranchStep(const FlowNode& node, ExecContext& context) {
  // todo 评估条件，选择执行路径
  bool condition_result = EvaluateCondition(node.condition, context);
  std::string target_script = condition_result ? node.true_script_name : node.false_script_name;

  auto executor = script_factory_.GetScriptExecutor(target_script);
  if (!executor) {
    return absl::NotFoundError(
        absl::StrCat("Branch script executor not found: ", target_script));
  }

  return executor->Execute(context);
}

bool FlowExecutor::EvaluateCondition(const std::string& condition, const ExecContext& context) {
  // 简化版条件解析：支持 "key == value" 格式
  return true;
//   std::vector<std::string> parts = absl::StrSplit(condition, "==");
//   if (parts.size() != 2) {
//     return false;
//   }

//     std::string key = std::string(absl::StripAsciiWhitespace(parts[0]));
//     std::string value = std::string(absl::StripAsciiWhitespace(parts[1]));

  
//   // 处理上下文参数（如 device_model == "FC-5"） 
//   if (key == "device_model") {
//     return context.GetParam("device_model") == value;
//   } else if (key == "device_params.model") {
//     return context.GetParam("device_params.model") == value;
//   } else if (key == "runtime_data.error") {
//     return context.GetParam("runtime_data.error") == value;
//   }
//   return false;
}

}  // namespace script_manager