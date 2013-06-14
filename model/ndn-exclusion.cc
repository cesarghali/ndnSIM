
#include "ndn-exclusion.h"
#include <boost/foreach.hpp>
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("ndn.Exclusion");

namespace ns3 {
  namespace ndn {

    Exclusion::Exclusion()
    {
    }
    
    Exclusion::Exclusion(const std::list<std::string> &components)
    {
      BOOST_FOREACH (const std::string &component, components)
	{
	  Add(component);
	}
    }

    size_t Exclusion::GetSerializedSize() const
    {
      return (this->size() * HASH_SIZE) + 2;
    }

    uint32_t Exclusion::Serialize(Buffer::Iterator start) const
    {
      Buffer::Iterator it = start;
      it.WriteU16(static_cast<uint16_t>(this->size()));
      
      Exclusion::const_iterator item = this->begin();
      while (item != this->end())
	{
	  it.Write(reinterpret_cast<const uint8_t*>(item->c_str()), HASH_SIZE);
	  item++;
	}

      return it.GetDistanceFrom(start);
    }

    uint32_t Exclusion::Deserialize(Buffer::Iterator start)
    {
      Buffer::Iterator it = start;
      uint16_t size = it.ReadU16();
      for (uint16_t i = 0; i < size; i++)
	{
	  uint16_t length = HASH_SIZE;
	  uint8_t tmp[length];
	  it.Read(tmp, length);

	  this->Add(std::string(reinterpret_cast<const char*>(tmp), HASH_SIZE));
	}

      return it.GetDistanceFrom(start);
    }

    void Exclusion::Add(std::string hash)
    {
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
      BOOST_FOREACH (const std::string &hash, m_hash)
	{
	  if (hash == digest)
	    {
	      return true;
	    }
	}

      return false;
    }
  } // namespace ndn
} // namespace ns3
