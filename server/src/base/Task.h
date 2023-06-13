/*
 Reviser: Polaris_hzn8
 Email: 3453851623@qq.com
 filename: Task.h
 Update Time: Tue 13 Jun 2023 09:01:22 CST
 brief: 
*/

#ifndef __TASK_H__
#define __TASK_H__

/**
 * CTask类是一个抽象基类，用于表示任务的概念
 * 只能用作其他具体任务类的基类 派生类必须实现run()函数来定义任务的具体逻辑
 * 通过派生类的实现，可以创建不同类型的任务对象，并通过调用run()函数来执行任务的逻辑
 * 
 * 这样的设计允许在任务执行过程中的多态行为，以便针对不同的任务类型执行不同的操作
*/
class CTask {
public:
    CTask(){}
    virtual ~CTask(){}
    
    virtual void run() = 0;
private:
};

#endif
