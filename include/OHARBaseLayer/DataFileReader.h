//
//  DataFileReader.h
//  PipesAndFiltersFramework
//
//  Created by Antti Juustila on 5.12.2013.
//  Copyright (c) 2013 Antti Juustila. All rights reserved.
//

#ifndef __PipesAndFiltersFramework__DataFileReader__
#define __PipesAndFiltersFramework__DataFileReader__

#include <list>

namespace OHARBase {
	
	//Forward declarations.
	class DataItem;
	class DataReaderObserver;
	
	/** An abstract class defining the interface for reading data from
	 files and parsing them to DataItem objects. Data file reader has an 
	 observer which gets a notification when data items are read from file.
	 The abstract parse() method needs to be implemented by subclasses to
	 provide application and file specific parsing of data read from a file.<p>
	 Implementation assumes that files contain lines of text, read one by one,
	 and the lines are parsed to get the actual content into DataItem member
	 variables.
	 @author Antti Juustila
	 */
	class DataFileReader {
	public:
		DataFileReader(DataReaderObserver & obs);
		virtual ~DataFileReader();
		
		virtual bool read(const std::string &fileName);
		
	protected:
		DataFileReader(const DataFileReader &) = delete;
		const DataFileReader & operator = (const DataFileReader &) = delete;
		
		/** Abstract method defining the interface subclasses must implement to parse
		 data from a line of text read from the data file.
		 @param str The string containing the data to parse into DataItem object.
		 @param contentType Application specific metadata which defines what kind of data
		 the str is expected to contain.
		 @return The DataItem object parsed from the string. Null if no data was successfully parsed.
		 */
		virtual DataItem * parse(const std::string & str, const std::string & contentType) = 0;
		
		/** The observer gets a notification after each parsed line where a DataItem was created. */
		DataReaderObserver & observer;
		
	private:
		
		/** Tag for printing debug output to Log. */
		static const std::string TAG;
	};
	
	
} //namespace
#endif /* defined(__PipesAndFiltersFramework__DataFileReader__) */
