// Copyright (C) 2013 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Benjamin Martin Seregi

#ifndef __INET_IEEE8021DRELAY_H
#define __INET_IEEE8021DRELAY_H

#include "inet/common/INETDefs.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/linklayer/ethernet/switch/IMacAddressTable.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"

namespace inet {

//
// This module forward frames (~EtherFrame) based on their destination MAC addresses to appropriate ports.
// See the NED definition for details.
//
class INET_API Ieee8021dRelay : public cSimpleModule, public ILifecycle
{
  public:
    Ieee8021dRelay();

    /**
     * Register single MAC address that this switch supports.
     */

    void registerAddress(MacAddress mac);

    /**
     * Register range of MAC addresses that this switch supports.
     */
    void registerAddresses(MacAddress startMac, MacAddress endMac);

  protected:
    MacAddress bridgeAddress;
    IInterfaceTable *ifTable = nullptr;
    IMacAddressTable *macTable = nullptr;
    InterfaceEntry *ie = nullptr;
    bool isOperational = false;
    bool isStpAware = false;

    typedef std::pair<MacAddress, MacAddress> MacAddressPair;


    struct Comp
    {
        bool operator() (const MacAddressPair& first, const MacAddressPair& second) const
        {
            return (first.first < second.first && first.second < second.first);
        }
    };

    bool in_range(const std::set<MacAddressPair, Comp>& ranges, MacAddress value)
    {
        return ranges.find(MacAddressPair(value, value)) != ranges.end();
    }


    std::set<MacAddressPair, Comp> registeredMacAddresses;

    // statistics: see finish() for details.
    int numReceivedNetworkFrames = 0;
    int numDroppedFrames = 0;
    int numReceivedBPDUsFromSTP = 0;
    int numDeliveredBDPUsToSTP = 0;
    int numDispatchedNonBPDUFrames = 0;
    int numDispatchedBDPUFrames = 0;

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void handleMessage(cMessage *msg) override;

    /**
     * Updates address table (if the port is in learning state)
     * with source address, determines output port
     * and sends out (or broadcasts) frame on ports
     * (if the ports are in forwarding state).
     * Includes calls to updateTableWithAddress() and getPortForAddress().
     *
     */
    void handleAndDispatchFrame(Packet *packet);

    void handleAndDispatchFrameFromHL(Packet *packet);

    void dispatch(Packet *packet, InterfaceEntry *ie);
    void learn(MacAddress srcAddr, int arrivalInterfaceId);
    void broadcast(Packet *packet, int arrivalInterfaceId);

    void sendUp(Packet *packet);

    // For lifecycle
    virtual void start();
    virtual void stop();
    bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    /*
     * Gets port data from the InterfaceTable
     */
    Ieee8021dInterfaceData *getPortInterfaceData(unsigned int portNum);

    bool isForwardingInterface(InterfaceEntry *ie);

    /*
     * Returns the first non-loopback interface.
     */
    virtual InterfaceEntry *chooseInterface();
    virtual void finish() override;
};

} // namespace inet

#endif // ifndef __INET_IEEE8021DRELAY_H

