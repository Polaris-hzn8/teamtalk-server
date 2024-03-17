/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: singleton.h
 Update Time: Tue 13 Jun 2023 18:08:36 CST
 brief: 
    模板类Singleton 用于实现单例模式（非线程安全）
    使用该模板类可以方便地实现各种类的单例模式
    通过调用 Instance()、GetInstance() 或 getInstance() 方法获取单例对象的引用或指针，并通过 Destroy() 方法销毁单例对象
*/

#ifndef BASE_SINGLETON_H_
#define BASE_SINGLETON_H_

template<typename T>
class Singleton {
public:
    /**
     * 提供了静态的 Instance()、GetInstance() 和 getInstance() 方法来获取单例对象的引用或指针 这三个方法的区别在于命名风格
     * 这三个方法都用于获取单例对象的引用或指针，它们的实现逻辑是相同的：
     *  1.首先检查静态成员变量 s_instance 是否为 nullptr，即单例对象是否已经被创建
     *  2.如果s_instance是nullptr，则调用CreateInstance()方法创建单例对象，并将其赋值给 s_instance
     *  3.返回s_instance的引用或指针
    */
    static T& Instance() {
        if(Singleton::s_instance==0) Singleton::s_instance = CreateInstance();
        return *(Singleton::s_instance);
    }
    static T* GetInstance() {
        if(Singleton::s_instance==0) Singleton::s_instance = CreateInstance();
        return Singleton::s_instance;
    }
    static T* getInstance() {
        if(Singleton::s_instance==0) Singleton::s_instance = CreateInstance();
        return Singleton::s_instance;
    }
    /**
     * 提供了静态的 Destroy() 方法用于销毁单例对象,具体的实现逻辑如下：
     *  1.检查静态成员变量 s_instance 是否为 nullptr，即单例对象是否存在
     *  2.如果 s_instance 不为 nullptr，则调用 DestroyInstance() 方法销毁单例对象
     *  3.将 s_instance 设置为 nullptr，表示单例对象已被销毁
    */
    static void Destroy() {
        if(Singleton::s_instance!=0) {
            DestroyInstance(Singleton::s_instance);
            Singleton::s_instance=0;
        }
    }
    
protected:
    /**
     * 构造函数和析构函数被保护起来，确保只能通过单例模式获取对象，并防止外部直接创建对象
     * 这里的构造函数和析构函数是保护（protected）的，因此只能在派生类中使用，不能直接在外部实例化或销毁单例对象
     * 这是为了确保单例对象的控制权和唯一性，只能通过 Instance() 或 GetInstance() 方法来获取单例对象的引用或指针
    */
    Singleton()	{
        //构造函数在创建单例对象时被调用
        //将当前对象的指针转换为 T* 类型，并将其赋值给静态成员变量 s_instance
        //确保单例对象在整个程序中只有一个实例，并且可以通过静态成员函数访问该实例
        Singleton::s_instance = static_cast<T*>(this);
    }
    ~Singleton() {
        //析构函数在销毁单例对象时被调用
        //将静态成员变量 s_instance 设置为 nullptr，表示单例对象已被销毁
        Singleton::s_instance = 0;
    }
    
private:
    //提供了私有的 CreateInstance() 和 DestroyInstance() 方法，用于创建和销毁单例对象
    static T* CreateInstance(){ return new T(); }
    static void DestroyInstance(T* p) { delete p; }
    
private:
    //通过静态成员变量 s_instance 来保存单例对象的指针
    static T *s_instance;
    
private:
    //实现私有的拷贝构造函数和赋值操作符，默认禁止拷贝和赋值操作
    /**
     * 私有的拷贝构造函数确保无法从单例对象创建新的副本
     * 构造函数的参数是 Singleton const & 类型，表示拷贝构造函数接受一个常量引用
     * 将构造函数声明为 explicit 是为了避免隐式的拷贝构造，只能在类内部访问
    */
    explicit Singleton(Singleton const &) { }
    /**
     * 私有的赋值操作符确保无法通过赋值操作将一个单例对象赋值给另一个对象
     * 赋值操作符的参数是 Singleton const & 类型，表示赋值操作符接受一个常量引用
     * 在赋值操作符的实现中，直接返回 *this，即当前对象的引用
    */
    Singleton& operator=(Singleton const&) { return *this; }
};

template<typename T>
T* Singleton<T>::s_instance=0;

#endif



