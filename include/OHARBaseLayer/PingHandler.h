//
//  PingHandler.h
//  PipesAndFiltersStudentNode
//
//  Created by Antti Juustila on 25.11.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#ifndef __PipesAndFiltersStudentNode__PingHandler__
#define __PipesAndFiltersStudentNode__PingHandler__


#include <OHARBaseLayer/DataHandler.h>

namespace OHARBase {
	
	
	class ProcessorNode;
	class Package;
	
	/**
	 PingHandler handles ping messages.
	 @author Antti Juustila
	 @see DataHandler
	 */
	class PingHandler : public DataHandler {
	public:
		PingHandler(ProcessorNode & myNode);
		virtual ~PingHandler();
		
		bool consume(Package & data) override;
		
	private:
		/** The processor node used to forward the ping message. */
		ProcessorNode & node;
		/** The tag used in the logging to indicate which object is logging now. */
		const std::string TAG;
	};
	
	
}

#endif /* defined(__PipesAndFiltersStudentNode__PingHandler__) */
