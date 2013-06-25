
#include "ndn-exclusion.h"
#include <boost/foreach.hpp>
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("ndn.Exclusion");

namespace ns3 {
  namespace ndn {

    Exclusion::Exclusion()
      : count (0)
    {
    }

    size_t Exclusion::GetSerializedSize() const
    {
      return (count * HASH_SIZE) + 1;
    }

    size_t Exclusion::GetMaxSerializedSize() const
    {
      return (count * HASH_SIZE) + 1;
      return (MAX_EXCLUSIONS * HASH_SIZE) + 1;
    }

    uint32_t Exclusion::Serialize(Buffer::Iterator start) const
    {
      Buffer::Iterator it = start;
      it.WriteU8(static_cast<uint8_t>(count));
      
      for (int i = 0; i < count; i++)
	{
	  it.Write(reinterpret_cast<const uint8_t*>(m_hash[i].c_str()), HASH_SIZE);
	}

      return it.GetDistanceFrom(start);
    }

    uint32_t Exclusion::Deserialize(Buffer::Iterator start)
    {
      Buffer::Iterator it = start;
      uint8_t size = it.ReadU8();
      count = 0;
      for (uint16_t i = 0; i < size; i++)
	{
	  uint8_t tmp[HASH_SIZE];
	  memset(tmp, 0, HASH_SIZE);
	  it.Read(tmp, HASH_SIZE);

	  this->Add(std::string((char*)tmp, HASH_SIZE));
	}

      NS_ASSERT (GetSerializedSize() == (it.GetDistanceFrom(start)));

      return it.GetDistanceFrom(start);
    }

    void Exclusion::Add(std::string hash)
    {
      if (count >= MAX_EXCLUSIONS)
	return;

      m_hash[count] = hash;
      count++;
    }
    
    std::vector<std::string> Exclusion::GetHashList()
    {
      std::vector<std::string> hash_list;

      for (int i = 0; i < count; i++)
	{
	  hash_list.push_back(m_hash[i]);
	}

      return hash_list;
    }

    int Exclusion::size () const
    {
      return count;
    }

    bool Exclusion::Contains (std::string digest) const
    {
      if (digest.size() != HASH_SIZE)
	{
	  return false;
	}


      for (int i = 0; i < count; i++)
	{
	  if (m_hash[i] == digest)
	    {
	      return true;
	    }
	}

      return false;
    }
  } // namespace ndn
} // namespace ns3
