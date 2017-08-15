/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ideal-wifi-manager.h"
#include "wifi-phy.h"
#include "ns3/assert.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include <cmath>

#define Min(a,b) ((a < b) ? a : b)

namespace ns3 {

/**
 * \brief hold per-remote-station state for Ideal Wifi manager.
 *
 * This struct extends from WifiRemoteStation struct to hold additional
 * information required by the Ideal Wifi manager
 */
struct IdealWifiRemoteStation : public WifiRemoteStation
{
  double m_lastSnr;  //!< SNR of last packet sent to the remote station
};

NS_OBJECT_ENSURE_REGISTERED (IdealWifiManager);

NS_LOG_COMPONENT_DEFINE ("IdealWifiManager");

TypeId
IdealWifiManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::IdealWifiManager")
    .SetParent<WifiRemoteStationManager> ()
    .SetGroupName ("Wifi")
    .AddConstructor<IdealWifiManager> ()
    .AddAttribute ("BerThreshold",
                   "The maximum Bit Error Rate acceptable at any transmission mode",
                   DoubleValue (10e-6),
                   MakeDoubleAccessor (&IdealWifiManager::m_ber),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

IdealWifiManager::IdealWifiManager ()
{
}

IdealWifiManager::~IdealWifiManager ()
{
}

void
IdealWifiManager::SetupPhy (Ptr<WifiPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  uint32_t nModes = phy->GetNModes ();
  WifiTxVector txVector;
  for (uint32_t i = 0; i < nModes; i++)
    {
      WifiMode mode = phy->GetMode (i);
      NS_LOG_DEBUG ("In SetupPhy, adding mode = " << mode.GetUniqueName ());
      // XXX need to convert this properly to use WifiTxVector
      txVector.SetNss (1);
      txVector.SetMode (mode);
      AddModeSnrThreshold (mode, phy->CalculateSnr (txVector, m_ber));
    }
  if (HasHtSupported () == true)
    {
      nModes = phy->GetNMcs ();
      for (uint32_t i = 0; i < nModes; i++)
        {
          WifiMode mode = phy->GetMcs (i);
          if (mode == WifiPhy::GetHtMcs8 () || mode == WifiPhy::GetHtMcs9 () ||
              mode == WifiPhy::GetHtMcs10 () || mode == WifiPhy::GetHtMcs11 () ||
              mode == WifiPhy::GetHtMcs12 () || mode == WifiPhy::GetHtMcs13 () ||
              mode == WifiPhy::GetHtMcs14 () || mode == WifiPhy::GetHtMcs15 ())
            {
              NS_LOG_DEBUG ("Setting NSS to 2");
              txVector.SetNss (2);
            }
          else 
            {
              NS_LOG_DEBUG ("Setting NSS to 1");
              txVector.SetNss (1);
            }
          txVector.SetMode (mode);
          NS_LOG_DEBUG ("In SetupPhy, adding HT mode = " << mode.GetUniqueName ());
          AddModeSnrThreshold (mode, phy->CalculateSnr (txVector, m_ber));
        }
    }
  WifiRemoteStationManager::SetupPhy (phy);
}

double
IdealWifiManager::GetSnrThreshold (WifiMode mode) const
{
  NS_LOG_FUNCTION (this << mode.GetUniqueName ());
  for (Thresholds::const_iterator i = m_thresholds.begin (); i != m_thresholds.end (); i++)
    {
      if (mode == i->second)
        {
          return i->first;
        }
    }
  NS_ASSERT (false);
  return 0.0;
}

void
IdealWifiManager::AddModeSnrThreshold (WifiMode mode, double snr)
{
  NS_LOG_FUNCTION (this << mode.GetUniqueName () << snr);
  m_thresholds.push_back (std::make_pair (snr,mode));
}

WifiRemoteStation *
IdealWifiManager::DoCreateStation (void) const
{
  NS_LOG_FUNCTION (this);
  IdealWifiRemoteStation *station = new IdealWifiRemoteStation ();
  station->m_lastSnr = 0.0;
  return station;
}


void
IdealWifiManager::DoReportRxOk (WifiRemoteStation *station,
                                double rxSnr, WifiMode txMode)
{
}

void
IdealWifiManager::DoReportRtsFailed (WifiRemoteStation *station)
{
}

void
IdealWifiManager::DoReportDataFailed (WifiRemoteStation *station)
{
}

void
IdealWifiManager::DoReportRtsOk (WifiRemoteStation *st,
                                 double ctsSnr, WifiMode ctsMode, double rtsSnr)
{
  NS_LOG_FUNCTION (this << st << ctsSnr << ctsMode.GetUniqueName () << rtsSnr);
  IdealWifiRemoteStation *station = (IdealWifiRemoteStation *)st;
  station->m_lastSnr = rtsSnr;
}

void
IdealWifiManager::DoReportDataOk (WifiRemoteStation *st,
                                  double ackSnr, WifiMode ackMode, double dataSnr)
{
  NS_LOG_FUNCTION (this << st << ackSnr << ackMode.GetUniqueName () << dataSnr);
  IdealWifiRemoteStation *station = (IdealWifiRemoteStation *)st;
  if (dataSnr == 0)
    {
      NS_LOG_WARN ("DataSnr reported to be zero; not saving this report.");
      return;
    }
  station->m_lastSnr = dataSnr;
}

void
IdealWifiManager::DoReportFinalRtsFailed (WifiRemoteStation *station)
{
}

void
IdealWifiManager::DoReportFinalDataFailed (WifiRemoteStation *station)
{
}

WifiTxVector
IdealWifiManager::DoGetDataTxVector (WifiRemoteStation *st, uint32_t size)
{
  NS_LOG_FUNCTION (this << st << size);
  IdealWifiRemoteStation *station = (IdealWifiRemoteStation *)st;
  //We search within the Supported rate set the mode with the
  //highest snr threshold possible which is smaller than m_lastSnr
  //to ensure correct packet delivery.
  double maxThreshold = 0.0;
  WifiMode maxMode = GetDefaultMode ();
  // XXX This model assumes that the receiver has 2 antennas, so the
  // number of spatial streams is assumed to be the number of transmit
  // antennas.  If only one spatial stream, then increase SNR by 3 dB
  // to account for diversity.
  uint32_t numSpatialStreams = GetPhy ()->GetNumberOfTransmitAntennas ();
  NS_ASSERT (numSpatialStreams == 1 || numSpatialStreams == 2);
  // XXX this should be converted to iterate the WifiPhy::WifiModeList
  // If the node is HT capable, search HT modes first
  if (HasHtSupported () == true)
    {
      for (uint32_t i = 0; i < GetNMcsSupported (station); i++)
        {
          WifiMode mode = GetMcsSupported (station, i);
          uint32_t nss = WifiPhy::GetNssForHtMcs (mode);
          NS_ASSERT (nss > 0);
          if (numSpatialStreams != nss)
            {
              NS_LOG_DEBUG ("Skipping mode = " << mode.GetUniqueName () << " due to spatial stream comparison");
              continue;
            }
          double threshold = GetSnrThreshold (mode);
          if (numSpatialStreams == 1)
            {
              NS_LOG_DEBUG ("Applying diversity gain for 1x2 uplink");
              threshold *= 2;  
            }
          NS_LOG_DEBUG ("Testing mode = " << mode.GetUniqueName () << " threshold " << threshold  << " maxThreshold " << maxThreshold << " last snr " << station->m_lastSnr);
          if (threshold > maxThreshold
              && threshold < station->m_lastSnr)
            {
              NS_LOG_DEBUG ("Candidate mode = " << mode.GetUniqueName () << " threshold " << threshold  << " last snr " << station->m_lastSnr);
              maxThreshold = threshold;
              maxMode = mode;
            }
        }
    }
  else
    {
      // Non-HT selection
      for (uint32_t i = 0; i < GetNSupported (station); i++)
        {
          WifiMode mode = GetSupported (station, i);
          double threshold = GetSnrThreshold (mode);
          NS_LOG_DEBUG ("mode = " << mode.GetUniqueName () << " threshold " << threshold  << " last snr " << station->m_lastSnr);
          if (threshold > maxThreshold
              && threshold < station->m_lastSnr)
            {
              maxThreshold = threshold;
              maxMode = mode;
            }
        }
    }
  uint32_t channelWidth = GetChannelWidth (station);
  if (channelWidth > 20 && channelWidth != 22)
    {
      //avoid to use legacy rate adaptation algorithms for IEEE 802.11n/ac
      channelWidth = 20;
    }
  NS_LOG_DEBUG ("Selecting maxMode " << maxMode.GetUniqueName () << " ss " << numSpatialStreams);
  return WifiTxVector (maxMode, GetDefaultTxPowerLevel (), GetLongRetryCount (station), false, numSpatialStreams, 0, channelWidth, GetAggregation (station), false);
}

WifiTxVector
IdealWifiManager::DoGetRtsTxVector (WifiRemoteStation *st)
{
  NS_LOG_FUNCTION (this << st);
  IdealWifiRemoteStation *station = (IdealWifiRemoteStation *)st;
  //We search within the Basic rate set the mode with the highest
  //snr threshold possible which is smaller than m_lastSnr to
  //ensure correct packet delivery.
  double maxThreshold = 0.0;
  WifiMode maxMode = GetDefaultMode ();
  for (uint32_t i = 0; i < GetNBasicModes (); i++)
    {
      WifiMode mode = GetBasicMode (i);
      double threshold = GetSnrThreshold (mode);
      if (threshold > maxThreshold
          && threshold < station->m_lastSnr)
        {
          maxThreshold = threshold;
          maxMode = mode;
        }
    }
  uint32_t channelWidth = GetChannelWidth (station);
  if (channelWidth > 20 && channelWidth != 22)
    {
      //avoid to use legacy rate adaptation algorithms for IEEE 802.11n/ac
      channelWidth = 20;
    }
  // XXX below is hardcoded to use 1 spatial stream; need to check how RTS
  // is sent in practice
  return WifiTxVector (maxMode, GetDefaultTxPowerLevel (), GetShortRetryCount (station), false, 1, 0, channelWidth, GetAggregation (station), false);
}

bool
IdealWifiManager::IsLowLatency (void) const
{
  return true;
}

} //namespace ns3
