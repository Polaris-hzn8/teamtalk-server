/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: singleton.h
 Update Time: Tue 13 Jun 2023 18:08:36 CST
 brief: 
    单例模式模板类Singleton（非线程安全）
    使用该模板类可以方便地实现各种类的单例模式
*/

#ifndef BASE_SINGLETON_H_
#define BASE_SINGLETON_H_

template<typename T>
class Singleton
{
public:
    static T& Instance() {
        if(Singleton::s_instance == NULL)
            Singleton::s_instance = CreateInstance();
        return *(Singleton::s_instance);
    }
    static T* GetInstance() {
        if(Singleton::s_instance == NULL)
            Singleton::s_instance = CreateInstance();
        return Singleton::s_instance;
    }
    static T* getInstance() {
        if(Singleton::s_instance == NULL)
            Singleton::s_instance = CreateInstance();
        return Singleton::s_instance;
    }
    static void Destroy() {
        if(Singleton::s_instance != NULL) {
            DestroyInstance(Singleton::s_instance);
            Singleton::s_instance=0;
        }
    }
    
protected:
    Singleton()	{
        Singleton::s_instance = static_cast<T*>(this);
    }
    ~Singleton() {
        Singleton::s_instance = NULL;
    }
    
private:
    static T* CreateInstance(){ return new T(); }
    static void DestroyInstance(T* p) { delete p; }

    static T* s_instance;
    
private:
    // 默认禁止拷贝和赋值操作
    explicit Singleton(Singleton const &) { }
    Singleton& operator=(Singleton const&) { return *this; }
};

template<typename T>
T* Singleton<T>::s_instance = NULL;

#endif

