
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
      return (MAX_EXCLUSIONS * HASH_SIZE) + 1;
    }

    uint32_t Exclusion::Serialize(Buffer::Iterator start) const
    {
      Buffer::Iterator it = start;
      it.WriteU8(static_cast<uint8_t>(count));
      
      for (int i = 0; i < count; i++)
	{
	  it.Write(reinterpret_cast<const uint8_t*>(m_hash[i]), HASH_SIZE);
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

	  this->Add((char*)tmp);
	}

      NS_ASSERT (GetSerializedSize() == (it.GetDistanceFrom(start)));

      return it.GetDistanceFrom(start);
    }

    void Exclusion::Add(char* hash)
    {
      if (count >= MAX_EXCLUSIONS)
	return;

      if (Contains(hash) == true)
	return;

      memset(m_hash[count], 0, HASH_SIZE + 1);
      memcpy(m_hash[count], hash, HASH_SIZE);
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

    bool Exclusion::Contains (char* hash) const
    {
      if (hash[0] == 0)
	{
	  return false;
	}

      for (int i = 0; i < count; i++)
	{
	  if (memcmp(m_hash[i], hash, HASH_SIZE) == 0)
	    {
	      return true;
	    }
	}

      return false;
    }
  } // namespace ndn
} // namespace ns3
