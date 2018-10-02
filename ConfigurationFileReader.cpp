//
//  ConfigurationFileReader.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 27.1.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#include <stdio.h>

#include "ConfigurationFileReader.h"
#include "ConfigurationDataItem.h"
#include "Log.h"

namespace OHARBase {

	const std::string ConfigurationFileReader::TAG{"ConfigReader"};
	
	OHARBase::ConfigurationFileReader::ConfigurationFileReader(DataReaderObserver & obs)
	: DataFileReader(obs) {
		
	}
	
	OHARBase::DataItem * OHARBase::ConfigurationFileReader::parse(const std::string & str, const std::string & contentType) {
		ConfigurationDataItem * item = nullptr;
		if (str.length() > 0) {
			Log::getInstance().entry(TAG, "Parsing line: %s", str.c_str());
			item = new ConfigurationDataItem();
			if (!item->parse(str, contentType)) {
				Log::getInstance().entry(TAG, "Configuration failed to parse!");
				delete item;
				item = 0;
			}
		}
		return item;
		
	}

	
}
