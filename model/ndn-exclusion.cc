
#include "ndn-exclusion.h"
#include <boost/foreach.hpp>
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("ndn.Exclusion");

namespace ns3 {
  namespace ndn {

    Exclusion::Exclusion()
    {
    }

    size_t Exclusion::GetSerializedSize() const
    {
      return (this->size() * HASH_SIZE) + 1;
    }

    size_t Exclusion::GetMaxSerializedSize() const
    {
      return (MAX_EXCLUSIONS * 40) + 1;
    }

    uint32_t Exclusion::Serialize(Buffer::Iterator start) const
    {
      Buffer::Iterator it = start;
      it.WriteU8(static_cast<uint8_t>(this->size()));
      
      Exclusion::const_iterator item = this->begin();
      while (item != this->end())
	{
	  it.Write(reinterpret_cast<const uint8_t*>(*item), HASH_SIZE);
	  item++;
	}

      return it.GetDistanceFrom(start);
    }

    uint32_t Exclusion::Deserialize(Buffer::Iterator start)
    {
      Buffer::Iterator it = start;
      uint8_t size = it.ReadU8();
      for (uint16_t i = 0; i < size; i++)
	{
	  uint8_t tmp[40];
	  it.Read(tmp, 40);

	  this->Add(reinterpret_cast<char*>(tmp));
	}

      NS_ASSERT (GetSerializedSize() == (it.GetDistanceFrom(start)));

      return it.GetDistanceFrom(start);
    }

    void Exclusion::Add(char* hash)
    {
      if (this->size() >= MAX_EXCLUSIONS)
	return;

      m_hash.push_back(hash);
    }

    Exclusion::iterator Exclusion::begin()
    {
      return m_hash.begin();
    }

    Exclusion::iterator Exclusion::end()
    {
      return m_hash.end();
    }

    Exclusion::const_iterator Exclusion::begin() const
    {
      return m_hash.begin();
    }

    Exclusion::const_iterator Exclusion::end() const
    {
      return m_hash.end();
    }

    size_t Exclusion::size () const
    {
      return m_hash.size();
    }

    bool Exclusion::Contains (std::string digest) const
    {
      if (digest.size() != HASH_SIZE)
	return false;

      BOOST_FOREACH (char* hash, m_hash)
	{
	  if (memcmp(hash, digest.c_str(), HASH_SIZE) == 0)
	    {
	      return true;
	    }
	}

      return false;
    }
  } // namespace ndn
} // namespace ns3
