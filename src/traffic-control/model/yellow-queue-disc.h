/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 NITK Surathkal
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
 * Authors: Vivek Jain <jain.vivek.anand@gmail.com>
 *          Sandeep Singh <hisandeepsingh@hotmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#ifndef YELLOW_QUEUE_DISC_H
#define YELLOW_QUEUE_DISC_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue-disc.h"
#include "ns3/nstime.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/timer.h"
#include "ns3/event-id.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {

class UniformRandomVariable;

class YellowQueueDisc : public QueueDisc
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief YellowQueueDisc Constructor
   */
  YellowQueueDisc ();

  /**
   * \brief YellowQueueDisc Destructor
   */
  virtual ~YellowQueueDisc ();

  /**
   * \brief Stats
   */
  typedef struct
  {
    uint32_t unforcedDrop;      //!< Early probability drops: proactive
    uint32_t forcedDrop;        //!< Drops due to queue limit: reactive
  } Stats;

  /**
   * \brief Set the operating mode of this queue.
   *
   * \param mode The operating mode of this queue.
   */
  void SetMode (Queue::QueueMode mode);

  /**
   * \brief Get the encapsulation mode of this queue.
   *
   * \returns The encapsulation mode of this queue.
   */
  Queue::QueueMode GetMode (void);

  /**
   * \brief Get the current value of the queue in bytes or packets.
   *
   * \returns The queue size in bytes or packets.
   */
  uint32_t GetQueueSize (void);

  /**
   * \brief Set the limit of the queue in bytes or packets.
   *
   * \param lim The limit in bytes or packets.
   */
  void SetQueueLimit (uint32_t lim);

  /**
   * \brief Get queue delay
   */
  Time GetQueueDelay (void);

  /**
   * \brief Get YELLOW statistics after running.
   *
   * \returns The drop statistics.
   */
  Stats GetStats ();

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

protected:
  /**
   * \brief Dispose of the object
   */
  virtual void DoDispose (void);

  /**
   * \brief Initialize the queue parameters.
   */
  virtual void InitializeParams (void);

  virtual bool DoEnqueue (Ptr<QueueDiscItem> item);
  virtual Ptr<QueueDiscItem> DoDequeue (void);
  virtual Ptr<const QueueDiscItem> DoPeek (void) const;
  virtual bool CheckConfig (void);

  /**
   * \brief Increment the value of marking probability
   */
  virtual void IncrementPmark (void);

  /**
   * \brief Decrement the value of marking probability
   */
  virtual void DecrementPmark (void);

  /**
   * \brief Calculate the value of Load Factor
   */
  virtual void CalculateLoadFactor (Ptr<QueueDiscItem> item);

  /**
   * \brief Check if a packet needs to be dropped due to probability drop
   * \returns false for no drop, true for drop
   */
  virtual bool DropEarly (void);

private:
  Queue::QueueMode m_mode;                      //!< Mode (bytes or packets)
  uint32_t m_queueLimit;                        //!< Queue limit in bytes / packets
  Stats m_stats;                                //!< YELLOW statistics
  Ptr<UniformRandomVariable> m_uv;              //!< Rng stream

  // ** Variables supplied by user
  double m_Pmark;                               //!< Marking Probability
  uint32_t m_meanPktSize;                       //!< Average Packet Size
  double m_alpha;                               //!< Alpha vale
  double m_beta;                                //!< Beta value
  double m_gamma;                               //!< Gamma value
  double m_delta;
  double m_udelta;
  double m_loadfactor;                          //!< Load factor
  double m_rate;                                //!< Input Link Rate
  double m_Linkcapacity;                        //!< Input Link Capacity
  Time m_freezeTime;                            //!< Time interval during which Pmark cannot be updated

  // ** Variables maintained by YELLOW
  double m_capacity;                            //!< Virtual link Capacity
  double m_increment;
  double m_decrement;
  uint32_t m_pkt;                               //!< Counter to track enqueued packets to calculate Link Input Rate
  Time m_lastUpdateTime;                        //!< last time at which Pmark was updated
  Time m_idleStartTime;                         //!< Time when YELLOW Queue Disc entered the idle period
  bool m_isIdle;                                //!< True if queue is Idle
};

} // namespace ns3

#endif // YELLOW_QUEUE_DISC_H
