//
//  DataHandler.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 14.10.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#include <ProcessorNode/DataHandler.h>

namespace OHARBase {

	DataHandler::DataHandler() {
	}
	
	DataHandler::~DataHandler() {
	}
	
	/**
	 Consumes a data package offered to the handler. Default empty implementation which just returns false.
	 @param data The data to handle.
	 @return true if data was handled and/or it should not be offered to any other handler. False otherwise.
	 Default implementation returns false;
	 */
	bool DataHandler::consume(Package & data) {
		return false;
	}

}
