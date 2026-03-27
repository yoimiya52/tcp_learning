#pragma once

#include "script_factory.h"
#include "config_parser.h"
#include "exec_context.h"

namespace script_manager {

class FlowExecutor {
 public:
  explicit FlowExecutor(ScriptFactory& factory) : script_factory_(factory) {}

  void Wait(){
      for(auto& t : threads_){
          if(t.joinable()){
              t.join();
          }
      }
      threads_.clear();
  }
  // 执行整个流程
  absl::Status ExecuteFlow(const std::vector<FlowNode>& flow, ExecContext& context);

 private:
  // 执行普通步骤
  absl::Status ExecuteCommonStep(const FlowNode& node, ExecContext& context);
  
  // 执行分支步骤
  absl::Status ExecuteBranchStep(const FlowNode& node, ExecContext& context);
  
  // 解析条件表达式（简化版，可扩展为完整表达式解析器）
  bool EvaluateCondition(const std::string& condition, const ExecContext& context);

  ScriptFactory& script_factory_;
  // 流程执行器的脚本执行线程列表
  std::vector<std::thread> threads_;
};

}  // namespace script_manager