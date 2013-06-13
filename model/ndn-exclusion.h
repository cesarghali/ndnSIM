
#ifndef _NDN_EXCLUSION_H_
#define _NDN_EXCLUSION_H_

#define HASH_SIZE 20    // the hash used is SHA_1

#include "ns3/simple-ref-count.h"
#include "ns3/buffer.h"

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

      inline void Add(std::string hash);
      inline iterator begin();
      inline iterator end();
      inline const_iterator begin() const;
      inline const_iterator end() const;
      inline size_t size () const;

    private:
      std::list<std::string> m_hash;
    };
    
  } // namespace ndn
} // namespace nd3

#endif // _NDN_EXCLUSION_H_
