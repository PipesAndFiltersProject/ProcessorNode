//
//  PingHandler.cpp
//  PipesAndFiltersNode
//
//  Created by Antti Juustila on 25.11.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <g3log/g3log.hpp>

#include <OHARBaseLayer/PingHandler.h>
#include <OHARBaseLayer/ProcessorNode.h>
#include <OHARBaseLayer/Package.h>

namespace OHARBase {
    
    /**
     Constructs a PingHandler object. Ping handler must now about the ProcessorNode
     in order to forward the ping message to the next ProcessorNode.
     @param myNode The processor node to use to forward the ping message.
     */
    PingHandler::PingHandler(ProcessorNode & myNode)
    : node(myNode), TAG("PingHandler ")
    {
    }
    
    PingHandler::~PingHandler() {
        
    }
    
    /**
     Implementation of handling the ping message. Basic functionality is to log the ping
     message arrival and then forward the message to the next ProcessorNode.
     @param data Handler checks if the data contains a ping message and if yes, handles it.
     @return If the message was ping message, returns true to indicate the ProcessorNode that
     no further processing of the message package is needed from other Handlers.
     */
    bool PingHandler::consume(Package & data) {
        if (data.getType() == Package::Control && data.getData() == "ping") {
            LOG(INFO) << TAG << "***** PING received, forwarding to next node! *****";
            node.sendData(data);
            return true;
        }
        return false;
    }
    
} //namespace


