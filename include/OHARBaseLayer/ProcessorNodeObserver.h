//
//  ProcessorNodeObserver.h
//
//  Created by Antti Juustila on 19.10.2018.
//  Copyright (c) 2018 Antti Juustila. All rights reserved.
//

#pragma once

#include <string>

namespace OHARBase {
    
    /**
     An interface ProcessorNode uses to notify it's observer of events in the node.
     @author Antti Juustila
     */
    class ProcessorNodeObserver {
    public:
        ProcessorNodeObserver() {/*empty*/};
        virtual ~ProcessorNodeObserver() {/*empty*/};
        
        /*! Type of the event in the node, notified to the observer. */
        enum EventType {
            ENullEvent, /*!< No event/undefined. */
            NotificationEvent, /*!< Normal notification that something of interest to user happened. */
            ShutDownEvent, /*!< Node notifies the App that it should close down (usually because notification from previous node asks it to.). */
            WarningEvent, /*!<Something fishy going on, warning the app/user to check out if config, networking or something else is wrong, file is missing etc. */
            ErrorEvent /*!< Something went badly wrong in the node. Notify the app/user. */
        };
        /** Called by the node to notify the app / user that something of interest happened in the node.
         @param e Type of the event.
         @param message The message to show to user.
         */
        virtual void NodeEventHappened(EventType e, const std::string & message) = 0;
        
    protected:
        
    private:
        ProcessorNodeObserver(const ProcessorNodeObserver &) = delete;
        const ProcessorNodeObserver & operator = (const ProcessorNodeObserver &) = delete;
    };
    
} //namespace

