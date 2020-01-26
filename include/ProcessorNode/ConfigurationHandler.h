//
//  ConfigurationHandler.h
//  PipesAndFiltersNode
//
//  Created by Antti Juustila on 15.12.2019.
//  Copyright (c) 2019 Antti Juustila. All rights reserved.
//

#pragma once

#include <ProcessorNode/DataHandler.h>

namespace OHARBase {
	
	
	class ProcessorNode;
	class Package;

	/**
	 ConfigurationHandler handles configuration related messages.
	 @author Antti Juustila
	 @see NodeConfiguration, ConfigurationFileReader, ProcessorNode
	 */
	class ConfigurationHandler : public DataHandler {
	public:
		ConfigurationHandler(ProcessorNode & myNode);
		virtual ~ConfigurationHandler();
		
		bool consume(Package & data) override;
      
	private:
		/** The processor node needed to handle the configuration messages. */
		ProcessorNode & node;
		/** The tag used in the logging to indicate which object is logging now. */
		static const std::string TAG;
	};
	
	
}

