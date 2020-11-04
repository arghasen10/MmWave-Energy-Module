/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Andrea Sacco
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
 * Author: Argha Sen <arghasen10@gmail.com>
 */
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/energy-source.h"
#include "mmwave-radio-energy-model.h"
#include "ns3/log.h"
#include "ns3/config-store-module.h"
#include "ns3/config.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("MmWaveRadioEnergyModel");
NS_OBJECT_ENSURE_REGISTERED ("MmWaveRadioEnergyModel");

void traceuefunc (std::string path, RxPacketTraceParams params)
{
    
}

TypeId
MmWaveRadioEnergyModel::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::MmWaveRadioEnergyModel")
        .SetParent<DeviceEnergyModel> ()
        .AddConstructor<MmWaveRadioEnergyModel> ()
        .AddTraceSource ("TotalEnergyConsumption",
                        "Total energy consumption of the radio device.",
                        MakeTraceSourceAccessor (&MmWaveRadioEnergyModel::m_totalEnergyConsumption),
                        "ns3::TracedValueCallback::Double")
        .AddAttribute ("DeepSleepA",
                        "The default Deep Sleep Current in Amperes.",
                        DoubleValue (0.001),
                        MakeDoubleAccessor (&MmWaveRadioEnergyModel::SetDeepSleepA,
                                            &MmWaveRadioEnergyModel::GetDeepSleepA),
                        MakeDoubleChecker<double> ())
        .AddAttribute ("LightSleepA",
                        "The default Light Sleep Current in Amperes.",
                        DoubleValue (0.020),
                        MakeDoubleAccessor (&MmWaveRadioEnergyModel::SetLightSleepA,
                                            &MmWaveRadioEnergyModel::GetLightSleepA),
                        MakeDoubleChecker<double> ())
        .AddAttribute ("MicroSleepA",
                        "The default Micro Sleep Current in Amperes.",
                        DoubleValue (0.045),
                        MakeDoubleAccessor (&MmWaveRadioEnergyModel::SetMicroSleepA,
                                            &MmWaveRadioEnergyModel::GetMicroSleepA),
                        MakeDoubleChecker<double> ())
        .AddAttribute ("RxCurrentA",
                        "The default Rx current in Amperes",
                        DoubleValue (0.350),
                        MakeDoubleAccessor (&MmWaveRadioEnergyModel::SetRxCurrentA,
                                            &MmWaveRadioEnergyModel::GetRxCurrentA),
                        MakeDoubleChecker<double> ())
        .AddAttribute ("TxCurrentA",
                        "The default Tx current in Amperes",
                        DoubleValue(0.350),
                        MakeDoubleAccessor (&MmWaveRadioEnergyModel::SetTxCurrentA,
                                            &MmWaveRadioEnergyModel::GetTxCurrentA),
                        MakeDoubleChecker<double> ())
  ;
  return tid;
}

MmWaveRadioEnergyModel::MmWaveRadioEnergyModel ()
:m_source (0),
 m_currentState (MmWavePhyState::IDLE),
 m_lastUpdateTime (Seconds(0.0)),
 m_nPendingChangeState (0)
{
    NS_LOG_FUNCTION (this);
    m_energyDepletionCallback.Nullify ();
    m_listener = new MmWaveRadioEnergyModelPhyListener;
    m_listener->SetChangeStateCallback (MakeCallback(&DeviceEnergyModel::ChangeState, this));
    
    m_source = 0;

}

MmWaveRadioEnergyModel::~MmWaveRadioEnergyModel ()
{
    NS_LOG_FUNCTION (this);
    delete m_listener;
}

void
MmWaveRadioEnergyModel::SetEnergySource (Ptr<EnergySource> source)
{
    NS_LOG_FUNCTION (this << source);
    NS_ASSERT (source != NULL);
    m_source = source;
    m_switchToOffEvent.Cancel ();
    Time durationOff = GetMaximumTimeInState (m_currentState);
    m_switchToOffEvent = Simulator::Schedule(durationOff, &MmWaveRadioEnergyModel::ChangeState, this, MmWavePhyState::IDLE);
}

void
MmWaveRadioEnergyModel::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  NS_ASSERT (node != NULL);
  m_node = node;
}

Ptr<Node>
MmWaveRadioEnergyModel::GetNode () const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

double
MmWaveRadioEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.IsPositive ());
  double energyToDecrease = 0.0;
  double supplyVoltage = m_source->GetSupplyVoltage ();
  energyToDecrease = duration.GetSeconds () * GetStateA(m_currentState) * supplyVoltage;

  m_source->UpdateEnergySource ();
  
  return m_totalEnergyConsumption + energyToDecrease;
}

double 
MmWaveRadioEnergyModel::GetDeepSleepA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_deepSleepCurrentA;
}

void
MmWaveRadioEnergyModel::SetDeepSleepA (double deepSleepA)
{
  NS_LOG_FUNCTION (this << deepSleepA);
  m_deepSleepCurrentA = deepSleepA;
}

double 
MmWaveRadioEnergyModel::GetLightSleepA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_lightSleepCurrentA;
}

void
MmWaveRadioEnergyModel::SetLightSleepA (double lightSleepA)
{
  NS_LOG_FUNCTION (this << lightSleepA);
  m_lightSleepCurrentA = lightSleepA;
}

void 
MmWaveRadioEnergyModel::GetMicroSleepA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_microSleepCurrentA;
}

double
MmWaveRadioEnergyModel::SetMicroSleepA (double microSleepA)
{
  NS_LOG_FUNCTION (this << microSleepA);
  m_microSleepCurrentA = microSleepA;
}

void 
MmWaveRadioEnergyModel::GetRxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxCurrentA;
}

double 
MmWaveRadioEnergyModel::SetRxCurrentA (double rxCurrentA)
{
  NS_LOG_FUNCTION (this << rxCurrentA);
  m_rxCurrentA = rxCurrentA;
} 

void 
MmWaveRadioEnergyModel::GetTxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txCurrentA;
}

double 
MmWaveRadioEnergyModel::SetTxCurrentA (double txCurrentA)
{
  NS_LOG_FUNCTION (this << txCurrentA);
  m_txCurrentA = txCurrentA;
}

MmWavePhyState
MmWaveRadioEnergyModel::GetCurrentState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

void
MmWaveRadioEnergyModel::SetEnergyDepletionCallback (
  MmWaveRadioEnergyDepletionCallback callback
)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
  {
    NS_LOG_DEBUG ("MmWaveRadioEnergyModel:Setting NULL energy depletion callback!");
  }
  m_energyDepletionCallback = callback;
}

void
MmWaveRadioEnergyModel::SetEnergyRechargedCallback (
  MmWaveRadioEnergyRechargedCallback callback
)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
  {
    NS_LOG_DEBUG ("MmWaveRadioEnergyModel:Setting NULL energy recharged callback!");
  }
  m_energyRechargedCallback = callback;
}

Time
MmWaveRadioEnergyModel::GetMaximumTimeInState (int state) const
{
  // if (state == MmWavePhyState::IDLE)
  // {
  //   NS_FATAL_ERROR ("Requested maximum remaining time for OFF state");
  // }
  double remainingEnergy = m_source->GetRemainingEnergy ();
  double supplyVoltage = m_source->GetSupplyVoltage ();
  double current  = GetStateA (state);
  return Seconds (remainingEnergy / (current*supplyVoltage));
}

void 
MmWaveRadioEnergyModel::ChangeState (int state)
{
  NS_LOG_FUNCTION (this << state);

  m_nPendingChangeState ++;
  if (m_nPendingChangeState > 1 && state == MmWavePhyState::IDLE)
  {
    SetMmWaveRadioState ( (MmWavePhyState) state);
    m_nPendingChangeState --;
    return;
  }

  if (state != MmWavePhyState::IDLE)
  {
    m_switchToOffEvent.Cancel ();
    Time durationToOff = GetMaximumTimeInState (state);
    m_switchToOffEvent = Simulator::Schedule (durationToOff, &MmWaveRadioEnergyModel::ChangeState, this, MmWavePhyState::IDLE);
  }

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.IsPositive ());

  double supplyVoltage = m_source->GetSupplyVoltage ();
  double energyToDecrease = duration.GetSeconds () * GetStateA (m_currentState) * supplyVoltage;

  m_totalEnergyConsumption += energyToDecrease;
  NS_ASSERT (m_totalEnergyConsumption <= m_source->GetInitialEnergy ());

  m_lastUpdateTime = Simulator::Now ();
  m_source->UpdateEnergySource ();

  if (m_nPendingChangeState <= 1 && m_currentState != MmWavePhyState::IDLE)
  {
    SetMmWaveRadioState ((MmWavePhyState) state);

    // some debug message
    NS_LOG_DEBUG ("MmWaveRadioEnergyModel:Total energy consumption is " <<
              m_totalEnergyConsumption << "J");
  } 
  m_nPendingChangeState --;

}

void 
MmWaveRadioEnergyModel::HandleEnergyDepletion (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("MmWaveRadioEnergyModel:Energy is depleted!");
  // invoke energy depletion callback, if set.
  if (!m_energyDepletionCallback.IsNull ())
    {
      m_energyDepletionCallback ();
    }
}

void
MmWaveRadioEnergyModel::HandleEnergyRecharged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("MmWaveRadioEnergyModel:Energy is recharged!");
  // invoke energy recharged callback, if set.
  if (!m_energyRechargedCallback.IsNull ())
    {
      m_energyRechargedCallback ();
    }
}

void
MmWaveRadioEnergyModel::HandleEnergyChanged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("MmWaveRadioEnergyModel:Energy is changed!");
  if (m_currentState != MmWavePhyState::OFF)
    {
      m_switchToOffEvent.Cancel ();
      Time durationToOff = GetMaximumTimeInState (m_currentState);
      m_switchToOffEvent = Simulator::Schedule (durationToOff, &MmWaveRadioEnergyModel::ChangeState, this, MmWavePhyState::IDLE);
    }
}

MmWaveRadioEnergyModelPhyListener *
MmWaveRadioEnergyModel::GetPhyListener (void)
{
  NS_LOG_FUNCTION (this);
  return m_listener;
}

/* 
* Private functions start here
*/

void 
MmWaveRadioEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_source = NULL;
  m_energyDepletionCallback.Nullify ();
}

double
MmWaveRadioEnergyModel::GetStateA (int state) const
{
  switch (state)
  {
  case MmWavePhyState::IDLE:
    return m_deepSleepCurrentA;
  case MmWavePhyState::RX_CTRL:
    return m_rxCurrentA;
  case MmWavePhyState::RX_DATA:
    return m_rxCurrentA;
  case MmWavePhyState::TX:
    return m_txCurrentA;
  }
  NS_FATAL_ERROR ("MmWaveRadioEnergyModel: undefined radio state " << state);
}


void
MmWaveRadioEnergyModel::SetCurrentA (double current)
{
  NS_LOG_FUNCTION (this << current);
  Time duration = Simulator::Now () - m_lastUpdateTime;

  double energyToDecrease = 0.0;
  double supplyVoltage = m_source->GetSupplyVoltage ();
  energyToDecrease = duration.GetSeconds () * current * supplyVoltage;

  // update total energy consumption
  m_totalEnergyConsumption += energyToDecrease;
  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();
  // notify energy source
  m_source->UpdateEnergySource ();
  // update the current drain
  m_actualCurrentA = current;
}


double
MmWaveRadioEnergyModel::DoGetCurrentA (void) const
{
  return GetStateA (m_currentState);
}

void 
MmWaveRadioEnergyModel::SetMmWaveRadioState (const MmWavePhyState state)
{
  NS_LOG_FUNCTION (this << state);
  m_currentState = state;
  std::string stateName;
  switch (state)
  {
  case MmWavePhyState::IDLE:
    stateName = "IDLE";
    break;
  case MmWavePhyState::RX_CTRL:
    stateName = "RX_CTRL";
  case MmWavePhyState::RX_DATA:
    stateName = "RX_DATA";
    break;
  case MmWavePhyState::TX:
    stateName = "TX";
    break;
  }

  NS_LOG_DEBUG ("MmWaveRadioEnergyModel:Switching to state: " << stateName <<
                " at time = " << Simulator::Now ());
}

// -------------------------------------------------------------------------- //

MmWaveRadioEnergyModelPhyListener::MmWaveRadioEnergyModelPhyListener ()
{
  NS_LOG_FUNCTION (this);
  m_changeStateCallback.Nullify ();
}
MmWaveRadioEnergyModelPhyListener::~MmWaveRadioEnergyModelPhyListener ()
{
  NS_LOG_FUNCTION (this);
}

MmWaveRadioEnergyModelPhyListener::SetChangeStateCallback (DeviceEnergyModel::ChangeStateCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);
  NS_ASSERT (!callback.IsNull ());
  m_changeStateCallback = callback;
}

void
MmWaveRadioEnergyModelPhyListener::NotifyRxStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("MmWaveRadioEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (MmWavePhyState::RX_DATA);
  m_switchToIdleEvent.Cancel ();
}

void 
MmWaveRadioEnergyModelPhyListener::NotifyRxEndOk (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
  {
    NS_FATAL_ERROR ("MmWaveRadioEnergyModelPhyListener:Change state callback not set!");
  }
  m_changeStateCallback (MmWavePhyState::IDLE);
}

void
MmWaveRadioEnergyModelPhyListener::NotifyRxEndError (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("MmWaveRadioEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (MmWavePhyState::IDLE);
}

void
MmWaveRadioEnergyModelPhyListener::NotifyTxStart (Time duration)
{
  NS_LOG_FUNCTION (this << duration);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("MmWaveRadioEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (MmWavePhyState::TX);
  // schedule changing state back to IDLE after TX duration
  m_switchToIdleEvent.Cancel ();
  m_switchToIdleEvent = Simulator::Schedule (duration, &MmWaveRadioEnergyModelPhyListener::SwitchToIdle, this);
}

void
MmWaveRadioEnergyModelPhyListener::SwitchToIdle (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("MmWaveRadioEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (MmWavePhyState::IDLE);
}

}