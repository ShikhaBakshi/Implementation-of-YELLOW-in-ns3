/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 NITK Surathkal
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
 * Authors: Shikha Bakshi<shikhabakshi912@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/simulator.h"
#include "ns3/abort.h"
#include "yellow-queue-disc.h"
#include "ns3/drop-tail-queue.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("YellowQueueDisc");

NS_OBJECT_ENSURE_REGISTERED (YellowQueueDisc);

TypeId YellowQueueDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::YellowQueueDisc")
    .SetParent<QueueDisc> ()
    .SetGroupName ("TrafficControl")
    .AddConstructor<YellowQueueDisc> ()
    .AddAttribute ("Mode",
                   "Determines unit for QueueLimit",
                   EnumValue (Queue::QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&YellowQueueDisc::SetMode),
                   MakeEnumChecker (Queue::QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    Queue::QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("QueueLimit",
                   "Queue limit in bytes/packets",
                   UintegerValue (20),
                   MakeUintegerAccessor (&YellowQueueDisc::SetQueueLimit),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("PMark",
                   "Marking Probabilty",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&YellowQueueDisc::m_Pmark),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MeanPktSize",
                   "Average of packet size",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&YellowQueueDisc::m_meanPktSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Alpha",
                   "Factor affecting queue drain rate",
                   DoubleValue (1.02),
                   MakeDoubleAccessor (&YellowQueueDisc::m_alpha),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Beta",
                   "Beta Value",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&YellowQueueDisc::m_beta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Delta",
                   "Delta Value",
                   DoubleValue (0.06),
                   MakeDoubleAccessor (&YellowQueueDisc::m_delta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Gamma",
                   "Link Utilization Factor",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&YellowQueueDisc::m_gamma),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("LinkCapacity",
                   "Queue Link Capacity",
                   DataRateValue (DataRate("10Mbps")),
                   MakeDataRateAccessor (&YellowQueueDisc::m_Linkcapacity),
                   MakeDataRateChecker ())
    .AddAttribute ("Udelta",
                   "Udelta decrement Value",
                   DoubleValue (11.25),
                   MakeDoubleAccessor (&YellowQueueDisc::m_udelta),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("FreezeTime",
                   "Time interval during which Pmark cannot be updated",
                   TimeValue (Seconds (0.5)),
                   MakeTimeAccessor (&YellowQueueDisc::m_freezeTime),
                   MakeTimeChecker ())
  ;

  return tid;
}

YellowQueueDisc::YellowQueueDisc () : QueueDisc ()
{
  NS_LOG_FUNCTION (this);
  m_uv = CreateObject<UniformRandomVariable> ();
}

YellowQueueDisc::~YellowQueueDisc ()
{
  NS_LOG_FUNCTION (this);
}

void
YellowQueueDisc::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_uv = 0;
  QueueDisc::DoDispose ();
}

void
YellowQueueDisc::SetMode (Queue::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);
  m_mode = mode;
}

Queue::QueueMode
YellowQueueDisc::GetMode (void)
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
YellowQueueDisc::SetQueueLimit (uint32_t lim)
{
  NS_LOG_FUNCTION (this << lim);
  m_queueLimit = lim;
}

uint32_t
YellowQueueDisc::GetQueueSize (void)
{
  NS_LOG_FUNCTION (this);
  if (GetMode () == Queue::QUEUE_MODE_BYTES)
    {
      return GetInternalQueue (0)->GetNBytes ();
    }
  else if (GetMode () == Queue::QUEUE_MODE_PACKETS)
    {
      return GetInternalQueue (0)->GetNPackets ();
    }
  else
    {
      NS_ABORT_MSG ("Unknown Yellow mode.");
    }
}

YellowQueueDisc::Stats
YellowQueueDisc::GetStats ()
{
  NS_LOG_FUNCTION (this);
  return m_stats;
}

int64_t
YellowQueueDisc::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uv->SetStream (stream);
  return 1;
}

bool
YellowQueueDisc::DoEnqueue (Ptr<QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << item);

  // uint32_t nQueued = GetQueueSize ();


  if (m_isIdle)
    {
      DecrementPmark ();
      m_isIdle = false; // not idle anymore
    }
  m_in_pkt++;
  Time now = Simulator::Now ();
  // std::cout<<"rate "<<m_rate;  
  if (now - m_lastUpdateTime >= m_freezeTime)
    {
      m_in_rate = m_in_pkt*(8.0*m_meanPktSize)/(now - m_lastUpdateTime).GetSeconds();
      //std::cout<<"Input Rate"<<m_in_rate;
     // m_lastUpdateTime = now;
      m_in_pkt=0;
    }
      
  //calculate load factor
  CalculateLoadFactor (item);
  if (m_loadfactor >= (1 + m_delta))
    {
      // Increment the Pmark
      IncrementPmark ();
    }
   else if(m_loadfactor<1)
    {
        // Decrement the Pmark
        DecrementPmark ();
    }    


      if (DropEarly ())
    {
      // Early probability drop: proactive
      m_stats.unforcedDrop++;
      Drop (item);
      return false;
    }


  // No drop
  bool isEnqueued = GetInternalQueue (0)->Enqueue (item);
//  std::cout<<"isEnqueued "<<isEnqueued;


  NS_LOG_LOGIC ("\t bytesInQueue  " << GetInternalQueue (0)->GetNBytes ());
  NS_LOG_LOGIC ("\t packetsInQueue  " << GetInternalQueue (0)->GetNPackets ());

  return isEnqueued;
}
void YellowQueueDisc::InitializeParams (void)
{
  m_lastUpdateTime = Time (Seconds (0.0));
  m_idleStartTime = Time (Seconds (0.0));
  m_stats.forcedDrop = 0;
  m_stats.unforcedDrop = 0;
  m_isIdle = true;
  m_pkt=0;
  m_in_pkt=0;
  m_loadfactor=1;
  m_rate=m_Linkcapacity.GetBitRate ();
  m_in_rate=m_Linkcapacity.GetBitRate ();
//  std::cout<<"m_rate "<<m_rate;
}

bool YellowQueueDisc::DropEarly (void)
{
  NS_LOG_FUNCTION (this);
  double u =  m_uv->GetValue ();
  //std::cout<<u<<" "<<m_Pmark<<"\n";
  if (u <= m_Pmark)
    {
      return true;
    }
  return false;
}
void YellowQueueDisc::CalculateLoadFactor (Ptr<QueueDiscItem> item)
{
  uint32_t nQueued = GetQueueSize ();
  double qcf;
  if ((GetMode () == Queue::QUEUE_MODE_PACKETS && nQueued >= m_queueLimit)
      || (GetMode () == Queue::QUEUE_MODE_BYTES && nQueued + item->GetPacketSize () > m_queueLimit))
    {
      qcf = (m_gamma * m_alpha * m_queueLimit) / ((m_alpha - 1) * nQueued + m_queueLimit);
    }
  else
    {
      qcf = (m_gamma * m_beta * m_queueLimit) / ((m_beta - 1) * nQueued + m_queueLimit);
    }

  m_capacity = qcf * m_rate;
  m_loadfactor = m_in_rate / m_capacity;
// std::cout<<"lf"<<m_loadfactor;

}

void YellowQueueDisc::IncrementPmark (void)
{
  NS_LOG_FUNCTION (this);
  m_increment = m_udelta * m_loadfactor / m_Linkcapacity.GetBitRate ();
 // std::cout<<m_increment<<"incre"<<m_Linkcapacity.GetBitRate ()<<"\n";
  Time now = Simulator::Now ();
  if (now - m_lastUpdateTime > m_freezeTime)
    {
      m_Pmark += m_increment;
      m_lastUpdateTime = now;
      if (m_Pmark > 1.0)
        {
          m_Pmark = 1.0;
        }
    }
}

void YellowQueueDisc::DecrementPmark (void)
{
  NS_LOG_FUNCTION (this);
  Time now = Simulator::Now ();
  if (m_isIdle)
    {
      uint32_t m = 0; // stores the number of times Pmark should be decremented
      m = ((now - m_idleStartTime) / m_freezeTime);
      m_decrement = m_udelta / (m_loadfactor * m_Linkcapacity.GetBitRate ());
      m_Pmark -= (m_decrement * m);
      m_lastUpdateTime = now;
      if (m_Pmark < 0.0)
        {
          m_Pmark = 0.0;
        }
    }
  else if (now - m_lastUpdateTime > m_freezeTime)
    {
      m_Pmark -= m_decrement;
      m_lastUpdateTime = now;
      if (m_Pmark < 0.0)
        {
          m_Pmark = 0.0;
        }
    }
}

Ptr<QueueDiscItem>
YellowQueueDisc::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<QueueDiscItem> item = StaticCast<QueueDiscItem> (GetInternalQueue (0)->Dequeue ());

  NS_LOG_LOGIC ("Popped " << item);

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());
  
  m_pkt++;
  Time now = Simulator::Now ();
     // std::cout<<"rate "<<m_rate;  
  if (now - m_lastUpdateTime >= m_freezeTime)
    {
      m_rate = m_pkt*(8.0*m_meanPktSize)/(now - m_lastUpdateTime).GetSeconds();
      //std::cout<<"Rate Calculated"<<m_rate;
     // m_lastUpdateTime = now;
      m_pkt=0;
    }
      

  /*if (m_loadfactor < 1 && !m_isIdle)
    {
      NS_LOG_LOGIC ("Queue empty");

      DecrementPmark ();

      m_idleStartTime = Simulator::Now ();
      m_isIdle = true;
    }*/

  return item;
}

Ptr<const QueueDiscItem>
YellowQueueDisc::DoPeek () const
{
  NS_LOG_FUNCTION (this);
  if (GetInternalQueue (0)->IsEmpty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<const QueueDiscItem> item = StaticCast<const QueueDiscItem> (GetInternalQueue (0)->Peek ());

  NS_LOG_LOGIC ("Number packets " << GetInternalQueue (0)->GetNPackets ());
  NS_LOG_LOGIC ("Number bytes " << GetInternalQueue (0)->GetNBytes ());

  return item;
}

bool
YellowQueueDisc::CheckConfig (void)
{
  NS_LOG_FUNCTION (this);
  if (GetNQueueDiscClasses () > 0)
    {
      NS_LOG_ERROR ("YellowQueueDisc cannot have classes");
      return false;
    }

  if (GetNPacketFilters () > 0)
    {
      NS_LOG_ERROR ("YellowQueueDisc cannot have packet filters");
      return false;
    }

  if (GetNInternalQueues () == 0)
    {
      // create a DropTail queue
      Ptr<Queue> queue = CreateObjectWithAttributes<DropTailQueue> ("Mode", EnumValue (m_mode));
      if (m_mode == Queue::QUEUE_MODE_PACKETS)
        {
          queue->SetMaxPackets (m_queueLimit);
        }
      else
        {
          queue->SetMaxBytes (m_queueLimit);
        }
      AddInternalQueue (queue);
    }

  if (GetNInternalQueues () != 1)
    {
      NS_LOG_ERROR ("YellowQueueDisc needs 1 internal queue");
      return false;
    }

  if (GetInternalQueue (0)->GetMode () != m_mode)
    {
      NS_LOG_ERROR ("The mode of the provided queue does not match the mode set on the YellowQueueDisc");
      return false;
    }

  if ((m_mode ==  Queue::QUEUE_MODE_PACKETS && GetInternalQueue (0)->GetMaxPackets () < m_queueLimit)
      || (m_mode ==  Queue::QUEUE_MODE_BYTES && GetInternalQueue (0)->GetMaxBytes () < m_queueLimit))
    {
      NS_LOG_ERROR ("The size of the internal queue is less than the queue disc limit");
      return false;
    }

  return true;
}

} //namespace ns3

