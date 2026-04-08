#ifndef _REF_OBJECT_H_
#define _REF_OBJECT_H_

#include "lock.h"

class CRefObject {
 public:
  CRefObject();
  virtual ~CRefObject();

  void SetLock(CLock* lock) { m_lock = lock; }

  void AddRef();
  void ReleaseRef();

 private:
  int m_refCount;
  CLock* m_lock;
};

#endif  // _REF_OBJECT_H_