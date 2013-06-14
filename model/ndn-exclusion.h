
#ifndef _NDN_EXCLUSION_H_
#define _NDN_EXCLUSION_H_

#define HASH_SIZE 20    // the hash used is SHA_1

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
      typedef std::list<std::string>::iterator       iterator;
      typedef std::list<std::string>::const_iterator const_iterator;

      Exclusion();
      Exclusion(const std::list<std::string> &components);

      size_t GetSerializedSize() const;
      uint32_t Serialize(Buffer::Iterator start) const;
      uint32_t Deserialize(Buffer::Iterator start);

      void Add(std::string hash);
      iterator begin();
      iterator end();
      const_iterator begin() const;
      const_iterator end() const;
      size_t size () const;
      bool Contains (std::string digest) const;

    private:
      std::list<std::string> m_hash;
    };    
  } // namespace ndn
} // namespace nd3

#endif // _NDN_EXCLUSION_H_
