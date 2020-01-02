//
//  ConfigurationHandler.h
//  PipesAndFiltersNode
//
//  Created by Antti Juustila on 25.11.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#pragma once

#include <OHARBaseLayer/DataHandler.h>

namespace OHARBase {
	
	
	class ProcessorNode;
	class Package;
	
	/**
	 ConfigurationHandler handles ping messages.
	 @author Antti Juustila
	 @see DataHandler
	 */
	class ConfigurationHandler : public DataHandler {
	public:
		ConfigurationHandler(ProcessorNode & myNode);
		virtual ~ConfigurationHandler();
		
		bool consume(Package & data) override;
		
	private:
		/** The processor node used to forward the ping message. */
		ProcessorNode & node;
		/** The tag used in the logging to indicate which object is logging now. */
		static const std::string TAG;
	};
	
	
}

