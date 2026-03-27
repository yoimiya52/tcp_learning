#include "script_executor.h"
#include <fstream>
#include <cstdlib>
#include <thread>
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"


namespace script_manager {


std::string GenerateOutputFilePath(const std::string& script_id, ExecContext& context) {
  // 获取当前时间戳（毫秒级），避免文件名冲突
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  if(context.GetOutputFilePath().empty()){
      std::string output_dir = "/tmp/script_output/";
      system(absl::StrCat("mkdir -p ", output_dir).c_str());
      // 生成文件名：script_id_时间戳.log
      std::string filename = absl::StrFormat("%s_%lld.log", script_id, ms);
      return absl::StrCat(output_dir, filename);
  }else{
      system(absl::StrCat("mkdir -p ", context.GetOutputFilePath()).c_str());
      std::string filename = absl::StrFormat("%s_%lld.log", script_id, ms);
      return absl::StrCat(context.GetOutputFilePath(), filename);
  }
}


absl::Status ShellScriptExecutor::Execute(ExecContext& context) {
  try {
    context.SetExecStatus(ScriptExecStatus::kRunning);
    
    // 生成输出文件路径
    std::string output_file_path = GenerateOutputFilePath(GetScriptId(),context);
  
    // 检查 ExecContext
    ExecContext* ctx_ptr = &context;
    if (ctx_ptr == nullptr) {
      return absl::InvalidArgumentError("ExecContext pointer is null");
    }

    // 检查脚本路径
    if (script_info_.path.empty()) {
      context.SetError(true, "Script path is empty");
      context.SetExecStatus(ScriptExecStatus::kFailed);
      return absl::InvalidArgumentError("Script path is empty");
    }

      try {
        // 打开输出文件
        std::ofstream output_file(output_file_path, std::ios::out | std::ios::app);
        if (!output_file.is_open()) {
          std::string error_msg = absl::StrCat("Failed to open file ", output_file_path, ": ", strerror(errno));
          ctx_ptr->SetError(true, error_msg);
          ctx_ptr->SetExecStatus(ScriptExecStatus::kFailed);
          return absl::InvalidArgumentError(error_msg);
        }

        // 时间戳
        std::string timestamp = absl::FormatTime(
            "%Y-%m-%d %H:%M:%S", absl::Now(), absl::LocalTimeZone());

        std::string start_msg = absl::StrFormat(
            "[%s] Start executing script: %s (path: %s)\n",
            timestamp, script_info_.script_id, script_info_.path);

        output_file << start_msg;
        ctx_ptr->AppendOutput(start_msg);

        // 检查脚本文件是否存在
        if (access(script_info_.path.c_str(), F_OK) != 0) {
          std::string error_msg = absl::StrCat("Script not found: ", script_info_.path);
          output_file << error_msg << "\n";
          ctx_ptr->SetError(true, error_msg);
          ctx_ptr->SetExecStatus(ScriptExecStatus::kFailed);
          return absl::NotFoundError(error_msg);
        }

        // 执行脚本
        std::string cmd = absl::StrCat("bash ", script_info_.path, " 2>&1");
        FILE* fp = popen(cmd.c_str(), "r");
        if (!fp) {
          std::string error_msg = absl::StrCat("Failed to execute script: ", script_info_.path, ": ", strerror(errno));
          output_file << error_msg << "\n";
          ctx_ptr->SetError(true, error_msg);
          ctx_ptr->SetExecStatus(ScriptExecStatus::kFailed);
          return absl::InternalError(error_msg);
        }

        // 读取输出
        char buffer[1024];
        std::string total_output;
        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
          std::string line(buffer);
          total_output += line;
          output_file << line;
          ctx_ptr->AppendOutput(line);
          output_file.flush();
        }

        // 处理返回值
        int ret = pclose(fp);
        int exit_code = 0;

        if (WIFEXITED(ret)) {
          exit_code = WEXITSTATUS(ret);
        } else if (WIFSIGNALED(ret)) {
          exit_code = WTERMSIG(ret);
        }
        ctx_ptr->SetExitCode(exit_code);

        // 执行结果
        if (ret != 0) {
          std::string error_msg = absl::StrFormat(
              "[%s] Script failed (exit code: %d)\nOutput:\n%s",
              timestamp, exit_code, total_output);

          output_file << error_msg << "\n";
          ctx_ptr->SetError(true, error_msg);
          ctx_ptr->SetExecStatus(ScriptExecStatus::kFailed);
        } else {
          std::string success_msg = absl::StrFormat(
              "[%s] Script succeeded (exit code: %d)\n",
              timestamp, exit_code);

          output_file << success_msg << "\n";
          // 临时测试，打印错误
          ctx_ptr->SetError(false,script_info_.script_id);
          ctx_ptr->SetExecStatus(ScriptExecStatus::kSucceeded);
        }

      } catch (const std::exception& e) {
        std::cout << "Exception caught in script execution thread: " << e.what() << std::endl;
        ctx_ptr->SetError(true, absl::StrCat("Script execution threw exception: ", e.what()));
        ctx_ptr->SetExecStatus(ScriptExecStatus::kFailed);
      } catch (...) {
        std::cout << "Unknown exception caught in script execution thread" << std::endl;
        ctx_ptr->SetError(true, "Unknown exception during script execution");
        ctx_ptr->SetExecStatus(ScriptExecStatus::kFailed);
      }

    return absl::OkStatus();

  } catch (const std::exception& e) {
    return absl::InternalError(absl::StrCat("Failed to start thread: ", e.what()));
  }
}

absl::Status DagScriptExecutor::Execute(ExecContext& context) {
    // 拷贝脚本信息和 context 给 lambda
    ScriptInfo script_info = script_info_;
    ExecContext* ctx = &context;  // 注意生命周期，线程中使用必须安全

    // g_thread_pool.Submit([script_info, ctx]() {
    //     // 构建 DAG 执行命令
    //     std::string cmd = absl::StrCat("apollo_dag_executor --dag_path ", script_info.path, " 2>&1");
    //     FILE* fp = popen(cmd.c_str(), "r");
    //     if (!fp) {
    //         ctx->SetError(true, absl::StrCat("Failed to execute DAG script: ", script_info.path));
    //         return;
    //     }

    //     // 读取执行输出
    //     char buffer[1024];
    //     std::string output;
    //     while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
    //         output += buffer;
    //     }

    //     int ret = pclose(fp);
    //     if (ret != 0) {
    //         std::string error_msg = absl::StrFormat(
    //             "DAG script execute failed (code: %d), output: %s", ret, output);
    //         ctx->SetError(true, error_msg);
    //     } else {
    //         ctx->SetError(false);
    //     }
    // });

    // fire-and-forget，不阻塞等待线程执行结果
    return absl::OkStatus();
}
}  // namespace script_manager