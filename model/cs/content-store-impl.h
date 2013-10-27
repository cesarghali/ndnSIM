/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef NDN_CONTENT_STORE_IMPL_H_
#define NDN_CONTENT_STORE_IMPL_H_

#include "ndn-content-store.h"
#include "ns3/packet.h"
#include "ns3/ndn-interest.h"
#include "ns3/ndn-content-object.h"
#include "ns3/ndn-face.h"
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/double.h"
#include "ns3/boolean.h"

#include "../../utils/trie/trie-with-policy.h"

#include <sys/time.h>

namespace ns3 {
namespace ndn {
namespace cs {

template<class CS>
class EntryImpl : public Entry
{
public:
  typedef Entry base_type;

public:
  EntryImpl (Ptr<ContentStore> cs, Ptr<const ContentObject> header, Ptr<const Packet> packet)
    : Entry (cs, header, packet)
    , item_ (0)
  {
  }

  void
  SetTrie (typename CS::super::iterator item)
  {
    item_ = item;
  }

  typename CS::super::iterator to_iterator () { return item_; }
  typename CS::super::const_iterator to_iterator () const { return item_; }

private:
  typename CS::super::iterator item_;
};


template<class Policy>
class ContentStoreImpl : public ContentStore,
                         protected ndnSIM::trie_with_policy< Name,
                                                             ndnSIM::smart_pointer_payload_traits< EntryImpl< ContentStoreImpl< Policy > >, Entry >,
                                                             Policy >
{
public:
  typedef ndnSIM::trie_with_policy< Name,
                                    ndnSIM::smart_pointer_payload_traits< EntryImpl< ContentStoreImpl< Policy > >, Entry >,
                                    Policy > super;

  typedef EntryImpl< ContentStoreImpl< Policy > > entry;

  static TypeId
  GetTypeId ();

  ContentStoreImpl () { };
  virtual ~ContentStoreImpl () { };

  // from ContentStore

  virtual inline boost::tuple<Ptr<Packet>, Ptr<const ContentObject>, Ptr<const Packet> >
  Lookup (Ptr<const Interest> interest, Ptr<Face> inFace = NULL);

  virtual inline bool
  Add (Ptr<const ContentObject> header, Ptr<const Packet> packet);

  virtual inline void
  PopulateSingle (ContentType type);

  virtual inline void
  Populate ();

  virtual inline void
  Populate (std::string badContentName, Time badContentFreshness, uint32_t badContentCount, uint32_t badContentPayloadSize);

  // virtual bool
  // Remove (Ptr<Interest> header);

  virtual inline void
  Print (std::ostream &os) const;

  virtual uint32_t
  GetSize () const;

  virtual Ptr<Entry>
  Begin ();

  virtual Ptr<Entry>
  End ();

  virtual Ptr<Entry>
  Next (Ptr<Entry>);

private:
  void
  SetMaxSize (uint32_t maxSize);

  uint32_t
  GetMaxSize () const;

  void
  SetDisableRanking (bool disableRanking);

  bool
  GetDisableRanking () const;

  void
  SetRandomizedBadContent (bool randomizedBadContent);

  bool
  GetRandomizedBadContent () const;

  void
  SetRateAtTimeout (double rateAtTimeout);

  double
  GetRateAtTimeout () const;

  void
  SetExclusionDiscardedTimeout (double exclusionDiscardedTimeout);

  double
  GetExclusionDiscardedTimeout () const;

  void
  SetSearchCacheAfter (int time);

  int
  GetSearchCacheAfter () const;

  void
  SetBadContentName (std::string badContentName);

  std::string
  GetBadContentName () const;

  void
  SetBadContentFreshness (Time badContentFreshness);

  Time
  GetBadContentFreshness () const;

  void
  SetBadContentCount (uint32_t badContentCount);

  uint32_t
  GetBadContentCount () const;

  void
  SetBadContentPayloadSize (uint32_t badContentPayloadSize);

  uint32_t
  GetBadContentPayloadSize () const;

  void
  SetBadContentRate (double badContentRate);

  double
  GetBadContentRate () const;

private:
  static LogComponent g_log; ///< @brief Logging variable
  boost::unordered_map<std::string, int> name_count;
  bool disable_ranking;
  double rate_at_timeout;
  double exclusion_discarded_timeout;
  double search_cache_after;
  std::string bad_content_name;
  Time bad_content_freshness;
  uint32_t bad_content_count;
  uint32_t bad_content_payload_size;
  double bad_content_rate;
  bool randomized_bad_content;

  /// @brief trace of for entry additions (fired every time entry is successfully added to the cache): first parameter is pointer to the CS entry
  TracedCallback< Ptr<const Entry> > m_didAddEntry;
};

//////////////////////////////////////////
////////// Implementation ////////////////
//////////////////////////////////////////


template<class Policy>
LogComponent ContentStoreImpl< Policy >::g_log = LogComponent (("ndn.cs." + Policy::GetName ()).c_str ());


template<class Policy>
TypeId
ContentStoreImpl< Policy >::GetTypeId ()
{
  static TypeId tid = TypeId (("ns3::ndn::cs::"+Policy::GetName ()).c_str ())
    .SetGroupName ("Ndn")
    .SetParent<ContentStore> ()
    .AddConstructor< ContentStoreImpl< Policy > > ()
    .AddAttribute ("MaxSize",
                   "Set maximum number of entries in ContentStore. If 0, limit is not enforced",
                   StringValue ("100"),
                   MakeUintegerAccessor (&ContentStoreImpl< Policy >::GetMaxSize,
                                         &ContentStoreImpl< Policy >::SetMaxSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("RateAtTimeout",
                   "Set the cache entry rate at timeout",
                   DoubleValue (0.01),
                   MakeDoubleAccessor (&ContentStoreImpl< Policy >::GetRateAtTimeout,
                                       &ContentStoreImpl< Policy >::SetRateAtTimeout),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ExclusionDiscardedTimeout",
                   "Set the time in seconds when the last exclusion will be fully discarded",
                   DoubleValue (-1),
                   MakeDoubleAccessor (&ContentStoreImpl< Policy >::GetExclusionDiscardedTimeout,
                                       &ContentStoreImpl< Policy >::SetExclusionDiscardedTimeout),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("DisableRanking",
                   "If set, the first content that matches the name in CS and is not excluded will be served. No content ranking will be applied.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&ContentStoreImpl< Policy >::GetDisableRanking,
                                        &ContentStoreImpl< Policy >::SetDisableRanking),
                   MakeBooleanChecker ())
    .AddAttribute ("SearchCacheAfter",
                   "Start seaching the cache after the number of seconds specified by this attribute",
                   IntegerValue (0),
                   MakeUintegerAccessor (&ContentStoreImpl< Policy >::GetSearchCacheAfter,
                                         &ContentStoreImpl< Policy >::SetSearchCacheAfter),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("BadContentName",
                   "Set the bad content name of the content objects that will be injected in the cache at initialization time",
                   StringValue (""),
                   MakeStringAccessor (&ContentStoreImpl< Policy >::GetBadContentName,
                                       &ContentStoreImpl< Policy >::SetBadContentName),
                   MakeStringChecker ())
    .AddAttribute ("BadContentFreshness",
                   "Freshness of bad content objects, if 0, then unlimited freshness",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&ContentStoreImpl< Policy >::GetBadContentFreshness,
                                     &ContentStoreImpl< Policy >::SetBadContentFreshness),
                   MakeTimeChecker ())
    .AddAttribute ("BadContentCount",
                   "Set the number of bad content objects to be injected",
                   StringValue ("0"),
                   MakeUintegerAccessor (&ContentStoreImpl< Policy >::GetBadContentCount,
                                         &ContentStoreImpl< Policy >::SetBadContentCount),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("BadContentPayloadSize",
                   "Virtual payload size for Content packets",
                   UintegerValue (1024),
                   MakeUintegerAccessor(&ContentStoreImpl< Policy >::GetBadContentPayloadSize,
                                        &ContentStoreImpl< Policy >::SetBadContentPayloadSize),
                   MakeUintegerChecker<uint32_t>())
    .AddAttribute ("BadContentRate",
                   "Indicate the probability of which the producer is going to generate a bad content (with a bogus hash)",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&ContentStoreImpl< Policy >::GetBadContentRate,
                                       &ContentStoreImpl< Policy >::SetBadContentRate),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RandomizedBadContent",
                   "If set, a random seed will be used when randomizing bad content",
                   BooleanValue (true),
                   MakeBooleanAccessor (&ContentStoreImpl< Policy >::GetRandomizedBadContent,
                                        &ContentStoreImpl< Policy >::SetRandomizedBadContent),
                   MakeBooleanChecker ())

    .AddTraceSource ("DidAddEntry", "Trace fired every time entry is successfully added to the cache",
                     MakeTraceSourceAccessor (&ContentStoreImpl< Policy >::m_didAddEntry))
    ;

  return tid;
}

template<class Policy>
boost::tuple<Ptr<Packet>, Ptr<const ContentObject>, Ptr<const Packet> >
ContentStoreImpl<Policy>::Lookup (Ptr<const Interest> interest, Ptr<Face> inFace)
{
  if (Simulator::Now().GetSeconds() < search_cache_after)
    {
      NS_LOG_FUNCTION ("Skip lookuping up in the cache");
      return boost::tuple<Ptr<Packet>, Ptr<ContentObject>, Ptr<Packet> > (0, 0, 0);
    }

  NS_LOG_FUNCTION (this << interest->GetName ());

  // Read the exclusion filter if exists in the interest
  ns3::Ptr<const Exclusion> exclusionFilter = interest->GetExclusionPtr ();

  // Process statistics about this content name
  int count = 1;
  std::pair<boost::unordered_map<std::string, int>::iterator, bool> pair = name_count.insert(std::make_pair (interest->GetName ().ToString (), count));
  if (pair.second == false)
    {
      pair.first->second++;
      count = pair.first->second;
    }

  /// @todo Change to search with predicate
  typename super::const_iterator node = this->deepest_prefix_match (interest->GetName (), exclusionFilter, disable_ranking, count, inFace);

  if (node != this->end ())
    {
      this->m_cacheHitsTrace (interest, node->payload ()->GetHeader ());

      // NS_LOG_DEBUG ("cache hit with " << node->payload ()->GetHeader ()->GetName ());
      return boost::make_tuple (node->payload ()->GetFullyFormedNdnPacket (),
                                node->payload ()->GetHeader (),
                                node->payload ()->GetPacket ());
    }
  else
    {
      // NS_LOG_DEBUG ("cache miss for " << interest->GetName ());
      this->m_cacheMissesTrace (interest);
      return boost::tuple<Ptr<Packet>, Ptr<ContentObject>, Ptr<Packet> > (0, 0, 0);
    }
}

template<class Policy>
bool
ContentStoreImpl<Policy>::Add (Ptr<const ContentObject> header, Ptr<const Packet> packet)
{
  // If maximum size is -1, do not add
  if (GetMaxSize() == (sizeof(uint32_t) * 256) - 1)
    {
      return true;
    }

  NS_LOG_FUNCTION (this << header->GetName ());

  // Calculate the SHA_1 has of the content object
  std::string strhash = header->GetHash ();

  Ptr< entry > newEntry = Create< entry > (this, header, packet);
  std::pair< typename super::iterator, bool > result = super::insert (header->GetName (), newEntry, const_cast<char*>(strhash.c_str()), header->GetFreshness ().GetSeconds (),
                                                                      rate_at_timeout, exclusion_discarded_timeout);

  if (result.first != super::end ())
    {
      if (result.second)
        {
          newEntry->SetTrie (result.first);

          m_didAddEntry (newEntry);
          return true;
        }
      else
        {
          // should we do anything?
          // update payload? add new payload?
          return false;
        }
    }
  else
    return false; // cannot insert entry
}

template<class Policy>
void
ContentStoreImpl<Policy>::PopulateSingle (ContentType type)
{
  static ContentObjectTail tail;

  Ptr<ContentObject> header = Create<ContentObject> ();
  header->SetName (Create<Name> (bad_content_name));
  header->SetFreshness (bad_content_freshness);
  // Add timestamp
  header->SetTimestamp(Simulator::Now());
  // Add signature
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if (randomized_bad_content == true)
    {
      srand((int)Simulator::Now().GetNanoSeconds() + tv.tv_usec);
    }
  else
    {
      srand(0);
    }
  header->SetSignature(rand());
  // Computing the hash in the content header
  header->SetHash(header->ComputeHash());
  // Produce a bad content
  if (type == BAD)
    {
      header->SetSignature(rand());
    }
  else
    {
      double r = (double)rand() / RAND_MAX;
      if (bad_content_rate != 0 && r <= bad_content_rate)
        {
          header->SetSignature(rand());
        }
    }

  Ptr<Packet> packet = Create<Packet> (bad_content_payload_size);
  packet->AddHeader (*header);
  packet->AddTrailer (tail);

  Add (header, packet);
}

template<class Policy>
void
ContentStoreImpl<Policy>::Populate ()
{
  for (uint32_t i = 0; i < bad_content_count; i++)
    {
      PopulateSingle(ANY);
    }
}

template<class Policy>
void
ContentStoreImpl<Policy>::Populate (std::string badContentName, Time badContentFreshness, uint32_t badContentCount, uint32_t badContentPayloadSize)
{
  SetBadContentName(badContentName);
  SetBadContentFreshness(badContentFreshness);
  SetBadContentCount(badContentCount);
  SetBadContentPayloadSize(badContentPayloadSize);

  Populate();
}

template<class Policy>
void
ContentStoreImpl<Policy>::Print (std::ostream &os) const
{
  for (typename super::policy_container::const_iterator item = this->getPolicy ().begin ();
       item != this->getPolicy ().end ();
       item++)
    {
      os << item->payload ()->GetName () << std::endl;
    }
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetMaxSize (uint32_t maxSize)
{
  this->getPolicy ().set_max_size (maxSize);
}

template<class Policy>
uint32_t
ContentStoreImpl<Policy>::GetMaxSize () const
{
  return this->getPolicy ().get_max_size ();
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetDisableRanking (bool disableRanking)
{
  this->disable_ranking = disableRanking;
}

template<class Policy>
bool
ContentStoreImpl<Policy>::GetDisableRanking () const
{
  return this->disable_ranking;
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetRandomizedBadContent (bool randomizedBadContent)
{
  this->randomized_bad_content = randomizedBadContent;
}

template<class Policy>
bool
ContentStoreImpl<Policy>::GetRandomizedBadContent () const
{
  return this->randomized_bad_content;
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetRateAtTimeout (double rateAtTimeout)
{
  NS_ASSERT_MSG (rateAtTimeout >= 0.01 && rateAtTimeout <= 1, "Rate at timeout should be at least 0.01 and at most 1");

  this->rate_at_timeout = rateAtTimeout;
}

template<class Policy>
double
ContentStoreImpl<Policy>::GetRateAtTimeout () const
{
  return this->rate_at_timeout;
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetExclusionDiscardedTimeout (double exclusionDiscardedTimeout)
{
  NS_ASSERT_MSG (exclusionDiscardedTimeout >= -1, "Exclusion discarded timeout cannot be negative. If you want to disable the discard factor you should set this attribute to -1");

  this->exclusion_discarded_timeout = exclusionDiscardedTimeout;
}

template<class Policy>
double
ContentStoreImpl<Policy>::GetExclusionDiscardedTimeout () const
{
  return this->exclusion_discarded_timeout;
}


template<class Policy>
void
ContentStoreImpl<Policy>::SetSearchCacheAfter (int searchCacheAfter)
{
  NS_ASSERT_MSG (search_cache_after >= 0, "SearchCacheAfter attribute cannot be negative");

  this->search_cache_after = searchCacheAfter;
}

template<class Policy>
int
ContentStoreImpl<Policy>::GetSearchCacheAfter () const
{
  return this->search_cache_after;
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetBadContentName (std::string badContentName)
{
  bad_content_name = badContentName;
}

template<class Policy>
std::string
ContentStoreImpl<Policy>::GetBadContentName () const
{
  return bad_content_name;
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetBadContentFreshness (Time badContentFreshness)
{
  bad_content_freshness = badContentFreshness;
}

template<class Policy>
Time
ContentStoreImpl<Policy>::GetBadContentFreshness () const
{
  return bad_content_freshness;
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetBadContentCount (uint32_t badContentCount)
{
  bad_content_count = badContentCount;
}

template<class Policy>
uint32_t
ContentStoreImpl<Policy>::GetBadContentCount () const
{
  return bad_content_count;
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetBadContentPayloadSize (uint32_t badContentPayloadSize)
{
  bad_content_payload_size = badContentPayloadSize;
}

template<class Policy>
uint32_t
ContentStoreImpl<Policy>::GetBadContentPayloadSize () const
{
  return bad_content_payload_size;
}

template<class Policy>
void
ContentStoreImpl<Policy>::SetBadContentRate (double badContentRate)
{
  bad_content_rate = badContentRate;
}

template<class Policy>
double
ContentStoreImpl<Policy>::GetBadContentRate () const
{
  return bad_content_rate;
}

template<class Policy>
uint32_t
ContentStoreImpl<Policy>::GetSize () const
{
  return this->getPolicy ().size ();
}

template<class Policy>
Ptr<Entry>
ContentStoreImpl<Policy>::Begin ()
{
  typename super::parent_trie::recursive_iterator item (super::getTrie ()), end (0);
  for (; item != end; item++)
    {
      if (item->payload () == 0) continue;
      break;
    }

  if (item == end)
    return End ();
  else
    return item->payload ();
}

template<class Policy>
Ptr<Entry>
ContentStoreImpl<Policy>::End ()
{
  return 0;
}

template<class Policy>
Ptr<Entry>
ContentStoreImpl<Policy>::Next (Ptr<Entry> from)
{
  if (from == 0) return 0;

  typename super::parent_trie::recursive_iterator
    item (*StaticCast< entry > (from)->to_iterator ()),
    end (0);

  for (item++; item != end; item++)
    {
      if (item->payload () == 0) continue;
      break;
    }

  if (item == end)
    return End ();
  else
    return item->payload ();
}


} // namespace cs
} // namespace ndn
} // namespace ns3

#endif // NDN_CONTENT_STORE_IMPL_H_
