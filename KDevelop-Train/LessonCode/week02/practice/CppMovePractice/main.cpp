猫猫矿工: 03-26 21:26:41
补上5个函数并实现移动的例子、

猫猫矿工: 03-26 21:26:41
构造、析构造、移动等
需要验证成功

猫猫矿工: 03-26 21:29:01
struct MyStruct {
    int a = 10;
    int *ptr = nullptr;

    MyStruct(int *num)
        : ptr(num)
    {
    }

    ~MyStruct()
    {
        if (ptr != nullptr)
            delete ptr;
    }

    MyStruct(const MyStruct &other) = default;

    MyStruct &operator =(const MyStruct &other) = default;

    MyStruct(MyStruct &&other) noexcept
    {
        if (this == &other)
            return;

        a = other.a;
        ptr = other.ptr;
        other.ptr = nullptr;
    }

    MyStruct &operator=(MyStruct &&other) noexcept
    {
        if (this == &other)
            return *this;

        a = other.a;

        if (ptr != nullptr)
            delete ptr;

        ptr = other.ptr;
        other.ptr = nullptr;

        return *this;
    }
};

int main()
{
    int *buf = new int[16];
    MyStruct s1{buf};
    assert(s1.ptr == buf);
    MyStruct s2{std::move(s1)};
    assert(s2.ptr == buf);
    assert(s1.ptr == nullptr);

    return 0;
}