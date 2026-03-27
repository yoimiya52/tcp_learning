#pragma once

#include <string>
#include <unordered_map>
#include <atomic>
#include <mutex>

namespace script_manager {

  enum class ScriptExecStatus {
  kNotStarted,  // 未开始
  kRunning,     // 执行中
  kSucceeded,   // 执行成功
  kFailed       // 执行失败
};

class ExecContext {
 public:
  // 设置上下文参数
  void SetParam(const std::string& key, const std::string& value);
  std::string GetParam(const std::string& key, const std::string& default_val = "") const;
  void SetError(bool is_error, const std::string& error_msg = "");
  bool HasError() const;
  std::string GetErrorMsg() const;

  // 新增：执行追踪相关方法
  void SetExecStatus(ScriptExecStatus status);
  ScriptExecStatus GetExecStatus() const;
  
  void SetOutputFilePath(const std::string& path);
  std::string GetOutputFilePath() const;
  
  void AppendOutput(const std::string& content);
  std::string GetOutput();
  
  void SetExitCode(int code);
  int GetExitCode() const;

 private:
  std::unordered_map<std::string, std::string> params_;
  bool has_error_ = false;
  std::string error_msg_;

  // 新增：执行追踪字段
  std::atomic<ScriptExecStatus> exec_status_{ScriptExecStatus::kNotStarted};
  std::string output_file_path_;
  std::string output_content_;
  std::mutex output_mutex_;  // 保护输出内容的线程安全写入
  int exit_code_ = 0;        // 脚本退出码
};

}  // namespace script_manager