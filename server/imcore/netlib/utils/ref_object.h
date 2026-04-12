#ifndef _REF_OBJECT_H_
#define _REF_OBJECT_H_

namespace network {

// 引用计数对象类
class NETWORK_DLL CRefObject {
  public:
   CRefObject();
   virtual ~CRefObject();
 
   void AddRef();
   void ReleaseRef();
 
  private:
   std::atomic<int> m_refCount;
};

}// namespace network

#endif  // _REF_OBJECT_H_