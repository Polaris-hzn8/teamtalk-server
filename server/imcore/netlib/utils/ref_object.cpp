
#include "ref_object.h"

// CRefObject实现
CRefObject::CRefObject() : m_refCount(1) {}

CRefObject::~CRefObject() {}

void CRefObject::AddRef() {
  m_refCount.fetch_add(1, std::memory_order_relaxed);
}

void CRefObject::ReleaseRef() {
  int oldCount = m_refCount.fetch_sub(1, std::memory_order_acq_rel);
  if (oldCount == 1) {
    delete this;
  }
}