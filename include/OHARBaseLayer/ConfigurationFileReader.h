//
//  ConfigurationFileReader.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 27.1.2016.
//  Copyright © 2016 Antti Juustila. All rights reserved.
//

#ifndef ConfigurationFileReader_h
#define ConfigurationFileReader_h

#include <string>

#include <OHARBaseLayer/DataFileReader.h>

namespace OHARBase {
	
	class DataItem;
	class DataReaderObserver;
	
	/**
	 Class reads Node configuration data from a file. The file name is usually provided
	 as the startup parameters of the app which instantiates the ProcessorNode. App then
     instantiates the Node and calls configure() function.<p>
     File has the following contents. Note that the key-value pairs are tab separated:<br />
	 <pre>
	 nodeconfiguration
     input	127.0.0.1:1234
	 output	130.231.96.123:1234
	 filein	inputfile.txt
	 fileout	outputfile.txt
	 </pre>
	 First line must contain the word "nodeconfiguration" to identify the content type of the file.
	 Second, the  <i>input</i> line specifies the port number of the incoming data. Next, <i>output</i>
	 line specifies the address the Node sends data to, including the host IP address and port number.
	 The two files specify the possible input and output files Node uses. If any of these do not
	 exist, the value for the configuration item should be "null" or omit the item/line alltogether from the config file.
	 */
	class ConfigurationFileReader : public DataFileReader {
	public:
		ConfigurationFileReader(DataReaderObserver & obs);
		
		virtual DataItem * parse(const std::string & str, const std::string & contentType) override;
		
	private:
		static const std::string TAG;
		
	};
	
	
} //namespace
#endif /* ConfigurationFileReader_h */
