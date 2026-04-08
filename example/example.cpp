#include <iostream>
#include "Node.h"

int main() {
    // ========== 示例1: 检查节点有效性 ==========
    std::cout << "=== 示例1: 检查节点有效性 ===" << std::endl;
    
    Node valid = Node::createObject();
    Node invalid;  // 默认构造的无效节点
    
    std::cout << "valid 是否有效: " << (valid.isValid() ? "是" : "否") << std::endl;
    std::cout << "invalid 是否有效: " << (invalid.isValid() ? "是" : "否") << std::endl;
    
    
    // ========== 示例2: 错误传播 ==========
    std::cout << "\n=== 示例2: 错误传播 ===" << std::endl;
    
    Node obj = Node::createObject();
    obj.set("name", std::string("Alice"));
    
    // 获取不存在的键，返回带错误信息的节点
    Node missingKey = obj.get("nickname");
    std::cout << "missingKey 是否有效: " << (missingKey.isValid() ? "是" : "否") << std::endl;
    std::cout << "missingKey 是否有错误: " << (missingKey.isError() ? "是" : "否") << std::endl;
    std::cout << "错误信息: " << missingKey.getError() << std::endl;
    
    // 对无效节点继续操作，错误会传播
    Node deepMissing = obj.get("address").get("city");
    std::cout << "deepMissing 是否有错误: " << (deepMissing.isError() ? "是" : "否") << std::endl;
    std::cout << "错误信息: " << deepMissing.getError() << std::endl;
    
    
    // ========== 示例3: 安全取值（自动处理无效节点）==========
    std::cout << "\n=== 示例3: 安全取值 ===" << std::endl;
    
    // 对无效节点取值，返回默认值
    std::string name = missingKey.getValue<std::string>();
    std::cout << "name (默认值): '" << name << "'" << std::endl;
    
    // 使用自定义默认值
    std::string nickname = missingKey.getValueOr(std::string("未知用户"));
    std::cout << "nickname: " << nickname << std::endl;
    
    
    // ========== 示例4: JSON 解析错误处理 ==========
    std::cout << "\n=== 示例4: JSON 解析错误处理 ===" << std::endl;
    
    // 合法的 JSON
    Node validJson = Node::fromJson(R"({"x": 1, "y": 2})");
    std::cout << "validJson 是否有效: " << (validJson.isValid() ? "是" : "否") << std::endl;
    std::cout << "validJson 是否有错误: " << (validJson.isError() ? "是" : "否") << std::endl;
    
    // 非法的 JSON
    Node invalidJson = Node::fromJson("{bad json}");
    std::cout << "invalidJson 是否有效: " << (invalidJson.isValid() ? "是" : "否") << std::endl;
    std::cout << "invalidJson 是否有错误: " << (invalidJson.isError() ? "是" : "否") << std::endl;
    std::cout << "错误信息: " << invalidJson.getError() << std::endl;
    
    
    // ========== 示例5: 链式调用中的错误处理 ==========
    std::cout << "\n=== 示例5: 链式调用中的错误处理 ===" << std::endl;
    
    Node config = Node::fromJson(R"({
        "database": {
            "host": "localhost",
            "port": 5432
        }
    })");
    
    // 正确的访问路径
    std::string host = config.get("database").get("host").getValueOr(std::string("127.0.0.1"));
    int64_t port = config.get("database").get("port").getValueOr(int64_t(3306));
    std::cout << "数据库: " << host << ":" << port << std::endl;
    
    // 错误的访问路径（自动返回默认值）
    std::string username = config.get("database").get("username").getValueOr(std::string("root"));
    int64_t timeout = config.get("database").get("timeout").getValueOr(int64_t(30));
    std::cout << "用户名: " << username << ", 超时: " << timeout << std::endl;
    
    // 检查是否有错误
    Node timeoutNode = config.get("database").get("timeout");
    if (timeoutNode.isError()) {
        std::cout << "警告: " << timeoutNode.getError() << std::endl;
    }
    
    
    // ========== 示例6: 构建复杂结构（零拷贝）==========
    std::cout << "\n=== 示例6: 构建复杂结构 ===" << std::endl;
    
    Node root = Node::createObject();
    root.set("version", std::string("1.0.0"));
    
    // 零拷贝创建嵌套结构
    Node db = root.setObject("database");
    db.set("host", std::string("localhost"))
      .set("port", int64_t(5432));
    
    Node pool = db.setObject("pool");
    pool.set("minSize", int64_t(5))
        .set("maxSize", int64_t(20));
    
    Node servers = root.setArray("servers");
    
    Node server1 = servers.appendObject();
    server1.set("host", std::string("192.168.1.10"))
           .set("port", int64_t(8080));
    
    Node server2 = servers.appendObject();
    server2.set("host", std::string("192.168.1.11"))
           .set("port", int64_t(8080));
    
    std::cout << root.toJson(true) << std::endl;
    
    

    
    return 0;
}