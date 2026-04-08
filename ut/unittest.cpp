#include "doctest.h"
#include "JConfigParser/Node.h"

TEST_CASE("Node 构造函数测试") {
    // 默认构造函数（无效节点）
    Node invalid;
    CHECK(!invalid.isValid());
    CHECK(!invalid.isError());
    
    // createObject 静态方法
    Node obj = Node::createObject();
    CHECK(obj.isValid());
    CHECK(obj.isObject());
    CHECK(!obj.isError());
    
    // createArray 静态方法
    Node arr = Node::createArray();
    CHECK(arr.isValid());
    CHECK(arr.isArray());
    CHECK(!arr.isError());
    
    // 字符串构造
    Node strNode("test");
    CHECK(strNode.isValid());
    CHECK(strNode.isString());
    CHECK(strNode.getValue<std::string>() == "test");
    
    // 整数构造
    Node intNode(42LL);
    CHECK(intNode.isValid());
    CHECK(intNode.isInt64());
    CHECK(intNode.getValue<int64_t>() == 42);
    
    // 浮点数构造
    Node doubleNode(3.14);
    CHECK(doubleNode.isValid());
    CHECK(doubleNode.is<double>());
    CHECK(doubleNode.getValue<double>() == 3.14);
    
    // 布尔值构造
    Node boolNode(true);
    CHECK(boolNode.isValid());
    CHECK(boolNode.isBool());
    CHECK(boolNode.getValue<bool>() == true);
}

TEST_CASE("Node 状态判断测试") {
    // 有效节点
    Node valid = Node::createObject();
    CHECK(valid.isValid());
    CHECK(!valid.isError());
    
    // 无效节点
    Node invalid;
    CHECK(!invalid.isValid());
    CHECK(!invalid.isError());
    
    // 错误节点
    Node errorNode = invalid.get("key");
    CHECK(!errorNode.isValid());
    CHECK(errorNode.isError());
    CHECK(!errorNode.getError().empty());
    
    // null节点
    Node nullNode;
    CHECK(!nullNode.isValid());
}

TEST_CASE("Node 类型判断测试") {
    Node obj = Node::createObject();
    CHECK(obj.isObject());
    CHECK(!obj.isArray());
    CHECK(!obj.isString());
    CHECK(!obj.isNumber());
    CHECK(!obj.isBool());
    
    Node arr = Node::createArray();
    CHECK(arr.isArray());
    CHECK(!arr.isObject());
    
    Node strNode("test");
    CHECK(strNode.isString());
    
    Node intNode(42LL);
    CHECK(intNode.isInt64());
    CHECK(intNode.isNumber());
    
    Node doubleNode(3.14);
    CHECK(doubleNode.isDouble());
    CHECK(doubleNode.isNumber());
    
    Node boolNode(true);
    CHECK(boolNode.isBool());
}

TEST_CASE("Node 取值测试") {
    Node obj = Node::createObject();
    obj.set("name", "Alice");
    obj.set("age", int64_t(30));
    obj.set("height", 1.65);
    obj.set("active", true);
    
    // 正确类型取值
    CHECK(obj.get("name").getValue<std::string>() == "Alice");
    CHECK(obj.get("age").getValue<int64_t>() == 30);
    CHECK(obj.get("height").getValue<double>() == 1.65);
    CHECK(obj.get("active").getValue<bool>() == true);
    
    // 类型不匹配取值（返回默认值）
    CHECK(obj.get("name").getValue<int64_t>() == 0);
    CHECK(obj.get("age").getValue<std::string>() == "");
    
    // getValueOr 测试
    CHECK(obj.get("name").getValueOr(std::string("default")) == "Alice");
    CHECK(obj.get("nonexistent").getValueOr(std::string("default")) == "default");
    CHECK(obj.get("nonexistent").getValueOr(int64_t(100)) == 100);
}

TEST_CASE("Node 子节点操作测试") {
    Node obj = Node::createObject();
    obj.set("name", "Bob");
    obj.set("age", int64_t(25));
    
    // has 方法测试
    CHECK(obj.has("name"));
    CHECK(obj.has("age"));
    CHECK(!obj.has("nonexistent"));
    
    // get 方法测试
    Node nameNode = obj.get("name");
    CHECK(nameNode.isValid());
    CHECK(nameNode.getValue<std::string>() == "Bob");
    
    Node nonexistentNode = obj.get("nonexistent");
    CHECK(!nonexistentNode.isValid());
    CHECK(nonexistentNode.isError());
    
    // keys 方法测试
    std::vector<std::string> keys = obj.keys();
    CHECK(keys.size() == 2);
    CHECK(std::find(keys.begin(), keys.end(), "name") != keys.end());
    CHECK(std::find(keys.begin(), keys.end(), "age") != keys.end());
    
    // size 方法测试
    CHECK(obj.size() == 2);
    
    Node arr = Node::createArray();
    arr.append(1);
    arr.append(2);
    arr.append(3);
    CHECK(arr.size() == 3);
    
    // at 方法测试
    CHECK(arr.at(0).getValue<int64_t>() == 1);
    CHECK(arr.at(1).getValue<int64_t>() == 2);
    CHECK(arr.at(2).getValue<int64_t>() == 3);
    
    Node outOfBounds = arr.at(10);
    CHECK(!outOfBounds.isValid());
    CHECK(outOfBounds.isError());
}

TEST_CASE("Node 设置值测试") {
    Node obj = Node::createObject();
    
    // set 方法测试
    obj.set("name", "Charlie");
    obj.set("age", int64_t(35));
    obj.set("height", 1.75);
    obj.set("active", false);
    
    CHECK(obj.get("name").getValue<std::string>() == "Charlie");
    CHECK(obj.get("age").getValue<int64_t>() == 35);
    CHECK(obj.get("height").getValue<double>() == 1.75);
    CHECK(obj.get("active").getValue<bool>() == false);
    
    // setObject 方法测试
    Node subObj = obj.setObject("address");
    CHECK(subObj.isValid());
    CHECK(subObj.isObject());
    
    subObj.set("street", "Main St");
    subObj.set("city", "New York");
    CHECK(obj.get("address").get("street").getValue<std::string>() == "Main St");
    CHECK(obj.get("address").get("city").getValue<std::string>() == "New York");
    
    // setArray 方法测试
    Node subArr = obj.setArray("hobbies");
    CHECK(subArr.isValid());
    CHECK(subArr.isArray());
    
    subArr.append("reading");
    subArr.append("gaming");
    CHECK(obj.get("hobbies").size() == 2);
    CHECK(obj.get("hobbies").at(0).getValue<std::string>() == "reading");
    
    // remove 方法测试
    obj.remove("age");
    CHECK(!obj.has("age"));
}

TEST_CASE("Node 数组追加测试") {
    Node arr = Node::createArray();
    
    // append 方法测试
    arr.append("first");
    arr.append(int64_t(2));
    arr.append(3.14);
    arr.append(false);
    
    CHECK(arr.size() == 4);
    CHECK(arr.at(0).getValue<std::string>() == "first");
    CHECK(arr.at(1).getValue<int64_t>() == 2);
    CHECK(arr.at(2).getValue<double>() == 3.14);
    CHECK(arr.at(3).getValue<bool>() == false);
    
    // appendObject 方法测试
    Node obj1 = arr.appendObject();
    obj1.set("name", "Object 1");
    CHECK(arr.size() == 5);
    CHECK(arr.at(4).get("name").getValue<std::string>() == "Object 1");
    
    // appendArray 方法测试
    Node arr1 = arr.appendArray();
    arr1.append(1);
    arr1.append(2);
    CHECK(arr.size() == 6);
    CHECK(arr.at(5).size() == 2);
    CHECK(arr.at(5).at(0).getValue<int64_t>() == 1);
}

TEST_CASE("Node 序列化测试") {
    Node obj = Node::createObject();
    obj.set("name", "David");
    obj.set("age", int64_t(40));
    
    Node hobbies = obj.setArray("hobbies");
    hobbies.append("reading");
    hobbies.append("swimming");
    
    // toJson 测试（紧凑格式）
    std::string compactJson = obj.toJson();
    CHECK(!compactJson.empty());
    CHECK(compactJson.find('\n') == std::string::npos);
    
    // toJson 测试（美化格式）
    std::string prettyJson = obj.toJson(true);
    CHECK(!prettyJson.empty());
    CHECK(prettyJson.find('\n') != std::string::npos);
}

TEST_CASE("Node JSON解析测试") {
    // 测试有效JSON
    std::string validJson = R"({"name": "Eve", "age": 28, "active": true})";
    Node validNode = Node::fromJson(validJson);
    CHECK(validNode.isValid());
    CHECK(!validNode.isError());
    CHECK(validNode.get("name").getValue<std::string>() == "Eve");
    CHECK(validNode.get("age").getValue<int64_t>() == 28);
    CHECK(validNode.get("active").getValue<bool>() == true);
    
    // 测试无效JSON
    std::string invalidJson = "{bad json}";
    Node invalidNode = Node::fromJson(invalidJson);
    CHECK(!invalidNode.isValid());
    CHECK(invalidNode.isError());
    CHECK(!invalidNode.getError().empty());
}

TEST_CASE("Node 拷贝与克隆测试") {
    Node original = Node::createObject();
    original.set("name", "Frank");
    original.set("age", int64_t(30));
    
    // 拷贝构造函数
    Node copy(original);
    CHECK(copy.isValid());
    CHECK(copy.get("name").getValue<std::string>() == "Frank");
    CHECK(copy.get("age").getValue<int64_t>() == 30);
    
    // 赋值运算符
    Node assigned;
    assigned = original;
    CHECK(assigned.isValid());
    CHECK(assigned.get("name").getValue<std::string>() == "Frank");
    CHECK(assigned.get("age").getValue<int64_t>() == 30);
    
    // 克隆方法
    Node cloned = original.clone();
    CHECK(cloned.isValid());
    CHECK(cloned.get("name").getValue<std::string>() == "Frank");
    CHECK(cloned.get("age").getValue<int64_t>() == 30);
    
    // 验证深拷贝
    original.set("age", int64_t(31));
    CHECK(copy.get("age").getValue<int64_t>() == 30); // 拷贝应该不受影响
    CHECK(assigned.get("age").getValue<int64_t>() == 30); // 赋值应该不受影响
    CHECK(cloned.get("age").getValue<int64_t>() == 30); // 克隆应该不受影响
}

TEST_CASE("Node 错误处理测试") {
    Node invalid;
    
    // 对无效节点操作
    Node error1 = invalid.get("key");
    CHECK(!error1.isValid());
    CHECK(error1.isError());
    
    Node error2 = invalid.at(0);
    CHECK(!error2.isValid());
    CHECK(error2.isError());
    
    // 对非对象使用get
    Node arr = Node::createArray();
    Node error3 = arr.get("key");
    CHECK(!error3.isValid());
    CHECK(error3.isError());
    
    // 对非数组使用at
    Node obj = Node::createObject();
    Node error4 = obj.at(0);
    CHECK(!error4.isValid());
    CHECK(error4.isError());
    
    // 数组越界
    Node error5 = arr.at(0);
    CHECK(!error5.isValid());
    CHECK(error5.isError());
}

TEST_CASE("Node 边界情况测试") {
    // 空字符串
    Node emptyStr("");
    CHECK(emptyStr.isValid());
    CHECK(emptyStr.isString());
    CHECK(emptyStr.getValue<std::string>() == "");
    
    // 空对象
    Node emptyObj = Node::createObject();
    CHECK(emptyObj.isValid());
    CHECK(emptyObj.isObject());
    CHECK(emptyObj.size() == 0);
    
    // 空数组
    Node emptyArr = Node::createArray();
    CHECK(emptyArr.isValid());
    CHECK(emptyArr.isArray());
    CHECK(emptyArr.size() == 0);
    
    // 大整数
    Node bigInt(INT64_MAX);
    CHECK(bigInt.isValid());
    CHECK(bigInt.isInt64());
    CHECK(bigInt.getValue<int64_t>() == INT64_MAX);
    
    // 负数
    Node negative(-42LL);
    CHECK(negative.isValid());
    CHECK(negative.isInt64());
    CHECK(negative.getValue<int64_t>() == -42);
}
