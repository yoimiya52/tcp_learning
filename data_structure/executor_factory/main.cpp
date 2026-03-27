#include <iostream>
#include <nlohmann/json.hpp>
#include "config_parser.h"
#include "flow_executor.h"

using json = nlohmann::json;
using namespace script_manager;
int main() {
    try {
    DeviceScriptConfig config = DeviceScriptConfig();
    ExecContext context = ExecContext();
    ConfigParser parser = ConfigParser();
    ScriptFactory factory = ScriptFactory();
    // 加载配置文件
    absl::Status status = parser.LoadConfigFromFile("/home/tars/projects/tcp_learning/data_structure/executor_factory/config/document_20251204_111812.json", config ,context);
    if (!status.ok()) {
        std::cerr << "Failed to load config file: " << status.ToString() << std::endl;
        return 1;
    }
    std::cout << "Device: " << config.device << std::endl;
    for (const auto& script : config.scripts) {
        std::cout << "Script: " << script.script_id << std::endl;
    }
    factory.Init(config.scripts);
    FlowExecutor executor = FlowExecutor(factory);
    executor.ExecuteFlow(config.flow, context);
    executor.Wait();
  } catch (const std::system_error& e) {
    std::cerr << "System error: " << e.what() << std::endl;
    std::cerr << "Error code: " << e.code() << std::endl;
    std::cerr << "Native error code: " << e.code().value() << std::endl;
    std::cerr << "Error message: " << strerror(e.code().value()) << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "Standard exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown exception" << std::endl;
  }
  return 0;
}