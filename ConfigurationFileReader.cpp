//
//  ConfigurationFileReader.cpp
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 27.1.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#include <stdio.h>

#include <g3log/g3log.hpp>

#include <ProcessorNode/ConfigurationFileReader.h>
#include <ProcessorNode/ConfigurationDataItem.h>


namespace OHARBase {

   // TODO: change configuration file format to JSON.

	const std::string ConfigurationFileReader::TAG{"ConfigReader "};
   const char configFileCommentChar('#');

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
    @todo Allow comment lines in config files, starting with "#". Just ignore those lines by returning nullptr.
    */
	std::unique_ptr<DataItem> OHARBase::ConfigurationFileReader::parse(const std::string & str, const std::string & contentType) {
		std::unique_ptr<DataItem> item(nullptr);
		if (str.length() > 0 && str[0] != configFileCommentChar) {
         item = std::make_unique<ConfigurationDataItem>();
         if (!item->parse(str, contentType)) {
            LOG(WARNING) << TAG << "Configuration failed to parse!";
            item.release();
            throw std::runtime_error("Configuration file corrupt");
         }
		}
		return item;
	}
   
} // namespace
