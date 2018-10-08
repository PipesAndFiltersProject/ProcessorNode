//
//  ConfigurationFileReader.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 27.1.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#include <stdio.h>

#include <OHARBaseLayer/ConfigurationFileReader.h>
#include <OHARBaseLayer/ConfigurationDataItem.h>
#include <OHARBaseLayer/Log.h>

namespace OHARBase {

	const std::string ConfigurationFileReader::TAG{"ConfigReader"};
	
   /**
    Constructor for configuration file reader.
    @param obs The observer who gets notifications of content read from the configuration file.
    */
	OHARBase::ConfigurationFileReader::ConfigurationFileReader(DataReaderObserver & obs)
	: DataFileReader(obs) {
		
	}
	
   /**
    Parses a line from the configuration file.
    @param str An item from the configuration file.
    @param contentType The type of the content in this item.
    @returns The configuration data item read from the line. Null if failed to parse the configuration item.
    */
	OHARBase::DataItem * OHARBase::ConfigurationFileReader::parse(const std::string & str, const std::string & contentType) {
		ConfigurationDataItem * item = nullptr;
		if (str.length() > 0) {
         Log::get() << TAG << "Parsing line: %s" << str << std::endl;
			item = new ConfigurationDataItem();
			if (!item->parse(str, contentType)) {
            Log::get() << TAG << "Configuration failed to parse!" << std::endl;
				delete item;
				item = 0;
			}
		}
		return item;
		
	}

	
}
