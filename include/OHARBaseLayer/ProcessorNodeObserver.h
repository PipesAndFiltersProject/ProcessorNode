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
 
   enum EventType { ENullEvent, UINotificationEvent, ShutDownEvent, Error };
   virtual void NodeEventHappened(EventType e, const std::string & message) = 0;

protected:

private:
   ProcessorNodeObserver(const ProcessorNodeObserver &) = delete;
   const ProcessorNodeObserver & operator = (const ProcessorNodeObserver &) = delete;
};
	
} //namespace

