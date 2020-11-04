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
#ifndef MMWAVE_RADIO_ENERGY_MODEL_H
#define MMWAVE_RADIO_ENERGY_MODEL_H

#include "ns3/device-energy-model.h"
#include "ns3/traced-value.h"
#include "ns3/nstime.h"
#include "mmwave-phy-listener.h"
#include "mmwave-phy-state.h"
#include "ns3/event-id.h"

namespace ns3 {

class MmWaveRadioEnergyModelPhyListener : public MmWavePhyListener
{

public:
    MmWaveRadioEnergyModelPhyListener ();
    virtual ~MmWaveRadioEnergyModelPhyListener();

    /**
     * \brief Sets the change state callback. Used by helper class.
     *
     * \param callback Change state callback.
     */
    void SetChangeStateCallback (DeviceEnergyModel::ChangeStateCallback callback);

    /**
     * \brief Switches the MmWaveRadioEnergyModel to RX state.
     *
     * \param duration the expected duration of the packet reception.
     *
     * Defined in ns3::MmWavePhyListener
     */
    void NotifyRxStart (Time duration);

    /**
     * \brief Switches the MmWaveRadioEnergyModel back to IDLE state.
     *
     * Defined in ns3::MmWavePhyListener
     *
     * Note that for the MmWaveRadioEnergyModel, the behavior of the function is the
     * same as NotifyRxEndError.
     */
    void NotifyRxEndOk (void);

    /**
     * \brief Switches the MmWaveRadioEnergyModel back to IDLE state.
     *
     * Defined in ns3::MmWavePhyListener
     *
     * Note that for the MmWaveRadioEnergyModel, the behavior of the function is the
     * same as NotifyRxEndOk.
     */
    void NotifyRxEndError (void);

    /**
     * \brief Switches the MmWaveRadioEnergyModel to TX state and switches back to
     * IDLE after TX duration.
     *
     * \param duration the expected transmission duration.
     *
     * Defined in ns3::MmWavePhyListener
     */
    void NotifyTxStart (Time duration);

private:

    /**
     * A helper function that makes scheduling m_changeStateCallback possible.
     */
    void SwitchToIdle (void);

    /**
     * Change state callback used to notify the MmWaveRadioEnergyModel of a state
     * change.
     */
    DeviceEnergyModel::ChangeStateCallback m_changeStateCallback;

    EventId m_switchToIdleEvent; ///< switch to idle event

};


class MmWaveRadioEnergyModel : public DeviceEnergyModel
{

public:
    /**
     * Callback type for energy depletion handling.
     */
    typedef Callback<void> MmWaveRadioEnergyDepletionCallback;

    /**
     * Callback type for energy recharged handling.
     */
    typedef Callback<void> MmWaveRadioEnergyRechargedCallback;

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId (void);
    MmWaveRadioEnergyModel ();
    virtual ~ MmWaveRadioEnergyModel ();
    /**
    * \brief Sets pointer to node.
    *
    * \param node Pointer to node.
    *
    * Implements DeviceEnergyModel::SetNode.
    */
    virtual void SetNode (Ptr<Node> node);

    /**
     * \brief Gets pointer to node.
     *
     * \returns Pointer to node.
     *
     * Implements DeviceEnergyModel::GetNode.
     */
    virtual Ptr<Node> GetNode (void) const;

    /**
     * \brief Sets pointer to EnergySouce installed on node.
     *
     * \param source Pointer to EnergySource installed on node.
     *
     * Implements DeviceEnergyModel::SetEnergySource.
     */
    virtual void SetEnergySource (Ptr<EnergySource> source);

    /**
     * \returns Total energy consumption of the Device.
     *
     * Implements DeviceEnergyModel::GetTotalEnergyConsumption.
     */
    virtual double GetTotalEnergyConsumption (void) const;


    /**
     * \param current the current draw of device.
     *
     * Set the actual current draw of the device.
     */
    void SetCurrentA (double current);

    // Setter & getters for state power consumption.
    /**
     * \brief Gets Deep Sleep current in Amperes.
     *
     * \returns deep sleep current of the mmwave device.
     */
    double GetDeepSleepA (void) const;

    /**
     * \brief Sets deep sleep current in Amperes.
     *
     * \param deepSleepCurrentA the deep sleep current
     */
    void SetDeepSleepA (double deepSleepCurrentA);

    /**
     * \brief Gets light sleep current in Amperes.
     *
     * \returns light sleep current of the mmwave device.
     */
    double GetLightSleepA (void) const;

    /**
     * \brief Sets light sleep current in Amperes.
     *
     * \param lightSleepCurrentA the light sleep current
     */
    void SetLightSleepA (double lightSleepCurrentA);

    /**
     * \brief Gets micro sleep current in Amperes.
     *
     * \returns micro sleep current of the mmwave device.
     */
    double GetMicroSleepA (void) const;

    /**
     * \brief Sets micro sleep current in Amperes.
     *
     * \param microSleepCurrentA the micro sleep current
     */
    void SetMicroSleepA (double microSleepCurrentA);

    /**
     * \brief Gets Rx current in Amperes.
     *
     * \returns rx current of the mmwave device.
     */
    double GetRxCurrentA (void) const;

    /**
     * \brief Sets RX current in Amperes.
     *
     * \param rxCurrentA the Rx current
     */
    void SetRxCurrentA (double rxCurrentA);

    /**
     *  \brief Gets Tx current in Amperes.
     * 
     *  \returns tx current of the mmwave device.
     */
    double GetTxCurrentA (void) const;

    /**
     * \brief Sets Tx current in Amperes.
     * 
     * \param txCurrentA the tx current. 
     */
    void SetTxCurrentA (double txCurrentA);

    /**
     * \returns Current state.
     */
    MmWavePhyState GetCurrentState (void) const;

    /**
     * \param callback Callback function.
     *
     * Sets callback for energy depletion handling.
     */
    void SetEnergyDepletionCallback (MmWaveRadioEnergyDepletionCallback callback);

    /**
     * \param callback Callback function.
     *
     * Sets callback for energy recharged handling.
     */
    void SetEnergyRechargedCallback (MmWaveRadioEnergyRechargedCallback callback);

    /**
     * \brief Changes state of the WifiRadioEnergyMode.
     *
     * \param newState New state the wifi radio is in.
     *
     * Implements DeviceEnergyModel::ChangeState.
     */

    /**
     * \param state the wifi state
     *
     * \returns the time the radio can stay in that state based on the remaining energy.
     */
    Time GetMaximumTimeInState (int state) const;
    void ChangeState (int newState);

    /**
     * \brief Handles energy depletion.
     *
     * Implements DeviceEnergyModel::HandleEnergyDepletion
     */
    void HandleEnergyDepletion (void);

    /**
     * \brief Handles energy recharged.
     *
     * Implements DeviceEnergyModel::HandleEnergyRecharged
     */
    void HandleEnergyRecharged (void);

    /**
     * \brief Handles energy changed.
     *
     * Implements DeviceEnergyModel::HandleEnergyChanged
     */
    void HandleEnergyChanged (void);
    /**
     * \returns Pointer to the PHY listener.
     */
    MmWaveRadioEnergyModelPhyListener * GetPhyListener (void);


private:
    void DoDispose (void);

    /**
     * \returns Current draw of device, at current state.
     *
     * Implements DeviceEnergyModel::GetCurrentA.
     */
    virtual double DoGetCurrentA (void) const;

    /**
     * \param state the MmWave state
     * \returns draw of device in Amperes, at given state.
     */
    double GetStateA (int state) const;

    /**
     * \param state New state the radio device is currently in.
     *
     * Sets current state. This function is private so that only the energy model
     * can change its own state.
     */
    void SetMmWaveRadioState (const MmWavePhyState state);


    Time m_lastUpdateTime;
    double m_txCurrentA;
    double m_rxCurrentA;
    double m_deepSleepCurrentA;
    double m_lightSleepCurrentA;
    double m_microSleepCurrentA;
    MmWavePhyState m_currentState;

    Ptr<EnergySource> m_source;
    Ptr<Node> m_node;
    TracedValue<double> m_totalEnergyConsumption;
    uint8_t m_nPendingChangeState;
    MmWaveRadioEnergyRechargedCallback m_energyRechargedCallback;
    MmWaveRadioEnergyDepletionCallback m_energyDepletionCallback;

    MmWaveRadioEnergyModelPhyListener *m_listener;
    EventId m_switchToOffEvent;
};


} //namespace ns3

#endif
