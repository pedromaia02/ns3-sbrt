/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
 * Copyright (c) 2015 University of Washington
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
 * Authors: Mirko Banchi <mk.banchi@gmail.com>
 *          Sebastien Deronne <sebastien.deronne@gmail.com>
 *
 * Modified from ht-wifi-network.cc
 */
#include <sstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/internet-module.h"

// This is a simple example of an IEEE 802.11n Wi-Fi network with a
// non-Wi-Fi interferer.  It is an adaptation of wifi-spectrum-per-example.
//
// Unless the --waveformPower argument is passed, it will behave like
// wifi-spectrum-per-example.  Adding --waveformPower=value for values
// greater than 0.0001 will result in frame losses
//
// Network topology:
//
//  Wifi 192.168.1.0
//
//         * - n3 interferer
//
//         AP
//    *    *
//    |    |
//    n1   n2
//

using namespace ns3;

// Global variables for use in callbacks.
double g_signalDbmAvg;
double g_noiseDbmAvg;
uint32_t g_samples;
uint16_t g_channelNumber;
uint32_t g_rate;

void MonitorSniffRx (Ptr<const Packet> packet, uint16_t channelFreqMhz,
                     uint16_t channelNumber, uint32_t rate,
                     bool isShortPreamble, double signalDbm,
                     double noiseDbm)
{
  g_samples++;
  g_signalDbmAvg += ((signalDbm - g_signalDbmAvg)/g_samples);
  g_noiseDbmAvg += ((noiseDbm - g_noiseDbmAvg)/g_samples);
  g_rate = rate;
  g_channelNumber = channelNumber;
}

NS_LOG_COMPONENT_DEFINE ("WifiSpectrumPerExample");

Ptr<SpectrumModel> SpectrumModelWifi5180MHz;

class static_SpectrumModelWifi5180MHz_initializer
{
public:
  static_SpectrumModelWifi5180MHz_initializer ()
  {
    BandInfo bandInfo;
    bandInfo.fc = 5180e6;
    bandInfo.fl = 5180e6 - 10e6;
    bandInfo.fh = 5180e6 + 10e6;

    Bands bands;
    bands.push_back (bandInfo);

    SpectrumModelWifi5180MHz = Create<SpectrumModel> (bands);
  }

} static_SpectrumModelWifi5180MHz_initializer_instance;


int main (int argc, char *argv[])
{
  bool udp = true;
  double distance = 50;
  double simulationTime = 10; //seconds
  uint16_t mcs = 256;
  std::string wifiType = "ns3::SpectrumWifiPhy";
  bool enablePcap = false;
  double waveformPower = 0;

  CommandLine cmd;
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("udp", "UDP if set to 1, TCP otherwise", udp);
  cmd.AddValue ("distance", "meters separation between nodes", distance);
  cmd.AddValue ("mcs", "restrict MCS to single value between 0 and 31", mcs);
  cmd.AddValue ("wifiType", "select Spectrum or Yans WifiPhy", wifiType);
  cmd.AddValue ("enablePcap", "enable pcap output", enablePcap);
  cmd.AddValue ("waveformPower", "Waveform power", waveformPower);
  cmd.Parse (argc,argv);

  uint16_t startIndex = 0;
  uint16_t stopIndex = 31;
  if (mcs < 32)
    {
      startIndex = mcs;
      stopIndex = mcs;
    }

  std::cout << "wifiType: " << wifiType << " distance: " << distance << "m; sent: 1000" << std::endl;
  std::cout << "MCS " << "\t" << "DataRate" << "\t" << "Throughput" << "\t" << "Received " << " " << "Signal (dBm) Noi/Inf(dBm) SNR(dB)\n";
  for (uint16_t i = startIndex; i <= stopIndex; i++)
    {
      uint32_t payloadSize; 
      if (udp)
        {
          payloadSize = 972; // 1000 bytes IPv4
        }
      else
        {
          payloadSize = 1448; // 1500 bytes IPv6
          Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
        }

      NodeContainer wifiStaNode;
      wifiStaNode.Create (1);
      NodeContainer wifiApNode;
      wifiApNode.Create (1);
      NodeContainer interferingNode;
      interferingNode.Create (1);

      YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
      SpectrumWifiPhyHelper spectrumPhy = SpectrumWifiPhyHelper::Default ();
      Ptr<MultiModelSpectrumChannel> spectrumChannel;
      if (wifiType == "ns3::YansWifiPhy")
        {
          YansWifiChannelHelper channel;
          channel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
          channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
          phy.SetChannel (channel.Create ());
    
          if (i <= 7)
            {
              phy.Set ("ShortGuardEnabled", BooleanValue (false));
              phy.Set ("ChannelWidth", UintegerValue (20));
            }
          else if (i > 7 && i <= 15)
            {
              phy.Set ("ShortGuardEnabled", BooleanValue (true));
              phy.Set ("ChannelWidth", UintegerValue (20));
            }
          else if (i > 15 && i <= 23)
            {
              phy.Set ("ShortGuardEnabled", BooleanValue (false));
              phy.Set ("ChannelWidth", UintegerValue (40));
            }
          else
            {
              phy.Set ("ShortGuardEnabled", BooleanValue (true));
              phy.Set ("ChannelWidth", UintegerValue (40));
            }
        }
      else if (wifiType == "ns3::SpectrumWifiPhy")
        {
          spectrumChannel
            = CreateObject<MultiModelSpectrumChannel> ();
          Ptr<FriisPropagationLossModel> lossModel
            = CreateObject<FriisPropagationLossModel> ();
          spectrumChannel->AddPropagationLossModel (lossModel);

          Ptr<ConstantSpeedPropagationDelayModel> delayModel
            = CreateObject<ConstantSpeedPropagationDelayModel> ();
          spectrumChannel->SetPropagationDelayModel (delayModel);

          spectrumPhy.SetChannel (spectrumChannel);
          spectrumPhy.SetChannelNumber (36); // 5.18 :
    
          if (i <= 7)
            {
              spectrumPhy.Set ("ShortGuardEnabled", BooleanValue (false));
              spectrumPhy.Set ("ChannelWidth", UintegerValue (20));
            }
          else if (i > 7 && i <= 15)
            {
              spectrumPhy.Set ("ShortGuardEnabled", BooleanValue (true));
              spectrumPhy.Set ("ChannelWidth", UintegerValue (20));
            }
          else if (i > 15 && i <= 23)
            {
              spectrumPhy.Set ("ShortGuardEnabled", BooleanValue (false));
              spectrumPhy.Set ("ChannelWidth", UintegerValue (40));
            }
          else
            {
              spectrumPhy.Set ("ShortGuardEnabled", BooleanValue (true));
              spectrumPhy.Set ("ChannelWidth", UintegerValue (40));
            }
        } 
      else
        {
          NS_FATAL_ERROR ("Unsupported WiFi type " << wifiType);
        } 


      WifiHelper wifi = WifiHelper::Default ();
      wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
      HtWifiMacHelper mac = HtWifiMacHelper::Default ();

      Ssid ssid = Ssid ("ns380211n");

      double datarate = 0;
      StringValue DataRate;
      if (i == 0)
        {
          DataRate = StringValue ("OfdmRate6_5MbpsBW20MHz");
          datarate = 6.5;
        }
      else if (i == 1)
        {
          DataRate = StringValue ("OfdmRate13MbpsBW20MHz");
          datarate = 13;
        }
      else if (i == 2)
        {
          DataRate = StringValue ("OfdmRate19_5MbpsBW20MHz");
          datarate = 19.5;
        }
      else if (i == 3)
        {
          DataRate = StringValue ("OfdmRate26MbpsBW20MHz");
          datarate = 26;
        }
      else if (i == 4)
        {
          DataRate = StringValue ("OfdmRate39MbpsBW20MHz");
          datarate = 39;
        }
      else if (i == 5)
        {
          DataRate = StringValue ("OfdmRate52MbpsBW20MHz");
          datarate = 52;
        }
      else if (i == 6)
        {
          DataRate = StringValue ("OfdmRate58_5MbpsBW20MHz");
          datarate = 58.5;
        }
      else if (i == 7)
        {
          DataRate = StringValue ("OfdmRate65MbpsBW20MHz");
          datarate = 65;
        }
      else if (i == 8)
        {
          DataRate = StringValue ("OfdmRate7_2MbpsBW20MHz");
          datarate = 7.2;
        }
      else if (i == 9)
        {
          DataRate = StringValue ("OfdmRate14_4MbpsBW20MHz");
          datarate = 14.4;
        }
      else if (i == 10)
        {
          DataRate = StringValue ("OfdmRate21_7MbpsBW20MHz");
          datarate = 21.7;
        }
      else if (i == 11)
        {
          DataRate = StringValue ("OfdmRate28_9MbpsBW20MHz");
          datarate = 28.9;
        }
      else if (i == 12)
        {
          DataRate = StringValue ("OfdmRate43_3MbpsBW20MHz");
          datarate = 43.3;
        }
      else if (i == 13)
        {
          DataRate = StringValue ("OfdmRate57_8MbpsBW20MHz");
          datarate = 57.8;
        }
      else if (i == 14)
        {
          DataRate = StringValue ("OfdmRate65MbpsBW20MHzShGi");
          datarate = 65;
        }
      else if (i == 15)
        {
          DataRate = StringValue ("OfdmRate72_2MbpsBW20MHz");
          datarate = 72.2;
        }
      else if (i == 16)
        {
          DataRate = StringValue ("OfdmRate13_5MbpsBW40MHz");
          datarate = 13.5;
        }
      else if (i == 17)
        {
          DataRate = StringValue ("OfdmRate27MbpsBW40MHz");
          datarate = 27;
        }
      else if (i == 18)
        {
          DataRate = StringValue ("OfdmRate40_5MbpsBW40MHz");
          datarate = 40.5;
        }
      else if (i == 19)
        {
          DataRate = StringValue ("OfdmRate54MbpsBW40MHz");
          datarate = 54;
        }
      else if (i == 20)
        {
          DataRate = StringValue ("OfdmRate81MbpsBW40MHz");
          datarate = 81;
        }
      else if (i == 21)
        {
          DataRate = StringValue ("OfdmRate108MbpsBW40MHz");
          datarate = 108;
        }
      else if (i == 22)
        {
          DataRate = StringValue ("OfdmRate121_5MbpsBW40MHz");
          datarate = 121.5;
        }
      else if (i == 23)
        {
          DataRate = StringValue ("OfdmRate135MbpsBW40MHz");
          datarate = 135;
        }
      else if (i == 24)
        {
          DataRate = StringValue ("OfdmRate15MbpsBW40MHz");
          datarate = 15;
        }
      else if (i == 25)
        {
          DataRate = StringValue ("OfdmRate30MbpsBW40MHz");
          datarate = 30;
        }
      else if (i == 26)
        {
          DataRate = StringValue ("OfdmRate45MbpsBW40MHz");
          datarate = 45;
        }
      else if (i == 27)
        {
          DataRate = StringValue ("OfdmRate60MbpsBW40MHz");
          datarate = 60;
        }
      else if (i == 28)
        {
          DataRate = StringValue ("OfdmRate90MbpsBW40MHz");
          datarate = 90;
        }
      else if (i == 29)
        {
          DataRate = StringValue ("OfdmRate120MbpsBW40MHz");
          datarate = 120;
        }
      else if (i == 30)
        {
          DataRate = StringValue ("OfdmRate135MbpsBW40MHzShGi");
          datarate = 135;
        }
      else
        {
          DataRate = StringValue ("OfdmRate150MbpsBW40MHz");
          datarate = 150;
        }

      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", DataRate,
                                    "ControlMode", DataRate);

      NetDeviceContainer staDevice;
      NetDeviceContainer apDevice;

      if (wifiType == "ns3::YansWifiPhy")
        {
          mac.SetType ("ns3::StaWifiMac",
                       "Ssid", SsidValue (ssid),
                       "ActiveProbing", BooleanValue (false));
          staDevice = wifi.Install (phy, mac, wifiStaNode);
          mac.SetType ("ns3::ApWifiMac",
                       "Ssid", SsidValue (ssid));
          apDevice = wifi.Install (phy, mac, wifiApNode);

        }
      else if (wifiType == "ns3::SpectrumWifiPhy")
        {
          mac.SetType ("ns3::StaWifiMac",
                       "Ssid", SsidValue (ssid),
                       "ActiveProbing", BooleanValue (false));
          staDevice = wifi.Install (spectrumPhy, mac, wifiStaNode);
          mac.SetType ("ns3::ApWifiMac",
                       "Ssid", SsidValue (ssid));
          apDevice = wifi.Install (spectrumPhy, mac, wifiApNode);
        }

      // mobility.
      MobilityHelper mobility;
      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

      positionAlloc->Add (Vector (0.0, 0.0, 0.0));
      positionAlloc->Add (Vector (distance, 0.0, 0.0));
      positionAlloc->Add (Vector (distance, distance, 0.0));
      mobility.SetPositionAllocator (positionAlloc);

      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

      mobility.Install (wifiApNode);
      mobility.Install (wifiStaNode);
      mobility.Install (interferingNode);

      /* Internet stack*/
      InternetStackHelper stack;
      stack.Install (wifiApNode);
      stack.Install (wifiStaNode);

      Ipv4AddressHelper address;

      address.SetBase ("192.168.1.0", "255.255.255.0");
      Ipv4InterfaceContainer staNodeInterface;
      Ipv4InterfaceContainer apNodeInterface;

      staNodeInterface = address.Assign (staDevice);
      apNodeInterface = address.Assign (apDevice);

      /* Setting applications */
      ApplicationContainer serverApp, sinkApp;
      if (udp)
        {
          //UDP flow
          UdpServerHelper myServer (9);
          serverApp = myServer.Install (wifiStaNode.Get (0));
          serverApp.Start (Seconds (0.0));
          serverApp.Stop (Seconds (simulationTime+1));

          UdpClientHelper myClient (staNodeInterface.GetAddress (0), 9);
          myClient.SetAttribute ("MaxPackets", UintegerValue (1000));
          myClient.SetAttribute ("Interval", TimeValue (MilliSeconds (5)));
          myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));

          ApplicationContainer clientApp = myClient.Install (wifiApNode.Get (0));
          clientApp.Start (Seconds (1.0));
          clientApp.Stop (Seconds (simulationTime+1));
        }
      else
        {
          //TCP flow
          uint16_t port = 50000;
          Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
          PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", apLocalAddress);
          sinkApp = packetSinkHelper.Install (wifiStaNode.Get (0));

          sinkApp.Start (Seconds (0.0));
          sinkApp.Stop (Seconds (simulationTime+1));

          OnOffHelper onoff ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());
          onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
          onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
          onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
          onoff.SetAttribute ("DataRate", DataRateValue (1000000000)); //bit/s
          ApplicationContainer apps;

          AddressValue remoteAddress (InetSocketAddress (staNodeInterface.GetAddress (0), port));
          onoff.SetAttribute ("Remote", remoteAddress);
          apps.Add (onoff.Install (wifiApNode.Get (0)));
          apps.Start (Seconds (1.0));
          apps.Stop (Seconds (simulationTime+1));
        }

      // Configure waveform generator

      Ptr<SpectrumValue> wgPsd = Create<SpectrumValue> (SpectrumModelWifi5180MHz);
      *wgPsd = waveformPower / (100*180000);
      NS_LOG_INFO ("wgPsd : " << *wgPsd);

      if (wifiType == "ns3::SpectrumWifiPhy")
        {
          WaveformGeneratorHelper waveformGeneratorHelper;
          waveformGeneratorHelper.SetChannel (spectrumChannel);
          waveformGeneratorHelper.SetTxPowerSpectralDensity (wgPsd);

          waveformGeneratorHelper.SetPhyAttribute ("Period", TimeValue (Seconds (0.0007)));    
          waveformGeneratorHelper.SetPhyAttribute ("DutyCycle", DoubleValue (1));
          NetDeviceContainer waveformGeneratorDevices = waveformGeneratorHelper.Install (interferingNode);

          Simulator::Schedule (Seconds (0.002), &WaveformGenerator::Start,
                               waveformGeneratorDevices.Get (0)->GetObject<NonCommunicatingNetDevice> ()->GetPhy ()->GetObject<WaveformGenerator> ());
        }

      Config::ConnectWithoutContext ("/NodeList/0/DeviceList/*/Phy/MonitorSnifferRx", MakeCallback (&MonitorSniffRx));
      if (wifiType == "ns3::YansWifiPhy")
        {
          if (enablePcap)
            {
              std::stringstream ss;
              ss << "wifi-spectrum-per-example-" << i;
              phy.EnablePcap (ss.str(), apDevice);
            }
        }

      g_signalDbmAvg = 0;
      g_noiseDbmAvg = 0;
      g_samples = 0;
      g_channelNumber = 0;
      g_rate = 0;

      Simulator::Stop (Seconds (simulationTime+1));
      Simulator::Run ();
      Simulator::Destroy ();

      double throughput = 0;
      uint32_t totalPacketsThrough = 0;
      if (udp)
        {
          //UDP
          totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
          throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
        }
      else
        {
          //TCP
          uint32_t totalPacketsThrough = DynamicCast<PacketSink> (sinkApp.Get (0))->GetTotalRx ();
          throughput = totalPacketsThrough * 8 / (simulationTime * 1000000.0); //Mbit/s
        }
      std::cout << i << "\t" << datarate << "\t\t" << throughput << 
          " Mbit/s" << "\t" << totalPacketsThrough << "\t   " << 
          g_signalDbmAvg << "    " << g_noiseDbmAvg << "    " << 
          (g_signalDbmAvg - g_noiseDbmAvg) << std::endl;
    }
  return 0;
}
