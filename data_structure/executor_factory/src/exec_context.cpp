#include "exec_context.h"
#include <iostream>
namespace script_manager {

void ExecContext::SetParam(const std::string& key, const std::string& value) {
  params_[key] = value;
}

std::string ExecContext::GetParam(const std::string& key, const std::string& default_val) const {
  auto it = params_.find(key);
  return it != params_.end() ? it->second : default_val;
}
void ExecContext::SetError(bool is_error, const std::string& error_msg) {
  has_error_ = is_error;
  error_msg_ = error_msg;
  std::cout << "isError: " << is_error << " error_msg: " << error_msg << std::endl;
}

bool ExecContext::HasError() const {
  return has_error_;
}

std::string ExecContext::GetErrorMsg() const {
  return error_msg_;
}
void ExecContext::SetExecStatus(ScriptExecStatus status) {
  exec_status_.store(status);
}

ScriptExecStatus ExecContext::GetExecStatus() const {
  return exec_status_.load();
}

void ExecContext::SetOutputFilePath(const std::string& path) {
  output_file_path_ = path;
}

std::string ExecContext::GetOutputFilePath() const {
  return output_file_path_;
}

void ExecContext::AppendOutput(const std::string& content) {
  std::lock_guard<std::mutex> lock(output_mutex_);
  output_content_ += content;
}

std::string ExecContext::GetOutput(){
  std::lock_guard<std::mutex> lock(output_mutex_);
  return output_content_;
}

void ExecContext::SetExitCode(int code) {
  exit_code_ = code;
}

int ExecContext::GetExitCode() const {
  return exit_code_;
}
}  // namespace script_manager