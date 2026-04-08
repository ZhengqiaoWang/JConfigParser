// JConfigParser 性能测试和合规测试
#include "../include/JConfigParser/Node.h"
#include "../third_party/rapidjson/include/rapidjson/document.h"
#include "../third_party/rapidjson/include/rapidjson/writer.h"
#include "../third_party/rapidjson/include/rapidjson/stringbuffer.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

// ==================== 工具函数 ====================

/**
 * @brief 读取文件内容
 * @param filepath 文件路径
 * @return 文件内容字符串，失败返回空字符串
 */
std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error: Cannot open file " << filepath << std::endl;
        return "";
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content(size, '\0');
    if (!file.read(content.data(), size)) {
        std::cerr << "Error: Cannot read file " << filepath << std::endl;
        return "";
    }

    return content;
}

/**
 * @brief 获取文件大小（可读格式）
 * @param filepath 文件路径
 * @return 格式化的文件大小字符串
 */
std::string getFileSize(const std::string& filepath) {
    auto size = fs::file_size(filepath);
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double sizeD = static_cast<double>(size);

    while (sizeD >= 1024 && unitIndex < 3) {
        sizeD /= 1024;
        unitIndex++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << sizeD << " " << units[unitIndex];
    return oss.str();
}

/**
 * @brief 格式化文件大小
 * @param size 字节数
 * @return 格式化的字符串
 */
std::string getFileSizeString(size_t size) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double sizeD = static_cast<double>(size);

    while (sizeD >= 1024 && unitIndex < 3) {
        sizeD /= 1024;
        unitIndex++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << sizeD << " " << units[unitIndex];
    return oss.str();
}

// ==================== 性能测试 ====================

/**
 * @brief JConfigParser 解析测试
 * @param jsonStr JSON字符串
 * @param iterations 迭代次数
 * @return 平均解析时间（毫秒）
 */
double benchmarkJConfigParserParse(const std::string& jsonStr, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        Node node = Node::fromJson(jsonStr);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count() / iterations;
}

/**
 * @brief RapidJSON 解析测试
 * @param jsonStr JSON字符串
 * @param iterations 迭代次数
 * @return 平均解析时间（毫秒）
 */
double benchmarkRapidJSONParse(const std::string& jsonStr, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        rapidjson::Document doc;
        doc.Parse(jsonStr.c_str());
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count() / iterations;
}

/**
 * @brief JConfigParser 序列化测试
 * @param node 节点
 * @param iterations 迭代次数
 * @return 平均序列化时间（毫秒）
 */
double benchmarkJConfigParserSerialize(const Node& node, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        std::string json = node.toJson(false);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count() / iterations;
}

/**
 * @brief RapidJSON 序列化测试
 * @param doc RapidJSON文档
 * @param iterations 迭代次数
 * @return 平均序列化时间（毫秒）
 */
double benchmarkRapidJSONSerialize(const rapidjson::Document& doc, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < iterations; i++) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count() / iterations;
}

/**
 * @brief 性能对比测试
 */
void runBenchmarkSpeed() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   性能测试：JConfigParser vs RapidJSON" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::vector<std::string> testFiles = {
        "../benchmark/nativejson-benchmark/data/canada.json",
        "../benchmark/nativejson-benchmark/data/citm_catalog.json",
        "../benchmark/nativejson-benchmark/data/twitter.json"
    };

    const int iterations = 10;  // 每个文件测试10次取平均

    for (const auto& filepath : testFiles) {
        std::cout << "测试文件: " << filepath << std::endl;
        std::cout << "文件大小: " << getFileSize(filepath) << std::endl;

        // 读取文件
        std::string jsonStr = readFile(filepath);
        if (jsonStr.empty()) {
            std::cerr << "跳过：无法读取文件\n" << std::endl;
            continue;
        }

        std::cout << std::fixed << std::setprecision(3);

        // JConfigParser 解析
        double jcpParseTime = benchmarkJConfigParserParse(jsonStr, iterations);
        std::cout << "  JConfigParser 解析: " << jcpParseTime << " ms/次" << std::endl;

        // RapidJSON 解析
        double rjParseTime = benchmarkRapidJSONParse(jsonStr, iterations);
        std::cout << "  RapidJSON 解析:    " << rjParseTime << " ms/次" << std::endl;

        // 解析性能对比
        double parseRatio = (jcpParseTime / rjParseTime) * 100;
        std::cout << "  解析性能对比:     JConfigParser 是 RapidJSON 的 "
                  << std::fixed << std::setprecision(1) << parseRatio << "%" << std::endl;

        // 解析并序列化
        Node jcpNode = Node::fromJson(jsonStr);
        rapidjson::Document rjDoc;
        rjDoc.Parse(jsonStr.c_str());

        // JConfigParser 序列化
        double jcpSerializeTime = benchmarkJConfigParserSerialize(jcpNode, iterations);
        std::cout << "  JConfigParser 序列化: " << jcpSerializeTime << " ms/次" << std::endl;

        // RapidJSON 序列化
        double rjSerializeTime = benchmarkRapidJSONSerialize(rjDoc, iterations);
        std::cout << "  RapidJSON 序列化:    " << rjSerializeTime << " ms/次" << std::endl;

        // 序列化性能对比
        double serializeRatio = (jcpSerializeTime / rjSerializeTime) * 100;
        std::cout << "  序列化性能对比:     JConfigParser 是 RapidJSON 的 "
                  << std::fixed << std::setprecision(1) << serializeRatio << "%" << std::endl;

        std::cout << std::endl;
    }
}

// ==================== 合规测试 ====================

/**
 * @brief 运行jsonchecker目录合规测试
 */
void runConformanceTest() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   合规测试：jsonchecker 测试集" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::string jsoncheckerDir = "../benchmark/nativejson-benchmark/data/jsonchecker";
    std::vector<std::string> passFiles;
    std::vector<std::string> failFiles;

    // 收集测试文件
    if (fs::exists(jsoncheckerDir)) {
        for (const auto& entry : fs::directory_iterator(jsoncheckerDir)) {
            if (entry.path().extension() == ".json") {
                std::string filename = entry.path().filename().string();
                if (filename.find("pass") == 0) {
                    passFiles.push_back(entry.path().string());
                } else if (filename.find("fail") == 0) {
                    failFiles.push_back(entry.path().string());
                }
            }
        }
    } else {
        std::cerr << "警告：jsonchecker 目录不存在: " << jsoncheckerDir << std::endl;
        return;
    }

    // 排序
    std::sort(passFiles.begin(), passFiles.end());
    std::sort(failFiles.begin(), failFiles.end());

    // 测试 pass 文件（应该解析成功）
    std::cout << "测试 PASS 文件 (应该解析成功)：" << std::endl;
    int passSuccess = 0;
    int passFail = 0;

    for (const auto& filepath : passFiles) {
        std::string jsonStr = readFile(filepath);
        if (jsonStr.empty()) {
            passFail++;
            std::cout << "  [SKIP] " << fs::path(filepath).filename() << std::endl;
            continue;
        }

        Node node = Node::fromJson(jsonStr);
        if (node.isValid()) {
            passSuccess++;
            std::cout << "  [PASS] " << fs::path(filepath).filename() << std::endl;
        } else {
            passFail++;
            std::cout << "  [FAIL] " << fs::path(filepath).filename()
                      << " - 应该成功但失败" << std::endl;
        }
    }

    std::cout << "\nPASS 文件结果: "
              << passSuccess << "/" << passFiles.size() << " 通过, "
              << passFail << " 失败\n" << std::endl;

    // 测试 fail 文件（应该解析失败）
    std::cout << "测试 FAIL 文件（应该解析失败）：" << std::endl;
    int failSuccess = 0;
    int failFail = 0;
    int excluded = 0;

    for (const auto& filepath : failFiles) {
        std::string filename = fs::path(filepath).filename().string();

        // 跳过marked as EXCLUDE的文件（例如 fail01_EXCLUDE.json, fail18_EXCLUDE.json）
        // 这些文件在RFC7159下可能是有效的，因此在测试中被排除
        if (filename.find("EXCLUDE") != std::string::npos) {
            excluded++;
            std::cout << "  [SKIP] " << filename << " (已排除)" << std::endl;
            continue;
        }

        std::string jsonStr = readFile(filepath);
        if (jsonStr.empty()) {
            failFail++;
            std::cout << "  [SKIP] " << fs::path(filepath).filename() << std::endl;
            continue;
        }

        Node node = Node::fromJson(jsonStr);
        if (!node.isValid()) {
            failSuccess++;
            std::cout << "  [PASS] " << fs::path(filepath).filename() << std::endl;
        } else {
            failFail++;
            std::cout << "  [FAIL] " << fs::path(filepath).filename()
                      << " - 应该失败但成功" << std::endl;
        }
    }

    std::cout << "\nFAIL 文件结果: "
              << failSuccess << "/" << (failFiles.size() - excluded) << " 通过, "
              << failFail << " 失败" << " (已排除 " << excluded << " 个EXCLUDE文件)\n" << std::endl;

    // 总结
    int totalTestedFiles = passFiles.size() + (failFiles.size() - excluded);  // 排除EXCLUDE文件
    int totalSuccess = passSuccess + failSuccess;
    int totalFail = passFail + failFail;

    std::cout << "========================================" << std::endl;
    std::cout << "合规测试总结：" << std::endl;
    std::cout << "  测试文件数: " << totalTestedFiles << " (已排除 " << excluded << " 个EXCLUDE文件)" << std::endl;
    std::cout << "  成功:       " << totalSuccess << " ("
              << std::fixed << std::setprecision(1)
              << (totalSuccess * 100.0 / totalTestedFiles) << "%)" << std::endl;
    std::cout << "  失败:       " << totalFail << " ("
              << (totalFail * 100.0 / totalTestedFiles) << "%)" << std::endl;
    std::cout << "========================================" << std::endl;
}

// ==================== 往返测试 ====================

/**
 * @brief 运行roundtrip目录测试
 */
void runRoundtripTest() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   往返测试：roundtrip 测试集" << std::endl;
    std::cout << "========================================\n" << std::endl;

    std::string roundtripDir = "../benchmark/nativejson-benchmark/data/roundtrip";
    std::vector<std::string> testFiles;

    // 收集测试文件
    if (fs::exists(roundtripDir)) {
        for (const auto& entry : fs::directory_iterator(roundtripDir)) {
            if (entry.path().extension() == ".json") {
                testFiles.push_back(entry.path().string());
            }
        }
    } else {
        std::cerr << "警告：roundtrip 目录不存在: " << roundtripDir << std::endl;
        return;
    }

    if (testFiles.empty()) {
        std::cout << "未找到 roundtrip 测试文件" << std::endl;
        return;
    }

    // 排序
    std::sort(testFiles.begin(), testFiles.end());

    std::cout << "找到 " << testFiles.size() << " 个 roundtrip 测试文件\n" << std::endl;

    int successCount = 0;
    int failCount = 0;

    for (const auto& filepath : testFiles) {
        std::string filename = fs::path(filepath).filename().string();
        std::string jsonStr = readFile(filepath);

        if (jsonStr.empty()) {
            failCount++;
            std::cout << "[FAIL] " << filename << " - 无法读取文件" << std::endl;
            continue;
        }

        // 第一次解析
        Node node1 = Node::fromJson(jsonStr);
        if (!node1.isValid()) {
            failCount++;
            std::cout << "[FAIL] " << filename << " - 解析失败" << std::endl;
            continue;
        }

        // 序列化
        std::string serialized = node1.toJson(false);

        // 第二次解析
        Node node2 = Node::fromJson(serialized);
        if (!node2.isValid()) {
            failCount++;
            std::cout << "[FAIL] " << filename << " - 序列化后解析失败" << std::endl;
            continue;
        }

        // 比较两次序列化结果应该相同
        std::string serialized2 = node2.toJson(false);
        if (serialized != serialized2) {
            failCount++;
            std::cout << "[FAIL] " << filename << " - 往返结果不一致" << std::endl;
            std::cout << "  第一次: " << serialized.substr(0, 50) << "..." << std::endl;
            std::cout << "  第二次: " << serialized2.substr(0, 50) << "..." << std::endl;
            continue;
        }

        successCount++;
        std::cout << "[PASS] " << filename << std::endl;
    }

    std::cout << "\n========================================" << std::endl;
    std::cout << "往返测试总结：" << std::endl;
    std::cout << "  总文件数: " << testFiles.size() << std::endl;
    std::cout << "  成功:     " << successCount << " ("
              << std::fixed << std::setprecision(1)
              << (successCount * 100.0 / testFiles.size()) << "%)" << std::endl;
    std::cout << "  失败:     " << failCount << " ("
              << (failCount * 100.0 / testFiles.size()) << "%)" << std::endl;
    std::cout << "========================================" << std::endl;
}

// ==================== 生成性能测试 ====================

/**
 * @brief JConfigParser生成大JSON（使用setObject/setArray避免拷贝）
 * @param depth 嵌套深度
 * @param width 数组宽度
 * @return 生成的Node
 */
Node generateLargeJsonJCP(int depth, int width) {
    Node root = Node::createArray();

    // 生成宽数组
    for (int i = 0; i < width; i++) {
        Node obj = root.appendObject();

        // 添加基本字段
        obj.set("id", i);
        obj.set("name", "item_" + std::to_string(i));
        obj.set("value", i * 1.5);
        obj.set("active", (i % 2 == 0));
        obj.set("description", "This is a test item for large JSON generation");

        // 创建嵌套结构：每层都有独立的level_X对象
        for (int d = 0; d < depth; d++) {
            std::string currentKey = "level_" + std::to_string(d);

            // 使用setObject创建嵌套对象，避免拷贝
            Node nested = obj.setObject(currentKey);

            // 添加数据字段
            nested.set("index", d);
            nested.set("title", "Level " + std::to_string(d));

            // 添加多个数据字段
            for (int k = 0; k < 10; k++) {
                nested.set("field_" + std::to_string(k), std::to_string(i * 1000 + d * 100 + k));
                nested.set("num_" + std::to_string(k), (i + d + k) * 1.5);
            }

            // 如果不是最后一层，添加嵌套对象（使用setObject避免拷贝）
            if (d < depth - 1) {
                nested.setObject("nested");
            }
        }
    }

    return root;
}

/**
 * @brief RapidJSON生成大JSON
 * @param depth 嵌套深度
 * @param width 数组宽度
 * @return 生成的Document
 */
std::unique_ptr<rapidjson::Document> generateLargeJsonRJ(int depth, int width) {
    auto doc = std::make_unique<rapidjson::Document>();
    doc->SetArray();
    rapidjson::Document::AllocatorType& allocator = doc->GetAllocator();

    // 生成宽数组
    for (int i = 0; i < width; i++) {
        rapidjson::Value obj(rapidjson::kObjectType);

        // 添加基本字段
        obj.AddMember("id", i, allocator);
        obj.AddMember("name", rapidjson::Value("item_" + std::to_string(i), allocator), allocator);
        obj.AddMember("value", i * 1.5, allocator);
        obj.AddMember("active", i % 2 == 0, allocator);
        obj.AddMember("description", rapidjson::Value("This is a test item for large JSON generation", allocator), allocator);

        // 创建嵌套结构：每层都有独立的level_X对象
        for (int d = 0; d < depth; d++) {
            std::string currentKey = "level_" + std::to_string(d);
            rapidjson::Value nested(rapidjson::kObjectType);

            // 添加数据字段
            nested.AddMember("index", d, allocator);
            nested.AddMember("title", rapidjson::Value("Level " + std::to_string(d), allocator), allocator);

            // 添加多个数据字段
            for (int k = 0; k < 10; k++) {
                std::string fieldKey = "field_" + std::to_string(k);
                std::string numKey = "num_" + std::to_string(k);
                std::string fieldValue = std::to_string(i * 1000 + d * 100 + k);
                double numValue = (i + d + k) * 1.5;

                nested.AddMember(rapidjson::Value(fieldKey.c_str(), allocator), rapidjson::Value(fieldValue.c_str(), allocator), allocator);
                nested.AddMember(rapidjson::Value(numKey.c_str(), allocator), rapidjson::Value().SetDouble(numValue), allocator);
            }

            // 如果不是最后一层，添加嵌套对象
            if (d < depth - 1) {
                nested.AddMember("nested", rapidjson::Value(rapidjson::kObjectType), allocator);
            }

            // 将nested添加到obj
            obj.AddMember(rapidjson::Value(currentKey.c_str(), allocator), nested, allocator);
        }

        doc->PushBack(obj, allocator);
    }

    return doc;
}

/**
 * @brief JConfigParser生成性能测试（迭代）
 * @param depth 深度
 * @param width 宽度
 * @param iterations 迭代次数
 * @return 平均生成时间
 */
double benchmarkJCPGenerate(int depth, int width, int iterations) {
    double totalTime = 0;
    for (int i = 0; i < iterations; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        Node node = generateLargeJsonJCP(depth, width);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        totalTime += elapsed.count();
    }
    return totalTime / iterations;
}

/**
 * @brief RapidJSON生成性能测试（迭代）
 * @param depth 深度
 * @param width 宽度
 * @param iterations 迭代次数
 * @return 平均生成时间
 */
double benchmarkRJGenerate(int depth, int width, int iterations) {
    double totalTime = 0;
    for (int i = 0; i < iterations; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        auto doc = generateLargeJsonRJ(depth, width);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsed = end - start;
        totalTime += elapsed.count();
    }
    return totalTime / iterations;
}

/**
 * @brief 运行大JSON生成性能对比测试
 */
void runLargeJsonBenchmark() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "   大JSON生成性能测试：JConfigParser vs RapidJSON" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // 不同的测试配置
    std::vector<std::pair<int, int>> configs = {
        {10, 100},   // 深度10，宽度100
        {20, 50},    // 深度20，宽度50
        {15, 200},   // 深度15，宽度200
        {200, 30}     // 深度200，宽度30
    };

    const int iterations = 5;  // 每个配置测试5次取平均

    for (const auto& config : configs) {
        int depth = config.first;
        int width = config.second;

        std::cout << "测试配置: 深度=" << depth << ", 宽度=" << width << std::endl;

        // JConfigParser 生成测试
        double jcpGenTime = benchmarkJCPGenerate(depth, width, iterations);
        std::cout << "  JConfigParser 生成: " << jcpGenTime << " ms/次" << std::endl;

        // RapidJSON 生成测试
        double rjGenTime = benchmarkRJGenerate(depth, width, iterations);
        std::cout << "  RapidJSON 生成:    " << rjGenTime << " ms/次" << std::endl;

        // 生成性能对比
        double genRatio = (jcpGenTime / rjGenTime) * 100;
        std::cout << "  生成性能对比:     JConfigParser 是 RapidJSON 的 "
                  << std::fixed << std::setprecision(1) << genRatio << "%" << std::endl;

        // 生成一个JSON用于获取大小
        Node sample = generateLargeJsonJCP(depth, width);
        std::string jsonStr = sample.toJson(false);
        std::cout << "  JSON大小:          " << getFileSizeString(jsonStr.size()) << std::endl;

        std::cout << std::endl;
    }
}

// ==================== 主函数 ====================

int main(int argc, char *argv[])
{
#ifdef _WIN32
    // 设置控制台为UTF-8编码以正确显示中文
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::cout << "\n========================================" << std::endl;
    std::cout << "   JConfigParser 测试套件" << std::endl;
    std::cout << "========================================" << std::endl;

    // 性能测试
    runBenchmarkSpeed();

    // 合规测试
    runConformanceTest();

    // 往返测试
    runRoundtripTest();

    // 大JSON生成性能测试
    runLargeJsonBenchmark();

    std::cout << "\n========================================" << std::endl;
    std::cout << "   所有测试完成" << std::endl;
    std::cout << "========================================\n" << std::endl;

    return 0;
}