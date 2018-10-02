//
//  DataHandler.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 14.10.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#ifndef __PipesAndFiltersFramework__DataHandler__
#define __PipesAndFiltersFramework__DataHandler__

#include <string>

namespace OHARBase {
	class Package;
	
	/**
	 DataHandler is an abstract class for handling data arriving to a Filter.
	 Datahandler consumes data packages the Filter (the ProcessorNode) offers to it.
	 Create new data handlers by inheriting DataHandler and implementing the consume method.
	 @author Antti Juustila
	 */
	class DataHandler {
	public:
		DataHandler(const DataHandler &) = delete;
		const DataHandler & operator =(const DataHandler &) = delete;
		
		virtual ~DataHandler();
		virtual bool consume(Package & data) = 0;
		
	protected:
		DataHandler();
		
	private:
		
	};
	
	
}


#endif /* defined(__PipesAndFiltersFramework__DataHandler__) */
