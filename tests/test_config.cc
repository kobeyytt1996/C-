/**
 * @file test_config.cc
 * @author your name (you@domain.com)
 * @brief 配置系统的测试代码
 * @version 0.1
 * @date 2022-03-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "../yuan/config.h"
#include "../yuan/log.h"
// 使用了yaml的三方库：https://github.com/jbeder/yaml-cpp/releases/tag/yaml-cpp-0.6.0
#include <yaml-cpp/yaml.h>

// 通过Config约定基础的配置项。这里写system.port，是因为代码里解析yaml的时候是用.连接的，看bin/conf/log.txt即可理解
// 配置系统的原则：约定优于配置。这里就是提前约定了值，大部分条目约定好了之后就不需要改，少部分的需通过yaml等配置来修改
// 比如yaml配置里写了很多项，但只有和我这里约定的条目相同，我才会使用其值。
yuan::ConfigVar<int>::ptr g_int_value_config =
    yuan::Config::Lookup("system.port", (int)8080, "system port");

yuan::ConfigVar<float>::ptr g_float_value_config = 
    yuan::Config::Lookup("system.value", static_cast<float>(10.21), "system value");
// 较复杂的值类型
yuan::ConfigVar<std::vector<int>>::ptr g_vec_int_value_config = 
    yuan::Config::Lookup("system.int_vec", std::vector<int>{0}, "system int_vec");

yuan::ConfigVar<std::list<int>>::ptr g_list_int_value_config = 
    yuan::Config::Lookup("system.int_list", std::list<int>{0}, "system int_list");

yuan::ConfigVar<std::set<int>>::ptr g_set_int_value_config = 
    yuan::Config::Lookup("system.int_set", std::set<int>{0}, "system int_set");

yuan::ConfigVar<std::unordered_set<int>>::ptr g_unordered_set_int_value_config = 
    yuan::Config::Lookup("system.int_unordered_set", std::unordered_set<int>{0}, "system int_unordered_set");

yuan::ConfigVar<std::map<std::string, int>>::ptr g_map_int_value_config = 
    yuan::Config::Lookup("system.int_map", std::map<std::string, int>{{"yuan", 10}}, "system int_map");

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

void test_config() {
    YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << "before: " << g_int_value_config->getValue();
    YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << "before: " << g_float_value_config->getValue();
#define XX(g_val, name, prefix) \
    for (auto i : g_val->getValue()) { \
        YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << #prefix " " #name " : " << i; \
    }  \
    YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << #prefix " " #name " yaml: " << g_val->toString();
#define XX_M(g_val, name, prefix) \
    for (auto i : g_val->getValue()) { \
        YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << #prefix " " #name " : { "  \
                                << i.first << " - " << i.second << "}"; \
    }  \
    YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << #prefix " " #name " yaml: " << g_val->toString();
    
    XX(g_vec_int_value_config, int_vec, before);
    XX(g_list_int_value_config, int_list, before);
    XX(g_set_int_value_config, int_set, before);
    XX(g_unordered_set_int_value_config, int_unordered_set, before);
    XX_M(g_map_int_value_config, int_map, before);

    YAML::Node root = YAML::LoadFile("/home/yuan/workspace/yuan/bin/conf/log.yml");
    yuan::Config::LoadFromYaml(root);

    XX(g_vec_int_value_config, int_vec, after);
    XX(g_list_int_value_config, int_list, after);
    XX(g_set_int_value_config, int_set, after);
    XX(g_unordered_set_int_value_config, int_unordered_set, after);
    XX_M(g_map_int_value_config, int_map, after);

#undef XX
    YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << "after: " << g_int_value_config->getValue();
    YUAN_LOG_INFO(YUAN_GET_ROOT_LOGGER()) << "after: " << g_float_value_config->getValue();
}

int main() {
    // test_yaml();
    test_config();

    return 0;
}