#include "Node.h"
#include <iostream>

using namespace ConfigParser;

#ifdef _WIN32
#include <windows.h>
#endif

// ========== 最佳实践指南 ==========
/*
JConfigParser 的错误处理机制:

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
【错误处理：使用 isValid() 统一检查】
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

快速参考表：
┌────────────────────────┬─────────────────────────────────────┐
│ 场景                    │ 推荐做法                           │
├────────────────────────┼─────────────────────────────────────┤
│ 检查节点是否可用        │ 用 isValid() 检查                  │
│ 获取错误信息            │ 用 getError()                      │
│ 需要默认值              │ 用 getValueOr(default)             │
│────────────────────────┴─────────────────────────────────────┤
│ 💡 最简单：不确定就用 getValueOr()，不需要任何检查！       │
└─────────────────────────────────────────────────────────────┘

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
【isValid() 返回 false 的情况】
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

当 isValid() 返回 false 时，节点不可用，getError() 会返回具体原因：

❌ 情况1：键不存在
   Node missing = obj.get("missingKey");
   missing.isValid()          → false
   missing.getError()         → "Key not found: missingKey"

❌ 情况2：索引越界
   Node outOfBounds = arr.at(100);
   outOfBounds.isValid()      → false
   outOfBounds.getError()     → "Index out of bounds: 100"

❌ 情况3：JSON 解析失败
   Node invalidJson = Node::fromJson("{invalid}");
   invalidJson.isValid()      → false
   invalidJson.getError()     → "Parse error: ..."

❌ 情况4：节点已被移动（移动语义）
   Node obj2 = std::move(obj1);
   obj1.isValid()             → false
   obj1.getError()            → "" (移动后源对象，已失效)

❌ 情况5：未初始化的节点
   Node emptyNode;  // 默认构造
   emptyNode.isValid()        → false
   emptyNode.getError()       → "" (未初始化)

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
【实战示例】
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

示例1：读取配置（推荐最简单方式）
───────────────────────────────────────────────────────────────

不需要任何 isValid() 或 isError() 检查！

    std::string host = config.get("host").getValueOr("localhost");
    int port = config.get("port").getValueOr(8080);

这是最简单、最安全、最常用的方式！

示例2：检查必需字段（需要错误信息时）
───────────────────────────────────────────────────────────────

当字段必须存在时：

    Node host = config.get("host");
    if (!host.isValid()) {
        std::cerr << "配置错误: " << host.getError() << std::endl;
        // getError() 会告诉你："Key 'host' not found"
        exit(1);
    }
    std::string hostValue = host.getValue<std::string>();

用 isValid() 检查，需要详细信息用 getError()

示例3：处理数组访问
───────────────────────────────────────────────────────────────

    Node users = data.get("users");  // 先访问键
    if (!users.isValid()) {
        std::cerr << "没有 users 字段: " << users.getError() << std::endl;
        return;
    }
    if (!users.isArray()) {
        std::cerr << "users 不是数组" << std::endl;
        return;
    }

    for (int i = 0; i < users.size(); i++) {
        Node user = users.at(i);  // 访问数组元素
        if (!user.isValid()) {
            std::cerr << "索引 " << i << " 越界" << std::endl;
            continue;
        }
        // 处理 user
        std::string name = user.get("name").getValueOr("Unknown");
        int age = user.get("age").getValueOr(0);
    }

检查键和索引都用 isValid()

示例4：JSON 解析后验证
───────────────────────────────────────────────────────────────

    Node doc = Node::fromJson(jsonStr);
    if (!doc.isValid()) {
        std::cerr << "解析失败: " << doc.getError() << std::endl;
        return;
    }
    // 解析成功，继续处理...

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
【决策树】
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

你是要干什么？
│
├─ 字段存在就处理，不存在就用默认值
│  └─ 用 getValueOr(default)（最简单！）
│
├─ 字段必须存在，不存在就报错
│  └─ 用 if (!node.isValid()) { ... } + getError()
│
└─ JSON 解析后验证
   └─ 用 if (!doc.isValid()) { ... }

简化版：不确定就用 getValueOr()！
*/

int main() {
#ifdef _WIN32
    // 设置控制台为UTF-8编码以正确显示中文
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::cout << "========================================" << std::endl;
    std::cout << "   JConfigParser 示例程序" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // ========== 示例1: 节点基础状态检查 ==========
    std::cout << "=== 示例1: 节点基础状态检查 ===" << std::endl;

    Node valid = Node::createObject();     // ✓ 有效的对象节点
    Node invalid;                           // ✗ 无效节点（默认构造）

    std::cout << "\n【使用 isValid() 检查节点状态】" << std::endl;
    std::cout << "  valid.isValid()   = " << (valid.isValid() ? "true" : "false")
              << " (可以安全操作)" << std::endl;
    std::cout << "  invalid.isValid() = " << (invalid.isValid() ? "true" : "false")
              << " (不可操作)" << std::endl;

    std::cout << "\n【结论】" << std::endl;
    std::cout << "  - isValid() = true: 节点可用，可以安全操作" << std::endl;
    std::cout << "  - isValid() = false: 节点不可用，getError() 可获取原因" << std::endl;

    // ========== 示例2: 访问不存在的键（错误处理） ==========
    std::cout << "\n=== 示例2: 访问不存在的键 ===\n" << std::endl;

    Node obj = Node::createObject();
    obj.set("name", std::string("Alice"));

    std::cout << "对象内容: " << obj.toJson(true) << "\n" << std::endl;

    std::cout << "【尝试访问不存在的键 \"nickname\"]" << std::endl;
    Node missingKey = obj.get("nickname");

    // 检查节点是否有效
    if (!missingKey.isValid()) {
        std::cout << "  ✗ 节点无效 (isValid() = false)" << std::endl;
        std::cout << "  ✗ 错误信息: " << missingKey.getError() << std::endl;
    }

    std::cout << "\n【深度访问不存在的路径】" << std::endl;
    std::cout << "  访问路径: obj -> \"address\" -> \"city\"" << std::endl;
    Node deepMissing = obj.get("address").get("city");

    if (!deepMissing.isValid()) {
        std::cout << "  ✗ 节点无效: " << deepMissing.getError() << std::endl;
        std::cout << "\n【提示】错误会沿着访问路径传播" << std::endl;
    }

    // ========== 示例3: 安全取值（推荐方式） ==========
    std::cout << "\n=== 示例3: 安全取值（推荐方式）===\n" << std::endl;

    std::cout << "【使用 getValueOr() 提供默认值】" << std::endl;

    // 对无效节点取值，返回默认值
    std::string nickname = missingKey.getValueOr(std::string("未知用户"));
    std::cout << "  nickname: " << nickname << " (使用默认值)" << std::endl;

    std::cout << "\n【推荐：链式调用 + 默认值】" << std::endl;
    std::cout << "  代码: obj.get(\"name\").getValueOr(\"default\")" << std::endl;
    std::cout << "  结果: " << obj.get("name").getValueOr(std::string("default")) << std::endl;
    std::cout << "\n  代码: obj.get(\"age\").getValueOr<int64_t>(18)" << std::endl;
    std::cout << "  结果: " << obj.get("age").getValueOr<int64_t>(18) << " (age不存在，使用默认值18)" << std::endl;

    // ========== 示例4: JSON 解析错误处理 ==========
    std::cout << "\n=== 示例4: JSON 解析错误处理 ===\n" << std::endl;

    std::cout << "【合法的 JSON】" << std::endl;
    Node validJson = Node::fromJson(R"({"x": 1, "y": 2})");
    std::cout << "  validJson.isValid() = " << (validJson.isValid() ? "true" : "false") << std::endl;
    std::cout << "  内容: " << validJson.toJson() << std::endl;

    std::cout << "\n【非法的 JSON】" << std::endl;
    Node invalidJson = Node::fromJson("{bad json}");
    std::cout << "  invalidJson.isValid() = " << (invalidJson.isValid() ? "true" : "false") << std::endl;
    std::cout << "  错误信息: " << invalidJson.getError() << std::endl;

    // ========== 示例5: 链式调用中的错误处理 ==========
    std::cout << "\n=== 示例5: 链式调用中的最佳实践 ===\n" << std::endl;

    Node config = Node::fromJson(R"({
        "database": {
            "host": "localhost",
            "port": 5432
        }
    })");

    std::cout << "【方式1：检查必需字段】" << std::endl;
    std::cout << "\n检查必需字段: database.host" << std::endl;
    Node hostNode = config.get("database").get("host");
    if (!hostNode.isValid()) {
        std::cout << "  ✗ 错误: " << hostNode.getError() << std::endl;
    } else {
        std::cout << "  ✓ host = " << hostNode.getValue<std::string>() << std::endl;
    }

    std::cout << "\n检查可选字段: database.username" << std::endl;
    Node usernameNode = config.get("database").get("username");
    if (!usernameNode.isValid()) {
        std::cout << "  ℹ 可选字段不存在，将使用默认值" << std::endl;
    } else {
        std::cout << "  ✓ username = " << usernameNode.getValue<std::string>() << std::endl;
    }

    std::cout << "\n【方式2：链式调用 + getValueOr（推荐用于可选字段）】" << std::endl;
    std::string host = config.get("database").get("host").getValueOr(std::string("127.0.0.1"));
    int64_t port = config.get("database").get("port").getValueOr(int64_t(3306));
    std::string username = config.get("database").get("username").getValueOr(std::string("root"));
    int64_t timeout = config.get("database").get("timeout").getValueOr(int64_t(30));

    std::cout << "  一行代码获取配置：" << std::endl;
    std::cout << "  host = " << host << std::endl;
    std::cout << "  port = " << port << std::endl;
    std::cout << "  username = " << username << " (默认值)" << std::endl;
    std::cout << "  timeout = " << timeout << " (默认值)" << std::endl;

    // ========== 示例6: 数组访问错误处理 ==========
    std::cout << "\n=== 示例6: 数组访问错误处理 ===\n" << std::endl;

    Node arr = Node::createArray();
    arr.append(10).append(20).append(30);
    std::cout << "数组内容: " << arr.toJson() << "\n" << std::endl;

    std::cout << "【正常访问】" << std::endl;
    Node elem0 = arr.at(0);
    std::cout << "  arr.at(0).isValid() = " << (elem0.isValid() ? "true" : "false") << std::endl;
    std::cout << "  值 = " << elem0.getValue<int64_t>() << std::endl;

    std::cout << "\n【越界访问】" << std::endl;
    Node outOfBounds = arr.at(10);
    std::cout << "  arr.at(10).isValid() = " << (outOfBounds.isValid() ? "true" : "false") << std::endl;
    if (!outOfBounds.isValid()) {
        std::cout << "  错误信息: " << outOfBounds.getError() << std::endl;
    }

    std::cout << "\n【推荐：使用 getValueOr() 安全访问】" << std::endl;
    int64_t value = arr.at(5).getValueOr<int64_t>(-1);
    std::cout << "  arr.at(5).getValueOr(-1) = " << value << " (越界，返回默认值)" << std::endl;

    // ========== 示例7: 构建复杂结构（零拷贝） ==========
    std::cout << "\n=== 示例7: 构建复杂结构 ===" << std::endl;

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

    std::cout << "生成的 JSON:\n" << root.toJson(true) << std::endl;

    // ========== 推荐使用总结 ==========
    std::cout << "\n========================================" << std::endl;
    std::cout << "   最佳实践总结" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\n1. 【检查必需字段】" << std::endl;
    std::cout << "   Node node = obj.get(\"required_key\");" << std::endl;
    std::cout << "   if (!node.isValid()) {" << std::endl;
    std::cout << "       std::cerr << node.getError() << std::endl;" << std::endl;
    std::cout << "   }" << std::endl;

    std::cout << "\n2. 【获取可选字段】" << std::endl;
    std::cout << "   std::string value = obj.get(\"key\").getValueOr(\"default\");" << std::endl;

    std::cout << "\n3. 【JSON 解析后验证】" << std::endl;
    std::cout << "   Node node = Node::fromJson(jsonStr);" << std::endl;
    std::cout << "   if (!node.isValid()) {" << std::endl;
    std::cout << "       std::cerr << node.getError() << std::endl;" << std::endl;
    std::cout << "       return;" << std::endl;
    std::cout << "   }" << std::endl;

    std::cout << "\n4. 【安全数组访问】" << std::endl;
    std::cout << "   int value = arr.at(index).getValueOr(defaultValue);" << std::endl;
    std::cout << "   // 越界时返回默认值，不会崩溃" << std::endl;

    std::cout << "\n💡 总结：优先用 getValueOr()，需要错误信息时用 isValid() + getError()" << std::endl;
    std::cout << "========================================\n" << std::endl;

    return 0;
}