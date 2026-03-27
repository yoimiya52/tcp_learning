#pragma once

#include <string>
#include "exec_context.h"
#include "config_parser.h"
#include "absl/status/status.h"
#include <thread>

namespace script_manager {

// 脚本执行器基类
class BaseScriptExecutor {
 public:
  explicit BaseScriptExecutor(const ScriptInfo& script_info) 
      : script_info_(script_info) {}
  virtual ~BaseScriptExecutor() {
      Wait();
  };

  // 执行脚本
  virtual absl::Status Execute(ExecContext& context) = 0;


  // 获取脚本ID
  std::string GetScriptId() const { return script_info_.script_id; }

  // 获取脚本路径
  std::string GetScriptPath() const { return script_info_.path; }

  void Wait(){
      for(auto& t : threads_){
          if(t.joinable()){
              t.join();
          }
      }
      threads_.clear();
  }
 protected:
  ScriptInfo script_info_;  // 脚本元信息
  std::vector<std::thread> threads_;
};

// Shell脚本执行器
class ShellScriptExecutor : public BaseScriptExecutor {
 public:
  explicit ShellScriptExecutor(const ScriptInfo& script_info) 
      : BaseScriptExecutor(script_info) {}
  
  absl::Status Execute(ExecContext& context) override;
};

// DAG脚本执行器
class DagScriptExecutor : public BaseScriptExecutor {
 public:
  explicit DagScriptExecutor(const ScriptInfo& script_info) 
      : BaseScriptExecutor(script_info) {}
  
  absl::Status Execute(ExecContext& context) override;
};

}  // namespace script_manager