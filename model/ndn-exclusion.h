
#ifndef _NDN_EXCLUSION_H_
#define _NDN_EXCLUSION_H_

#define HASH_SIZE 40    // the hash used is SHA_1 and it is represented in hex
#define MAX_EXCLUSIONS 10


#include "ns3/simple-ref-count.h"
#include "ns3/buffer.h"
#include "ns3/attribute.h"
#include "ns3/attribute-helper.h"

#include <string>
#include <list>

namespace ns3 {
  namespace ndn {

    class Exclusion : public SimpleRefCount<Exclusion>
    {
    public:
      typedef std::list<char*>::iterator       iterator;
      typedef std::list<char*>::const_iterator const_iterator;

      Exclusion();

      size_t GetSerializedSize() const;
      size_t GetMaxSerializedSize() const;
      uint32_t Serialize(Buffer::Iterator start) const;
      uint32_t Deserialize(Buffer::Iterator start);

      void Add(char* hash);
      iterator begin();
      iterator end();
      const_iterator begin() const;
      const_iterator end() const;
      size_t size () const;
      bool Contains (std::string digest) const;

    /* private: */
      std::list<char*> m_hash;
    };    
  } // namespace ndn
} // namespace nd3

#endif // _NDN_EXCLUSION_H_
