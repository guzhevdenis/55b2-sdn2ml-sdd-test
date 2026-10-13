#include "VncSdnPubSub.h"
#include <poll.h>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

volatile std::sig_atomic_t stopRequested = 0;
void stop(int) { stopRequested = 1; };

int main(int argc, char **argv)
{
    // TODO. Make it via configuration file
    std::string inputInterface{"ens192"};
    std::string outputInterface{"ens192"};

    std::signal(SIGINT, stop);
    std::signal(SIGTERM, stop);

    // SDN Subscribers configuration
    std::array<std::unique_ptr<vnc::VncInputSubscriber>, vnc::kIocCount> subscribers;
    std::array<pollfd, vnc::kIocCount> descriptors{};

    for (std::size_t i = 0; i < vnc::kIocCount; ++i)
    {
        subscribers[i].reset(new vnc::VncInputSubscriber(
            "pcf" + std::to_string(i) + "-nds-sdn-topic",
            "/home/codac-dev/55b2-sdn2ml-sdd/55b2-sdn2ml-sdd/b-55b2-sdn2ml-sdd/src/main/resources",
            inputInterface));

        descriptors[i].fd = subscribers[i]->socket();
        descriptors[i].events = POLLIN;

        std::cout << "socket for pcf" << std::to_string(i) << "-nds-sdn-topic -- " << descriptors[i].fd << std::endl;
    }
    
    // SDN Publishers configuration
    std::array<std::unique_ptr<vnc::VncOutputPublisher>, vnc::kPlasmaParamCount> publishers;
    for (std::size_t i = 0; i < vnc::kPlasmaParamCount; ++i)
    {
        publishers[i].reset(new vnc::VncOutputPublisher(
            "vnc-mp" + std::to_string(i) + "-topic",
            "/home/codac-dev/55b2-sdn2ml-sdd/55b2-sdn2ml-sdd/b-55b2-sdn2ml-sdd/src/main/resources",
            outputInterface));
    }

    // Receiving the data. Pack it in InputData
    vnc::InputCountRates InputData; // Data from all IOCs
    while (!stopRequested)
    {
        // Status is OK even only 1 socket is ready 
        const int status = ::poll(descriptors.data(), descriptors.size(), 100);
        if (status < 0)
        {
            std::cout << "Error during poll" << std::endl;
        }
        std::cout << status << std::endl;

        for (size_t i = 0; i <vnc::kIocCount; ++i)
            {
                //Data from 1 IOC 
                vnc::IocCountRates IocCountRates;
                subscribers[i]->subscribe(IocCountRates);
                InputData.update(i,IocCountRates);
            }

        vnc::MlProcessing processor;
        vnc::OutputData outputData;

        // Publish topics with measurement parameters to SDN
            for (size_t i = 0; i < vnc::kPlasmaParamCount; ++i){
                // TODO. Redisigned vnc::MlProcessing for multiple MP
                outputData.outputValue = processor.process(InputData.data());
                 // Bad -> copying
                publishers[i]->publish(outputData);
            }
    }
} 