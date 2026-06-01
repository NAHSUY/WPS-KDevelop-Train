#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

class Configure {
public:
    int addItem(std::string str) {
        for (int i = 0; i < (int)m_vItems.size(); ++i) {
            if (m_vItems[i] == str) {
                return i;
            }
        }
        m_vItems.push_back(str);
        return (int)m_vItems.size() - 1;
    }

    std::string getItem(int index) {
        if (index < 0 || index >= (int)m_vItems.size()) {
            return "";
        }
        return m_vItems[index];
    }

    int getSize() const {
        return (int)m_vItems.size();
    }

private:
    std::vector<std::string> m_vItems;
};

#define TEST(name) \
    struct Test_##name { Test_##name() { std::cout << "TEST: " #name << std::endl; run(); } void run(); } test_##name; \
    void Test_##name::run()

#define TEST_F(name) TEST(name)

int main() {
    std::cout << "Running tests..." << std::endl;
    return 0;
}

// TEST 1: 测试 addItem
TEST(AddItem) {
    Configure cfg;
    int idx1 = cfg.addItem("hello");
    int idx2 = cfg.addItem("world");
    int idx3 = cfg.addItem("hello"); // 重复添加

    if (idx1 != 0 || idx2 != 1 || idx3 != 0) {
        std::cout << "  FAILED: addItem index mismatch" << std::endl;
    } else {
        std::cout << "  PASSED" << std::endl;
    }
}

// TEST 2: 测试 getItem
TEST(GetItem) {
    Configure cfg;
    cfg.addItem("apple");
    cfg.addItem("banana");

    if (cfg.getItem(0) != "apple" || cfg.getItem(1) != "banana") {
        std::cout << "  FAILED: getItem returned wrong value" << std::endl;
    } else if (cfg.getItem(-1) != "" || cfg.getItem(100) != "") {
        std::cout << "  FAILED: getItem should return empty for invalid index" << std::endl;
    } else {
        std::cout << "  PASSED" << std::endl;
    }
}

// TEST 3: 测试 getSize
TEST(GetSize) {
    Configure cfg;
    if (cfg.getSize() != 0) {
        std::cout << "  FAILED: initial size should be 0" << std::endl;
        return;
    }
    cfg.addItem("one");
    cfg.addItem("two");
    cfg.addItem("one"); // 重复，不应增加大小

    if (cfg.getSize() != 2) {
        std::cout << "  FAILED: size should be 2, got " << cfg.getSize() << std::endl;
    } else {
        std::cout << "  PASSED" << std::endl;
    }
}
