//
//  ConfigurationFileReader.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 27.1.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#pragma once

#include <string>

#include <ProcessorNode/DataFileReader.h>

namespace OHARBase {
	
	class DataItem;
	class DataReaderObserver;
	
	/**
   Class reads Node configuration data from a file. The file name is usually provided
   as the startup parameters of the app which instantiates the ProcessorNode. App then
   instantiates the Node and calls configure() function.<p>
   See project documentation on the structure and detailed contents of the configuration file. <br />
   First line in config file must contain only the word "nodeconfiguration" to identify the content type of the file.
	 */
	class ConfigurationFileReader : public DataFileReader {
	public:
		ConfigurationFileReader(DataReaderObserver & obs);
		
      virtual std::unique_ptr<DataItem> parse(const std::string & str, const std::string & contentType) override;
		
	private:
		static const std::string TAG;
		
	};
	
	
} //namespace
