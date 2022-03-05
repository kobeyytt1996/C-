#include "../yuan/config.h"
#include "../yuan/log.h"
// 使用了yaml的三方库：https://github.com/jbeder/yaml-cpp/releases/tag/yaml-cpp-0.6.0
#include <yaml-cpp/yaml.h>

// 通过Config获得基础的配置项
yuan::ConfigVar<int>::ptr g_int_value_config =
    yuan::Config::Lookup("system.port", (int)8080, "system.port");

// 遍历yaml的node的示例，看注释了解如何解析yaml中的符号
void print_yaml(const YAML::Node &node, int level) {
    // :是一个map，其右边是一个scalar node
    if (node.IsScalar()) {
        YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << node.Scalar() << "-" << node.Type() << " - " << level;
    } else if (node.IsNull()) {
        YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << "NULL - " << node.Type() << " - " << level;
    } 
    // 多个并列的:是一个map，it->first即每个:左边的键值，it->second即每个:右边对应的node
    else if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } 
    // - 即代表列表
    else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); ++i)
        {
            YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}

// 使用yaml三方库解析的方式
void test_yaml() {
    YAML::Node root = YAML::LoadFile("/home/yuan/workspace/yuan/bin/conf/log.yml");
    print_yaml(root, 0);
    // YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << root;
}

int main() {
    YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << g_int_value_config->getValue();
    YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << g_int_value_config->toString();

    test_yaml();

    return 0;
}